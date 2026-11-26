#include "applicationdatabase.h"

#include "databaseschema.h"
#include "demouserseeder.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

static bool prepareApplicationDatabase(QSqlDatabase &database, QString *errorMessage)
{
    const QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (dataPath.isEmpty() || !QDir().mkpath(dataPath)) {
        *errorMessage = QStringLiteral("Cannot create application data directory: %1").arg(dataPath);
        return false;
    }

    const QString databasePath = QDir(dataPath).filePath(QStringLiteral("mixingstation.sqlite"));
    const QString legacyDatabasePath = QDir(dataPath).filePath(QStringLiteral("pumpstation.sqlite"));

    if (!QFile::exists(databasePath) && QFile::exists(legacyDatabasePath)) {
        if (!QFile::rename(legacyDatabasePath, databasePath)) {
            *errorMessage = QStringLiteral("Cannot rename legacy database: %1 to %2").arg(legacyDatabasePath, databasePath);
            return false;
        }

        qInfo() << "Legacy database renamed to" << databasePath;
    }

    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(databasePath);

    if (!database.open()) {
        *errorMessage = QStringLiteral("Cannot open SQLite database: %1").arg(database.lastError().text());
        return false;
    }

    QSqlQuery foreignKeys(database);

    if (!foreignKeys.exec("PRAGMA foreign_keys = ON")) {
        *errorMessage = QStringLiteral("Cannot enable SQLite foreign keys: %1").arg(foreignKeys.lastError().text());
        return false;
    }

    QString schemaError;

    if (!initializeDatabaseSchema(database, &schemaError)) {
        *errorMessage = schemaError;
        return false;
    }

    QString demoUsersError;

    if (!seedDemoUsersIfEmpty(database, &demoUsersError)) {
        *errorMessage = QStringLiteral("Cannot prepare demo users: %1").arg(demoUsersError);
        return false;
    }

    return true;
}

ApplicationDatabase::ApplicationDatabase(const QSqlDatabase &connection)
    : m_connection(connection),
    m_userRepository(m_connection),
    m_runRepository(m_connection),
    m_eventRepository(m_connection),
    m_trendSampleRepository(m_connection),
    m_userListModel(m_connection),
    m_eventHistoryModel(m_connection),
    m_runListModel(m_connection),
    m_trendHistorySource(m_connection),
    m_retryQueue(m_runRepository, m_eventRepository, m_trendSampleRepository)
{
}

std::unique_ptr<ApplicationDatabase> ApplicationDatabase::open(QString &errorMessage)
{
    QSqlDatabase connection;

    if (!prepareApplicationDatabase(connection, &errorMessage))
        return nullptr;
    qInfo() << "SQLite database opened at" << connection.databaseName();
    auto database = std::unique_ptr<ApplicationDatabase>(new ApplicationDatabase(connection));
    QString modelError;

    if (!database->m_userListModel.reload(&modelError)) {
        errorMessage = QStringLiteral("Cannot load users: %1").arg(modelError);
        return nullptr;
    }

    if (!database->m_runListModel.reload(&modelError)) {
        errorMessage = QStringLiteral("Cannot load runs: %1").arg(modelError);
        return nullptr;
    }

    if (!database->m_eventHistoryModel.reload(&modelError)) {
        errorMessage = QStringLiteral("Cannot load event history: %1").arg(modelError);
        return nullptr;
    }

    return database;
}