#include "operatorsession.h"
#include "accesspolicy.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

OperatorSession::OperatorSession(const QSqlDatabase &database, QObject *parent)
    : QObject(parent),
    m_database(database)
{
}

QString OperatorSession::operatorId() const
{
    return m_operatorId;
}

QString OperatorSession::operatorName() const
{
    return m_operatorName;
}

QString OperatorSession::operatorRole() const
{
    return m_operatorRole;
}

bool OperatorSession::selected() const
{
    return !m_operatorId.isEmpty();
}

bool OperatorSession::selectOperator(const QString &id)
{
    const QString userId = id.trimmed();

    if (userId.isEmpty())
        return false;

    QSqlQuery query(m_database);

    if (!query.prepare(
            "SELECT display_name, role FROM users "
            "WHERE user_id = :user_id AND is_active = 1")) {
        qWarning() << "Cannot prepare user selection:"
                   << query.lastError().text();
        return false;
    }

    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qWarning() << "Cannot select user:" << query.lastError().text();
        return false;
    }

    if (!query.next())
        return false;

    const QString name = query.value(0).toString();
    const QString role = query.value(1).toString();

    if (m_operatorId == userId
        && m_operatorName == name
        && m_operatorRole == role) {
        return true;
    }

    m_operatorId = userId;
    m_operatorName = name;
    m_operatorRole = role;
    emit operatorChanged();
    return true;
}
bool OperatorSession::canControlBatch() const
{
    const auto role = AccessPolicy::roleFromText(m_operatorRole);
    return AccessPolicy::canControlBatch(role);
}

bool OperatorSession::canResetFault() const
{
    const auto role = AccessPolicy::roleFromText(m_operatorRole);
    return AccessPolicy::canResetFault(role);
}

bool OperatorSession::canManageUsers() const
{
    const auto role = AccessPolicy::roleFromText(m_operatorRole);
    return AccessPolicy::canManageUsers(role);
}
bool OperatorSession::canApproveDrain() const
{
    const auto role = AccessPolicy::roleFromText(m_operatorRole);
    return AccessPolicy::canApproveDrain(role);
}