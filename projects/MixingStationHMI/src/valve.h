#ifndef VALVE_H
#define VALVE_H

#include <QObject>

class Valve : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString stateText READ stateText NOTIFY stateChanged)
    Q_PROPERTY(bool open READ isOpen NOTIFY stateChanged)
    Q_PROPERTY(bool fault READ hasFault NOTIFY stateChanged)

public:
    enum class State
    {
        Closed,
        Open,
        Fault
    };

    explicit Valve(int id, QObject *parent = nullptr);
    QString stateText() const;

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();
    Q_INVOKABLE void setFault();
    void resetFault();
    int id() const;
    bool isOpen() const;
    bool hasFault() const;

    State state() const;

private:
    int m_id;
    State m_state;

signals:
     void stateChanged();
};

#endif // VALVE_H