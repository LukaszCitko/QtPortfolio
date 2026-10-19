
#include <QObject>

#include "batchcontroller.h"
#include "eventmanager.h"
#include "mixer.h"
#include "mixingtank.h"
#include "operatorsession.h"
#include "processconfiguration.h"
#include "processeventcoordinator.h"
#include "pump.h"
#include "valve.h"
#include "communication/remoteprocesscontroller.h"
#include "controllerprotocol.h"
#include "communication/modbuscontrollerclient.h"

ProcessEventCoordinator::ProcessEventCoordinator(EventManager &events)
    : m_events(events)
{
}

void ProcessEventCoordinator::watchTemperature(MixingTank &tank)
{
    EventManager *events = &m_events;
    const double minimumTemperature = static_cast<double>(ProcessConfiguration::MinimumProcessTemperatureTenthsCelsius) / 10.0;
    const double maximumTemperature = static_cast<double>(ProcessConfiguration::MaximumProcessTemperatureTenthsCelsius) / 10.0;

    QObject::connect(&tank, &MixingTank::temperatureChanged, events,
                     [&tank, events, minimumTemperature, maximumTemperature, previousTemperature = tank.temperature()]() mutable
                     {
                         const double temperature = tank.temperature();

                         if (previousTemperature <= maximumTemperature && temperature > maximumTemperature)
                         {
                             events->addWarning("TK1", QString("Temperature above process limit: %1 °C").arg(temperature, 0, 'f', 1));
                         }
                         else if (previousTemperature >= minimumTemperature && temperature < minimumTemperature)
                         {
                             events->addInfo("TK1", "Temperature too low. TK1 heating mode activated");
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
                events->addInfo("BATCH", "Batch " + controllerPtr->batchId() + " started by " + sessionPtr->operatorName());
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

    QObject::connect(controllerPtr, &BatchController::batchIdChanged, events, [controllerPtr, events]() { events->setBatchId(controllerPtr->batchId()); });
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
                                 ? "Mixer connection detected"
                                 : "Mixer connection lost");
                     }
    );
}
void ProcessEventCoordinator::watchBatchId(RemoteProcessController &controller)
{
    RemoteProcessController *controllerPtr = &controller;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &RemoteProcessController::batchIdChanged, events, [controllerPtr, events]()
                    {
                        events->setBatchId(controllerPtr->batchId());
                    }
    );
}

void ProcessEventCoordinator::watchBatchState(RemoteProcessController &controller, OperatorSession &session)
{
    RemoteProcessController *controllerPtr = &controller;
    OperatorSession *sessionPtr = &session;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &RemoteProcessController::stateChanged, events, [controllerPtr, sessionPtr, events, previousState = controllerPtr->state()]() mutable
                    {
                        const int currentState = controllerPtr->state();
                        const int oldState = previousState;

                        if (currentState == oldState)
                        {
                            return;
                        }

                        previousState = currentState;

                        const int idleState = static_cast<int>(ControllerProtocol::ProcessState::Idle);
                        const int fillingState = static_cast<int>(ControllerProtocol::ProcessState::FillingWater);
                        const int pausedState = static_cast<int>(ControllerProtocol::ProcessState::Paused);
                        const int drainingState = static_cast<int>(ControllerProtocol::ProcessState::Draining);
                        const int completeState = static_cast<int>(ControllerProtocol::ProcessState::Complete);

                        if (oldState == idleState && currentState == fillingState)
                        {
                            events->addInfo("BATCH", "Batch " + controllerPtr->batchId() + " started by " + sessionPtr->operatorName());
                        }
                        else if (currentState == drainingState)
                        {
                            const QString action = oldState == pausedState ? QStringLiteral("resumed") : QStringLiteral("started");
                            events->addWarning("V4", QStringLiteral("Hardware drain %1 by %2").arg(action, sessionPtr->operatorName()));
                        }
                        else if (oldState == drainingState && currentState == completeState)
                        {
                            events->addInfo("V4", QStringLiteral("Drain %1 completed").arg(controllerPtr->batchId()));
                        }
                        else if (currentState == pausedState)
                        {
                            events->addWarning("BATCH", "Hardware batch paused");
                        }
                        else if (oldState == pausedState)
                        {
                            events->addInfo("BATCH", "Hardware batch resumed by " + sessionPtr->operatorName());
                        }
                    }
    );
}

void ProcessEventCoordinator::watchStageCompletion(RemoteProcessController &controller)
{
    RemoteProcessController *controllerPtr = &controller;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &RemoteProcessController::stateChanged, events, [controllerPtr, events, previousState = controllerPtr->state()]() mutable
                     {
                        const int currentState = controllerPtr->state();
                        const int oldState = previousState;

                        if (currentState == oldState)
                        {
                            return;
                        }

                        previousState = currentState;

                        using State = ControllerProtocol::ProcessState;

                        if (oldState == static_cast<int>(State::FillingWater) && currentState == static_cast<int>(State::DosingConcentrate))
                        {
                            events->addInfo("BATCH", "Water filling completed. Next: concentrate dosing");
                        }
                        else if (oldState == static_cast<int>(State::DosingConcentrate) && currentState == static_cast<int>(State::TemperatureCheck))
                        {
                            events->addInfo("BATCH", "Concentrate dosing completed. Next: temperature check");
                        }
                        else if (oldState == static_cast<int>(State::TemperatureCheck) && currentState == static_cast<int>(State::Mixing))
                        {
                            events->addInfo("BATCH", "Temperature check passed. Next: mixing");
                        }
                        else if (oldState == static_cast<int>(State::Mixing) && currentState == static_cast<int>(State::ReadyForTransfer))
                        {
                            events->addInfo("BATCH", "Mixing completed. Ready for transfer");
                        }
                        else if (oldState == static_cast<int>(State::Transferring) && currentState == static_cast<int>(State::Complete))
                        {
                            events->addInfo("BATCH", "Transfer completed. Batch complete");
                        }
                    }
    );
}

void ProcessEventCoordinator::watchConnection(RemoteProcessController &controller)
{
    RemoteProcessController *controllerPtr = &controller;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &RemoteProcessController::initialSnapshotReceived, events, [controllerPtr, events]()
                    {
                        const QString runId = controllerPtr->batchId();
                        const QString message = runId.isEmpty()
                                                    ? QStringLiteral("HMI connected to controller")
                                                    : QStringLiteral("HMI connected · active run %1 synchronized").arg(runId);

                        events->addInfo(QStringLiteral("CONTROLLER"), message);
                    }
    );

    QObject::connect(controllerPtr, &RemoteProcessController::connectionLost, events, [controllerPtr, events]()
                    {
                        const QString runId = controllerPtr->batchId();
                        const QString message = runId.isEmpty()
                                                    ? QStringLiteral("Controller communication lost")
                                                    : QStringLiteral("Controller communication lost · active run %1 status unavailable").arg(runId);

                        events->addWarning(QStringLiteral("CONTROLLER"), message);
                    }
    );

    QObject::connect(controllerPtr, &RemoteProcessController::connectionRestored, events, [controllerPtr, events]()
                    {
                        const QString runId = controllerPtr->batchId();
                        const QString message = runId.isEmpty()
                                                    ? QStringLiteral("Controller connection restored")
                                                    : QStringLiteral("Controller connection restored · active run %1 synchronized").arg(runId);

                        events->addInfo(QStringLiteral("CONTROLLER"), message);
                    }
    );
}

void ProcessEventCoordinator::watchRecovery(RemoteProcessController &controller, OperatorSession &session)
{
    RemoteProcessController *controllerPtr = &controller;
    OperatorSession *sessionPtr = &session;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &RemoteProcessController::recoveryRequiredChanged, events, [controllerPtr, sessionPtr, events]()
                    {
                        if (controllerPtr->recoveryRequired())
                        {
                            const QString runId = controllerPtr->batchId();
                            const QString message = runId.isEmpty()
                                                        ? QStringLiteral("Active run interrupted by controller restart. Tank state is unknown")
                                                        : QStringLiteral("Run %1 interrupted by controller restart. Tank state is unknown").arg(runId);
                            events->addAlarm(QStringLiteral("RECOVERY"), message);
                            return;
                        }

                        const QString operatorName = sessionPtr->selected() ? sessionPtr->operatorName() : QStringLiteral("unknown user");
                        events->addInfo(QStringLiteral("RECOVERY"), QStringLiteral("Tank mechanically emptied. Recovery completed by %1").arg(operatorName));
                    }
    );
}

void ProcessEventCoordinator::watchCommandResults(ModbusControllerClient &client)
{
    EventManager *events = &m_events;

    const auto commandName = [](quint16 commandCode)
    {
        switch (static_cast<ControllerProtocol::Command>(commandCode))
        {
        case ControllerProtocol::Command::StartBatch:
            return QStringLiteral("Start batch");
        case ControllerProtocol::Command::PauseBatch:
            return QStringLiteral("Pause batch");
        case ControllerProtocol::Command::ResumeBatch:
            return QStringLiteral("Resume batch");
        case ControllerProtocol::Command::StartTransfer:
            return QStringLiteral("Start transfer");
        case ControllerProtocol::Command::ResetFaults:
            return QStringLiteral("Reset faults");
        case ControllerProtocol::Command::StartDrain:
            return QStringLiteral("Start drain");
        case ControllerProtocol::Command::PrepareNextBatch:
            return QStringLiteral("Prepare next batch");
        case ControllerProtocol::Command::None:
            return QStringLiteral("Unknown");
        }

        return QStringLiteral("Unknown");
    };

    const auto resultDescription = [](int result)
    {
        switch (static_cast<ControllerProtocol::CommandResult>(result))
        {
        case ControllerProtocol::CommandResult::DuplicateSequence:
            return QStringLiteral("duplicate command sequence");
        case ControllerProtocol::CommandResult::InvalidState:
            return QStringLiteral("invalid controller state");
        case ControllerProtocol::CommandResult::UnsupportedCommand:
            return QStringLiteral("unsupported command");
        case ControllerProtocol::CommandResult::None:
            return QStringLiteral("no result returned");
        case ControllerProtocol::CommandResult::Accepted:
            return QStringLiteral("accepted");
        }

        return QStringLiteral("unknown result");
    };

    QObject::connect(&client, &ModbusControllerClient::commandFinished, events, [events, commandName, resultDescription](quint16 commandCode, quint16 commandSequence, int result)
                     {
                         if (result == static_cast<int>(ControllerProtocol::CommandResult::Accepted))
                         {
                             return;
                         }

                         const QString message = QStringLiteral("%1 command rejected: %2 (sequence %3)").arg(commandName(commandCode)).arg(resultDescription(result)).arg(commandSequence);
                         events->addWarning(QStringLiteral("CONTROLLER"), message);
                     });
}
