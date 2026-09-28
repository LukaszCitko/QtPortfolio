#ifndef OPERATORSESSION_H
#define OPERATORSESSION_H

#include <QObject>
#include <QString>

class OperatorSession : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString operatorId READ operatorId NOTIFY operatorChanged)
    Q_PROPERTY(QString operatorName READ operatorName NOTIFY operatorChanged)
    Q_PROPERTY(bool selected READ selected NOTIFY operatorChanged)

public:
    explicit OperatorSession(QObject *parent = nullptr);

    QString operatorId() const;
    QString operatorName() const;
    bool selected() const;

    Q_INVOKABLE void selectOperator(const QString &id,
                                    const QString &name);

signals:
    void operatorChanged();

private:
    QString m_operatorId;
    QString m_operatorName;
};

#endif