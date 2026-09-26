#include "operatorsession.h"

OperatorSession::OperatorSession(QObject *parent)
    : QObject(parent)
{
}

QString OperatorSession::operatorName() const
{
    return m_operatorName;
}

bool OperatorSession::selected() const
{
    return !m_operatorName.isEmpty();
}

void OperatorSession::selectOperator(const QString &name)
{
    const QString normalizedName = name.trimmed();

    if (m_operatorName == normalizedName)
        return;

    m_operatorName = normalizedName;
    emit operatorNameChanged();
}