#include "databaseschema.h"

#include <QSqlError>
#include <QSqlQuery>

bool initializeDatabaseSchema(QSqlDatabase &database, QString *errorMessage)
{
    QSqlQuery versionQuery(database);

    if (!versionQuery.exec("PRAGMA user_version") || !versionQuery.next()) {
        *errorMessage = "Cannot read database version: " + versionQuery.lastError().text();
        return false;
    }

    const int version = versionQuery.value(0).toInt();

    if (version == 1)
        return true;

    if (version != 0)
    {
        *errorMessage = "Unsupported database version: " + QString::number(version);
        return false;
    }

    if (!database.transaction())
    {

        *errorMessage = "Cannot start schema transaction: " + database.lastError().text();
        return false;
    }

    const char *statements[] = {
        R"SQL(
            CREATE TABLE users (
                user_id TEXT NOT NULL PRIMARY KEY,
                display_name TEXT NOT NULL,
                role TEXT NOT NULL
                    CHECK (role IN ('OPERATOR', 'TECHNICIAN', 'ADMIN')),
                is_active INTEGER NOT NULL DEFAULT 1
                    CHECK (is_active IN (0, 1))
            )
        )SQL",

        R"SQL(
            CREATE TABLE runs (
                run_id TEXT NOT NULL PRIMARY KEY,
                kind TEXT NOT NULL CHECK (kind IN ('BATCH', 'DRAIN')),
                batch_operator_user_id TEXT REFERENCES users(user_id),
                started_at_ms INTEGER NOT NULL,
                ended_at_ms INTEGER,
                outcome TEXT
            )
        )SQL",

        R"SQL(
            CREATE TABLE events (
                event_id INTEGER PRIMARY KEY,
                run_id TEXT REFERENCES runs(run_id),
                timestamp_ms INTEGER NOT NULL,
                level TEXT NOT NULL,
                source TEXT NOT NULL,
                message TEXT NOT NULL,
                actor_user_id TEXT REFERENCES users(user_id),
                authorized_by_user_id TEXT REFERENCES users(user_id)
            )
        )SQL",

        R"SQL(
            CREATE TABLE trend_samples (
                sample_id INTEGER PRIMARY KEY,
                run_id TEXT NOT NULL REFERENCES runs(run_id),
                timestamp_ms INTEGER NOT NULL,
                volume_l REAL NOT NULL,
                temperature_c REAL NOT NULL
            )
        )SQL",

        "CREATE INDEX events_by_run_time "
        "ON events(run_id, timestamp_ms)",

        "CREATE INDEX samples_by_run_time "
        "ON trend_samples(run_id, timestamp_ms)",

        "PRAGMA user_version = 1"
    };

    for (const char *statement : statements) {
        QSqlQuery query(database);

        if (!query.exec(QString::fromUtf8(statement))) {
            *errorMessage = "Cannot create database schema: "
                            + query.lastError().text();
            database.rollback();
            return false;
        }
    }

    if (!database.commit()) {
        *errorMessage = "Cannot commit database schema: "
                        + database.lastError().text();
        database.rollback();
        return false;
    }

    return true;
}