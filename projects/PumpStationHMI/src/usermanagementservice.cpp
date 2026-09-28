#include "usermanagementservice.h"

#include "accesspolicy.h"
#include "database/userlistmodel.h"
#include "database/userrepository.h"
#include "eventmanager.h"
#include "operatorsession.h"

UserManagementService::UserManagementService(
    OperatorSession &session,
    UserRepository &repository,
    UserListModel &users,
    EventManager &events,
    QObject *parent)
    : QObject(parent),
    m_session(session),
    m_repository(repository),
    m_users(users),
    m_events(events)
{
}

QString UserManagementService::lastError() const
{
    return m_lastError;
}

void UserManagementService::setLastError(const QString &message)
{
    if (m_lastError == message)
        return;

    m_lastError = message;
    emit lastErrorChanged();
}

bool UserManagementService::addUser(const QString &displayName,
                                    const QString &role)
{
    setLastError({});

    if (!m_session.canManageUsers()) {
        setLastError("Admin role required");
        m_events.addWarning("ACCESS", "User creation denied");
        return false;
    }

    const QString selectedRole = role.trimmed().toUpper();

    if (AccessPolicy::roleFromText(selectedRole) == AccessPolicy::Role::None) {
        setLastError("Invalid user role");
        return false;
    }

    QString error;

    if (!m_repository.addUser(displayName, selectedRole, &error)) {
        setLastError(error);
        return false;
    }

    m_events.addInfo(
        "USERS",
        "User added: " + displayName.trimmed()
            + " (" + selectedRole + ") by " + m_session.operatorName());

    if (!m_users.reload(&error)) {
        setLastError("User saved, but list refresh failed: " + error);
        return false;
    }

    return true;
}

bool UserManagementService::changeRole(const QString &userId,
                                       const QString &role)
{
    setLastError({});

    if (!m_session.canManageUsers()) {
        setLastError("Admin role required");
        m_events.addWarning("ACCESS", "User role change denied");
        return false;
    }

    if (userId == m_session.operatorId()) {
        setLastError("Cannot change your own role");
        return false;
    }

    const QString selectedRole = role.trimmed().toUpper();

    if (AccessPolicy::roleFromText(selectedRole) == AccessPolicy::Role::None) {
        setLastError("Invalid user role");
        return false;
    }

    QString error;

    if (!m_repository.changeRole(userId, selectedRole, &error)) {
        setLastError(error);
        return false;
    }

    m_events.addInfo(
        "USERS",
        "Role changed for user " + userId
            + " to " + selectedRole
            + " by " + m_session.operatorName());

    if (!m_users.reload(&error)) {
        setLastError("Role saved, but list refresh failed: " + error);
        return false;
    }

    return true;
}
bool UserManagementService::removeUser(const QString &userId)
{
    setLastError({});

    if (!m_session.canManageUsers()) {
        setLastError("Admin role required");
        m_events.addWarning("ACCESS", "User removal denied");
        return false;
    }

    if (userId == m_session.operatorId()) {
        setLastError("Cannot remove your own user");
        return false;
    }

    QString error;

    if (!m_repository.deactivateUser(userId, &error)) {
        setLastError(error);
        return false;
    }

    m_events.addInfo(
        "USERS",
        "User removed: " + userId
            + " by " + m_session.operatorName());

    if (!m_users.reload(&error)) {
        setLastError("User removed, but list refresh failed: " + error);
        return false;
    }

    return true;
}