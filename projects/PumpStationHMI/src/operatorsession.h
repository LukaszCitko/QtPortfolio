#ifndef OPERATORSESSION_H
#define OPERATORSESSION_H

#include <QObject>
#include <QString>

class OperatorSession : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString operatorName READ operatorName NOTIFY operatorNameChanged)
    Q_PROPERTY(bool selected READ selected NOTIFY operatorNameChanged)

public:
    explicit OperatorSession(QObject *parent = nullptr);

    QString operatorName() const;
    bool selected() const;

    Q_INVOKABLE void selectOperator(const QString &name);

signals:
    void operatorNameChanged();

private:
    QString m_operatorName;
};

#endif