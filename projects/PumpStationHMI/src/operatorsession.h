#ifndef OPERATORSESSION_H
#define OPERATORSESSION_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>

class OperatorSession : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString operatorId READ operatorId NOTIFY operatorChanged)
    Q_PROPERTY(QString operatorName READ operatorName NOTIFY operatorChanged)
    Q_PROPERTY(QString operatorRole READ operatorRole NOTIFY operatorChanged)
    Q_PROPERTY(bool selected READ selected NOTIFY operatorChanged)
    Q_PROPERTY(bool canControlBatch READ canControlBatch NOTIFY operatorChanged)
    Q_PROPERTY(bool canResetFault READ canResetFault NOTIFY operatorChanged)
    Q_PROPERTY(bool canManageUsers READ canManageUsers NOTIFY operatorChanged)

public:
    explicit OperatorSession(const QSqlDatabase &database, QObject *parent = nullptr);

    QString operatorRole() const;

    Q_INVOKABLE bool selectOperator(const QString &id);
    QString operatorId() const;
    QString operatorName() const;
    bool selected() const;
    bool canControlBatch() const;
    bool canResetFault() const;
    bool canManageUsers() const;
    bool canApproveDrain() const;



signals:
    void operatorChanged();

private:
    QString m_operatorId;
    QString m_operatorName;
    QSqlDatabase m_database;
    QString m_operatorRole;
};

#endif