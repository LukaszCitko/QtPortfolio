#ifndef FAULTRESETSERVICE_H
#define FAULTRESETSERVICE_H

#include <QObject>

class OperatorSession;
class Pump;
class Mixer;
class EventManager;

class FaultResetService : public QObject
{
    Q_OBJECT

public:
    FaultResetService(OperatorSession &session,
                      Pump &pump1,
                      Mixer &mixer,
                      EventManager &events,
                      QObject *parent = nullptr);

    Q_INVOKABLE bool resetPump1();
    Q_INVOKABLE bool resetMixer();

private:
    OperatorSession &m_session;
    Pump &m_pump1;
    Mixer &m_mixer;
    EventManager &m_events;
};

#endif