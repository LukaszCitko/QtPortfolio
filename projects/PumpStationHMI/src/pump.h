#ifndef PUMP_H
#define PUMP_H

#include <QObject>

class Pump : public QObject
{
private:
    Q_OBJECT


public:

    explicit Pump(QObject *parent = nullptr);

    enum class State
    {
        Stopped,
        Running,
        Fault
    };

    void start();
    void stop();
    void setFault();
    bool isRunning() const;
    void resetFault();
    State state() const;

private:
    State m_state;

signals:
};

#endif // PUMP_H
