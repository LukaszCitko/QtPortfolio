#include "faultresetservice.h"

#include "eventmanager.h"
#include "mixer.h"
#include "operatorsession.h"
#include "pump.h"

FaultResetService::FaultResetService(OperatorSession &session,
                                     Pump &pump1,
                                     Mixer &mixer,
                                     EventManager &events,
                                     QObject *parent)
    : QObject(parent),
    m_session(session),
    m_pump1(pump1),
    m_mixer(mixer),
    m_events(events)
{
}

bool FaultResetService::resetPump1()
{
    if (!m_session.canResetFault()) {
        m_events.addWarning("ACCESS", "P1 fault reset denied");
        return false;
    }

    if (m_pump1.state() != Pump::State::Fault)
        return false;

    m_pump1.resetFault();
    m_events.addInfo("P1", "Fault reset by " + m_session.operatorName());
    return true;
}

bool FaultResetService::resetMixer()
{
    if (!m_session.canResetFault()) {
        m_events.addWarning("ACCESS", "M1 fault reset denied");
        return false;
    }

    if (m_mixer.state() != Mixer::State::Fault)
        return false;

    m_mixer.resetFault();
    m_events.addInfo("M1", "Fault reset by " + m_session.operatorName());
    return true;
}