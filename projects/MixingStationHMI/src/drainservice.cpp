#include "drainservice.h"

#include "accesspolicy.h"
#include "batchcontroller.h"
#include "eventmanager.h"
#include "mixingtank.h"
#include "operatorsession.h"
#include "valve.h"

DrainService::DrainService(OperatorSession &session,
                           BatchController &controller,
                           MixingTank &tank,
                           Valve &drainValve,
                           EventManager &events,
                           QObject *parent)
    : QObject(parent),
    m_session(session),
    m_controller(controller),
    m_tank(tank),
    m_drainValve(drainValve),
    m_events(events)
{
}

bool DrainService::requestDrain()
{
    const auto role =
        AccessPolicy::roleFromText(m_session.operatorRole());

    if (!AccessPolicy::canApproveDrain(role)) {
        m_events.addWarning(
            "ACCESS",
            "Drain denied: technician authorization required");
        return false;
    }

    return startDrain();
}

bool DrainService::demoRequestDrain()
{
    const auto role =
        AccessPolicy::roleFromText(m_session.operatorRole());

    if (role != AccessPolicy::Role::Operator)
        return false;

    if (!startDrain())
        return false;

    m_events.addWarning(
        "V4",
        "Simulation override: drain requested by "
            + m_session.operatorName());

    return true;
}

bool DrainService::startDrain()
{
    if (m_tank.volume() <= 0.0
        || m_tank.state() == MixingTank::State::Fault
        || m_drainValve.state() != Valve::State::Closed
        || m_controller.state() == BatchController::State::Draining) {
        m_events.addWarning("V4", "Drain unavailable");
        return false;
    }

    m_controller.emergencyDrain();

    if (m_controller.state() != BatchController::State::Draining) {
        m_events.addWarning("V4", "Drain could not start");
        return false;
    }

    return true;
}