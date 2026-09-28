#include "processcontroller.h"

#include "pump.h"
#include "valve.h"
#include "eventmanager.h"


ProcessController::ProcessController(Pump *pump,
                                     Valve *valve,
                                     EventManager *eventManager,
                                     QObject *parent)
    : QObject{parent}
    , m_pump(pump)
    , m_valve(valve)
    , m_eventManager(eventManager)
{
    Q_ASSERT(pump != nullptr);
    Q_ASSERT(valve != nullptr);

    connect(m_valve, &Valve::stateChanged,
            this, &ProcessController::onValveStateChanged);

    connect(m_pump, &Pump::temperatureFromSensorChanged,
            this, &ProcessController::onTemperatureChanged);
    updateStartBlockedReason();

}
void ProcessController::startPump()
{
    if (m_pump == nullptr || m_valve == nullptr)
    {
        return;
    }

    updateStartBlockedReason();

    if (!m_valve->isOpen())
    {
        return;
    }

    if (m_pump->temperatureFromSensor() > MaxTemperature)
    {
        return;
    }
    if (m_pump->state() == Pump::State::Fault)
    {
        return;
    }
    if (m_pump->state() != Pump::State::Running)
    {
        m_pump->start();

        if (m_eventManager != nullptr)
        {
            m_eventManager->addInfo(
                QString("Pump %1").arg(m_pump->id()),
                "Pump started");
        }
    }
    updateStartBlockedReason();
}

void ProcessController::stopPump()
{
    if (m_pump == nullptr)
    {
        return;
    }

    if (m_pump->state() == Pump::State::Running)
    {
        m_pump->stop();

        if (m_eventManager != nullptr)
        {
            m_eventManager->addInfo(
                QString("Pump %1").arg(m_pump->id()),
                "Pump stopped");

        }
    }
    updateStartBlockedReason();
}

void ProcessController::onValveStateChanged()
{
    if (m_valve == nullptr || m_pump == nullptr)
    {
        return;
    }

    if (!m_valve->isOpen())
    {
        if (m_eventManager != nullptr)
        {
            m_eventManager->addInfo(
                QString("Valve %1").arg(m_valve->id()),
                "Valve closed");
        }

        if (m_pump->state() == Pump::State::Running)
        {
            m_pump->stop();

            if (m_eventManager != nullptr)
            {
                m_eventManager->addInfo(
                    QString("Pump %1").arg(m_pump->id()),
                    "Pump stopped");
            }
        }
    }
    else
    {
        if (m_eventManager != nullptr)
        {
            m_eventManager->addInfo(
                QString("Valve %1").arg(m_valve->id()),
                "Valve opened");
        }
    }

    updateStartBlockedReason();
}

void ProcessController::onTemperatureChanged()
{
    if (m_pump == nullptr)
    {
        return;
    }

    if (m_pump->temperatureFromSensor() > MaxTemperature)
    {
        if (m_pump->state() != Pump::State::Fault)
        {
            m_pump->setFault();

            if (m_eventManager != nullptr)
            {
                m_eventManager->addAlarm(
                    QString("Pump %1").arg(m_pump->id()),
                    "High temperature");
            }
        }
    }

    updateStartBlockedReason();
}

QString ProcessController::startBlockedReason() const
{
    return m_startBlockedReason;
}

void ProcessController::updateStartBlockedReason()
{
    QString reason;

    if (m_pump == nullptr || m_valve == nullptr)
    {
        reason = "CONTROLLER ERROR";
    }
    else if (m_pump->temperatureFromSensor() > MaxTemperature)
    {
        reason = "HIGH TEMPERATURE";
    }
    else if (!m_valve->isOpen())
    {
        reason = QString("VALVE %1 CLOSED").arg(m_valve->id());
    }

    if (reason == m_startBlockedReason)
    {
        return;
    }

    m_startBlockedReason = reason;
    emit startBlockedReasonChanged();
}
void ProcessController::setPumpFault()
{
    if (m_pump == nullptr)
        return;

    if (m_pump->state() != Pump::State::Fault)
    {
        m_pump->setFault();

        if (m_eventManager != nullptr)
        {
            m_eventManager->addAlarm(
                QString("Pump %1").arg(m_pump->id()),
                "Pump fault");
        }
    }
}
void ProcessController::resetPumpFault()
{
    if (m_pump == nullptr)
        return;

    if (m_pump->state() == Pump::State::Fault)
    {
        m_pump->resetFault();

        if (m_eventManager != nullptr)
        {
            m_eventManager->addInfo(
                QString("Pump %1").arg(m_pump->id()),
                "Pump fault reset");
        }
    }

    updateStartBlockedReason();
}