#include "processeventcoordinator.h"

#include <QObject>

#include "eventmanager.h"
#include "mixingtank.h"
#include "pump.h"
#include "valve.h"
#include "mixer.h"
#include "operatorsession.h"
#include "batchcontroller.h"

ProcessEventCoordinator::ProcessEventCoordinator(EventManager &events)
    : m_events(events)
{
}

void ProcessEventCoordinator::watchTemperature(MixingTank &tank)
{
    EventManager *events = &m_events;

    QObject::connect(&tank, &MixingTank::temperatureChanged, events,
        [&tank, events, previousTemperature = tank.temperature()]() mutable
        {
            const double temperature = tank.temperature();

            if (previousTemperature < 90.0 && temperature >= 90.0) {
                events->addWarning("TK1", "Temperature reached 90 °C");
            } else if (previousTemperature > 25.0
                       && temperature <= 25.0) {
                events->addInfo("TK1", "TEMP too low. TK1 in heating mode (simulated)");
            }

            previousTemperature = temperature;
        }
    );
}
void ProcessEventCoordinator::watchFaults(
    Pump &pump1, Valve &valve1,
    Pump &pump2, Valve &valve2,
    Pump &pump3, Valve &valve3,
    Mixer &mixer, MixingTank &tank,
    Valve &valve4)
{
    EventManager *events = &m_events;

    auto watchFault = [events](auto &device,
                               auto stateChangedSignal,
                               const QString &tag,
                               auto faultState)
    {
        auto *devicePtr = &device;

        QObject::connect(devicePtr, stateChangedSignal, events,
            [devicePtr, events, tag, faultState, wasFault = (device.state() == faultState)]() mutable
            {
                const bool isFault = devicePtr->state() == faultState;

                if (isFault == wasFault)
                    return;

                wasFault = isFault;

                if (isFault)
                    events->addAlarm(tag, "Device fault detected");
                else
                    events->addInfo(tag, "Device fault cleared");
            }
        );
    };

    watchFault(pump1, &Pump::stateChanged, "P1", Pump::State::Fault);
    watchFault(valve1, &Valve::stateChanged, "V1", Valve::State::Fault);
    watchFault(pump2, &Pump::stateChanged, "P2", Pump::State::Fault);
    watchFault(valve2, &Valve::stateChanged, "V2", Valve::State::Fault);
    watchFault(pump3, &Pump::stateChanged, "P3", Pump::State::Fault);
    watchFault(valve3, &Valve::stateChanged, "V3", Valve::State::Fault);
    watchFault(mixer, &Mixer::stateChanged, "M1", Mixer::State::Fault);
    watchFault(tank, &MixingTank::stateChanged, "TK1", MixingTank::State::Fault);
    watchFault(valve4, &Valve::stateChanged, "V4", Valve::State::Fault);
}

void ProcessEventCoordinator::watchRpmChanges(
    Pump &pump1, Pump &pump2, Pump &pump3,Mixer &mixer, OperatorSession &session)
{
    EventManager *events = &m_events;
    OperatorSession *sessionPtr = &session;

    auto watchPumpRpm = [events, sessionPtr](Pump &device, const QString &tag)
    {
        Pump *pump = &device;

        QObject::connect(pump, &Pump::targetRpmChanged, sessionPtr,
            [pump, events, sessionPtr, tag, previousRpm = pump->targetRpm()]() mutable
            {
                const double newRpm = pump->targetRpm();
                const QString operatorName = sessionPtr->selected()
                                                 ? sessionPtr->operatorName()
                                                 : "SYSTEM";

                events->addInfo(tag,QString("Target RPM changed: %1 → %2 RPM by %3")
                        .arg(previousRpm, 0, 'f', 0)
                        .arg(newRpm, 0, 'f', 0)
                        .arg(operatorName));

                previousRpm = newRpm;
            }
        );
    };

    watchPumpRpm(pump1, "P1");
    watchPumpRpm(pump2, "P2");
    watchPumpRpm(pump3, "P3");

    Mixer *mixerPtr = &mixer;

    QObject::connect(mixerPtr, &Mixer::targetRpmChanged, events,
        [mixerPtr, events, previousRpm = mixerPtr->targetRpm()]() mutable
        {
            const double newRpm = mixerPtr->targetRpm();

            QString reason = "process control";

            if (mixerPtr->state() == Mixer::State::Fault)
                reason = "mixer fault";
            else if (!mixerPtr->isRunning())
                reason = "mixer stopped";

            events->addInfo("M1", QString("Target RPM changed: %1 → %2 RPM (%3)")
                    .arg(previousRpm, 0, 'f', 0)
                    .arg(newRpm, 0, 'f', 0)
                    .arg(reason));

            previousRpm = newRpm;
        }
    );
}
void ProcessEventCoordinator::watchStageCompletion(
    BatchController &controller)
{
    EventManager *events = &m_events;

    QObject::connect(&controller, &BatchController::stageCompleted, events,
        [events](BatchController::State completedStage)
        {
            switch (completedStage) {
            case BatchController::State::FillingWater:
                events->addInfo("BATCH","Water filling completed. Next: concentrate dosing");
                break;

            case BatchController::State::DosingConcentrate:
                events->addInfo("BATCH","Concentrate dosing completed. Next: temperature check");
                break;

            case BatchController::State::TemperatureCheck:
                events->addInfo("BATCH","Temperature check passed. Next: mixing");
                break;

            case BatchController::State::Mixing:
                events->addInfo("BATCH", "Mixing completed. Ready for transfer");
                break;

            case BatchController::State::Transferring:
                events->addInfo("BATCH", "Transfer completed. Batch complete");
                break;

            default:
                break;
            }
        }
    );
}
void ProcessEventCoordinator::watchBatchState(BatchController &controller, MixingTank &tank, OperatorSession &session)
{
    BatchController *controllerPtr = &controller;
    MixingTank *tankPtr = &tank;
    OperatorSession *sessionPtr = &session;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &BatchController::stateChanged, sessionPtr,
        [controllerPtr, tankPtr, sessionPtr, events, previousState = controllerPtr->state()]() mutable
        {
            using State = BatchController::State;

            const State currentState = controllerPtr->state();

            if (currentState == previousState)  return;

            const State oldState = previousState;
            previousState = currentState;

            if (currentState == State::TemperatureCheck
                && tankPtr->temperature() <= 25.0) {
                events->addInfo( "TK1", "TEMP too low. TK1 in heating mode (simulated)");
            }

            if (oldState == State::Idle
                && currentState == State::FillingWater) {
                events->addInfo( "BATCH", "Batch started by " + sessionPtr->operatorName());
            } else if (currentState == State::Draining) {
                const bool resumedDrain =
                    oldState == State::Paused
                    && controllerPtr->stopReason()
                           == BatchController::StopReason::DrainValveUnavailable;

                events->addWarning("BATCH",QString(resumedDrain ? "Drain resumed by " : "Drain started by ")
                        + sessionPtr->operatorName());
            } else if (oldState == State::Draining
                       && currentState == State::Idle) {
                events->addInfo("BATCH", "Drain completed");
            } else if (currentState == State::Paused) {
                if (controllerPtr->stopReason()
                    == BatchController::StopReason::OperatorPause) {
                    events->addInfo("BATCH", "Batch paused by " + sessionPtr->operatorName());
                } else {
                    events->addWarning( "BATCH", "Batch paused: " + controllerPtr->stopReasonText());
                }
            } else if (oldState == State::Paused
                       && currentState != State::Idle) {
                events->addInfo( "BATCH",  "Batch resumed by " + sessionPtr->operatorName());
            }
        }
    );
}

void ProcessEventCoordinator::watchBatchId(BatchController &controller)
{
    BatchController *controllerPtr = &controller;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &BatchController::batchIdChanged, events,
        [controllerPtr, events]() { events->setBatchId(controllerPtr->batchId()); }
    );
}
void ProcessEventCoordinator::watchMixerConnection(Mixer &mixer)
{
    EventManager *events = &m_events;
    Mixer *mixerPtr = &mixer;

    QObject::connect(mixerPtr, &Mixer::connectionChanged, events,
                     [mixerPtr, events]()
                     {
                         events->addInfo(
                             "M1",
                             mixerPtr->isConnected()
                                 ? "Mixer connection detected (simulated)"
                                 : "Mixer connection lost (simulated)");
                     }
    );
}