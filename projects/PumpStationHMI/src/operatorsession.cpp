#include "operatorsession.h"

OperatorSession::OperatorSession(QObject *parent)
    : QObject(parent)
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

bool OperatorSession::selected() const
{
    return !m_operatorId.isEmpty() && !m_operatorName.isEmpty();
}

void OperatorSession::selectOperator(const QString &id,
                                     const QString &name)
{
    const QString normalizedId = id.trimmed();
    const QString normalizedName = name.trimmed();

    if (normalizedId.isEmpty() || normalizedName.isEmpty())
        return;

    if (m_operatorId == normalizedId
        && m_operatorName == normalizedName) {
        return;
    }

    m_operatorId = normalizedId;
    m_operatorName = normalizedName;
    emit operatorChanged();
}