#include "userrepository.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

UserRepository::UserRepository(const QSqlDatabase &database)
    : m_database(database)
{
}

bool UserRepository::addUser(const QString &displayName,
                             const QString &role,
                             QString *errorMessage)
{
    const QString name = displayName.trimmed();

    if (name.isEmpty()) {
        *errorMessage = "User name is required";
        return false;
    }

    QSqlQuery query(m_database);

    if (!query.prepare(
            "INSERT INTO users (user_id, display_name, role) "
            "VALUES (:user_id, :display_name, :role)")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    query.bindValue(":user_id",
                    QUuid::createUuid().toString(QUuid::WithoutBraces));
    query.bindValue(":display_name", name);
    query.bindValue(":role", role);

    if (!query.exec()) {
        *errorMessage = query.lastError().text();
        return false;
    }

    return true;
}

bool UserRepository::changeRole(const QString &userId,
                                const QString &role,
                                QString *errorMessage)
{
    if (userId.trimmed().isEmpty()) {
        *errorMessage = "User ID is required";
        return false;
    }

    QSqlQuery query(m_database);

    if (!query.prepare(
            "UPDATE users SET role = :role "
            "WHERE user_id = :user_id AND is_active = 1")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    query.bindValue(":role", role);
    query.bindValue(":user_id", userId.trimmed());

    if (!query.exec()) {
        *errorMessage = query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() != 1) {
        *errorMessage = "Active user not found";
        return false;
    }

    return true;
}
bool UserRepository::deactivateUser(const QString &userId,
                                    QString *errorMessage)
{
    if (userId.trimmed().isEmpty()) {
        *errorMessage = "User ID is required";
        return false;
    }

    QSqlQuery query(m_database);

    if (!query.prepare(
            "UPDATE users SET is_active = 0 "
            "WHERE user_id = :user_id AND is_active = 1")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    query.bindValue(":user_id", userId.trimmed());

    if (!query.exec()) {
        *errorMessage = query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() != 1) {
        *errorMessage = "Active user not found";
        return false;
    }

    return true;
}