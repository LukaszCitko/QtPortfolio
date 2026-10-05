#ifndef DRAINSERVICE_H
#define DRAINSERVICE_H

#include <QObject>

class OperatorSession;
class BatchController;
class MixingTank;
class Valve;
class EventManager;

class DrainService : public QObject
{
    Q_OBJECT

public:
    DrainService(OperatorSession &session,
                 BatchController &controller,
                 MixingTank &tank,
                 Valve &drainValve,
                 EventManager &events,
                 QObject *parent = nullptr);

    Q_INVOKABLE bool requestDrain();
    Q_INVOKABLE bool demoRequestDrain();

private:

    bool startDrain();

    OperatorSession &m_session;
    BatchController &m_controller;
    MixingTank &m_tank;
    Valve &m_drainValve;
    EventManager &m_events;
};

#endif