#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

#include "pump.h"
#include "valve.h"
#include "mixer.h"
#include "mixingtank.h"
#include "batchcontroller.h"
#include "processsimulator.h"
#include "eventmanager.h"
#include "operatorsession.h"
#include "trendrecorder.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;


// Process equipment

    // Water supply
    Pump pump1(1);
    Valve valve1(1);

    // Concentrate supply
    Pump pump2(2);
    Valve valve2(2);

    // Finished product transfer
    Pump pump3(3);
    Valve valve3(3);

    // Emergency drain
    Valve valve4(4);

    // Mixing tank
    Mixer mixer;
    MixingTank mixingTank;

    EventManager eventManager;

    QObject::connect(&mixingTank, &MixingTank::temperatureChanged, &eventManager,
        [&mixingTank, &eventManager, previousTemperature = mixingTank.temperature()]() mutable
        {
            const double temperature = mixingTank.temperature();

            if (previousTemperature < 90.0 && temperature >= 90.0)
            {
                eventManager.addWarning(
                    "TK1", "Temperature reached 90 °C");
            }
            else if (previousTemperature > 25.0 && temperature <= 25.0)
            {
                eventManager.addInfo(
                    "TK1", "TEMP too low. TK1 in heating mode (simulated)");
            }

            previousTemperature = temperature;
        });

    auto watchFault = [&eventManager](auto &device,
                                      auto stateChangedSignal,
                                      const QString &tag,
                                      auto faultState)
    {
        QObject::connect(&device, stateChangedSignal,&eventManager,
            [&device, &eventManager, tag, faultState, wasFault = (device.state() == faultState)]() mutable
            {
                const bool isFault = device.state() == faultState;

                if (isFault == wasFault)
                    return;

                wasFault = isFault;

                if (isFault)
                    eventManager.addAlarm(tag, "Device fault detected");
                else
                    eventManager.addInfo(tag, "Device fault cleared");
            });
    };

    watchFault(pump1, &Pump::stateChanged, "P1", Pump::State::Fault);
    watchFault(valve1, &Valve::stateChanged, "V1", Valve::State::Fault);
    watchFault(pump2, &Pump::stateChanged, "P2", Pump::State::Fault);
    watchFault(valve2, &Valve::stateChanged, "V2", Valve::State::Fault);
    watchFault(pump3, &Pump::stateChanged, "P3", Pump::State::Fault);
    watchFault(valve3, &Valve::stateChanged, "V3", Valve::State::Fault);
    watchFault(mixer, &Mixer::stateChanged, "M1", Mixer::State::Fault);
    watchFault(mixingTank, &MixingTank::stateChanged, "TK1", MixingTank::State::Fault);
    watchFault(valve4, &Valve::stateChanged, "V4", Valve::State::Fault);


// Process services

    ProcessSimulator simulator(
        &pump1, &valve1,
        &pump2, &valve2,
        &pump3, &valve3,

        &mixingTank);

// Batch controller
    BatchController batchController(
        &pump1, &valve1,
        &pump2, &valve2,
        &pump3, &valve3,
        &valve4,
        &mixer, &mixingTank);
//Trend recorder
    TrendRecorder trendRecorder(&mixingTank);

    QObject::connect(&batchController, &BatchController::stateChanged, &trendRecorder,
        [&batchController, &trendRecorder, previousState = batchController.state()]() mutable
        {
            using State = BatchController::State;

            const State currentState = batchController.state();

            if (currentState == previousState)
                return;

            const State oldState = previousState;
            previousState = currentState;

            if ((oldState == State::Idle &&
                 currentState == State::FillingWater) ||
                (currentState == State::Draining &&
                 !trendRecorder.isRecording()))
            {
                trendRecorder.beginRun(batchController.batchId());
            }

            if (currentState == State::Complete ||
                (oldState == State::Draining &&
                 currentState == State::Idle))
            {
                trendRecorder.endRun();
            }
        });

    QTimer batchTimer;
    batchTimer.setInterval(100);

    QObject::connect(&batchTimer, &QTimer::timeout, &batchController, [&batchController](){ batchController.simulateStep(0.1); });

    batchTimer.start();


    OperatorSession operatorSession;

    auto watchTargetRpm = [&eventManager, &operatorSession](Pump &device, const QString &tag)
        {
            QObject::connect(&device, &Pump::targetRpmChanged, &eventManager,
            [&device, &eventManager, &operatorSession, tag, previousRpm = device.targetRpm()]() mutable
                {
                const double newRpm = device.targetRpm();
                const QString operatorName = operatorSession.selected()
                                                 ? operatorSession.operatorName()
                                                 : "SYSTEM";

                eventManager.addInfo(
                    tag,
                    QString("Target RPM changed: %1 → %2 RPM by %3")
                        .arg(previousRpm, 0, 'f', 0)
                        .arg(newRpm, 0, 'f', 0)
                        .arg(operatorName));

                previousRpm = newRpm;
            });
    };

    watchTargetRpm(pump1, "P1");
    watchTargetRpm(pump2, "P2");
    watchTargetRpm(pump3, "P3");

    QObject::connect(&mixer, &Mixer::targetRpmChanged, &eventManager,
        [&mixer, &eventManager, previousRpm = mixer.targetRpm()]() mutable
        {
            const double newRpm = mixer.targetRpm();

            QString reason = "process control";

            if (mixer.state() == Mixer::State::Fault)
                reason = "mixer fault";
            else if (!mixer.isRunning())
                reason = "mixer stopped";

            eventManager.addInfo(
                "M1",
                QString("Target RPM changed: %1 → %2 RPM (%3)")
                    .arg(previousRpm, 0, 'f', 0)
                    .arg(newRpm, 0, 'f', 0)
                    .arg(reason));

            previousRpm = newRpm;
        }
    );

    QObject::connect(&batchController, &BatchController::stageCompleted, &eventManager,
                     [&eventManager](BatchController::State completedStage)
                     {
                         switch (completedStage) {
                         case BatchController::State::FillingWater:
                             eventManager.addInfo(
                                 "BATCH",
                                 "Water filling completed. Next: concentrate dosing");
                             break;

                         case BatchController::State::DosingConcentrate:
                             eventManager.addInfo(
                                 "BATCH",
                                 "Concentrate dosing completed. Next: temperature check");
                             break;

                         case BatchController::State::TemperatureCheck:
                             eventManager.addInfo(
                                 "BATCH",
                                 "Temperature check passed. Next: mixing");
                             break;

                         case BatchController::State::Mixing:
                             eventManager.addInfo(
                                 "BATCH",
                                 "Mixing completed. Ready for transfer");
                             break;

                         case BatchController::State::Transferring:
                             eventManager.addInfo(
                                 "BATCH",
                                 "Transfer completed. Batch complete");
                             break;

                         default:
                             break;
                         }
                     }
    );

    QObject::connect(&batchController,&BatchController::stateChanged,&eventManager,
                    [&mixingTank, &batchController, &operatorSession, &eventManager,previousState = batchController.state()]() mutable
                    {
                        using State = BatchController::State;

                        const State currentState = batchController.state();

                        if (currentState == previousState)
                            return;

                        const State oldState = previousState;
                        previousState = currentState;

                        if (currentState == State::TemperatureCheck &&
                            mixingTank.temperature() <= 25.0)
                        {
                            eventManager.addInfo(
                                "TK1", "TEMP too low. TK1 in heating mode (simulated)");
                        }

                        if (oldState == State::Idle &&
                            currentState == State::FillingWater)
                        {
                            eventManager.addInfo(
                                "BATCH",
                                "Batch started by " + operatorSession.operatorName());
                        }
                        else if (currentState == State::Draining)
                        {
                            const bool resumedDrain =
                                oldState == State::Paused &&
                                batchController.stopReason() ==
                                    BatchController::StopReason::DrainValveUnavailable;

                            eventManager.addWarning(
                                "BATCH",
                                QString(resumedDrain ? "Drain resumed by "
                                                     : "Drain started by ")
                                    + operatorSession.operatorName());
                        }
                        else if (oldState == State::Draining &&
                                 currentState == State::Idle)
                        {
                            eventManager.addInfo("BATCH", "Drain completed");
                        }

                        else if (currentState == State::Paused)
                        {
                            if (batchController.stopReason() ==
                                BatchController::StopReason::OperatorPause)
                            {
                                eventManager.addInfo(
                                    "BATCH",
                                    "Batch paused by " + operatorSession.operatorName());
                            }
                            else
                            {
                                eventManager.addWarning(
                                    "BATCH",
                                    "Batch paused: " + batchController.stopReasonText());
                            }
                        }

                        else if (oldState == State::Paused &&
                                 currentState != State::Idle)
                        {
                            eventManager.addInfo(
                                "BATCH",
                                "Batch resumed by " + operatorSession.operatorName());
                        }
                    }
    );

    QObject::connect(&batchController, &BatchController::batchIdChanged, &eventManager,
        [&batchController, &eventManager]() {eventManager.setBatchId(batchController.batchId());} );
// QML context properties

    engine.rootContext()->setContextProperty("pump", &pump1);
    engine.rootContext()->setContextProperty("valve1", &valve1);

    engine.rootContext()->setContextProperty("pump2", &pump2);
    engine.rootContext()->setContextProperty("valve2", &valve2);

    engine.rootContext()->setContextProperty("pump3", &pump3);
    engine.rootContext()->setContextProperty("valve3", &valve3);

    engine.rootContext()->setContextProperty("valve4", &valve4);

    engine.rootContext()->setContextProperty("mixingTank", &mixingTank);
    engine.rootContext()->setContextProperty("mixer", &mixer);


    engine.rootContext()->setContextProperty("batchController", &batchController);

    engine.rootContext()->setContextProperty("operatorSession", &operatorSession);
    engine.rootContext()->setContextProperty("eventManager",&eventManager);
    engine.rootContext()->setContextProperty("trendRecorder", &trendRecorder);
// QML loading

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []()
        {
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.loadFromModule("PumpStationHMI", "HMIScreen");

    return QGuiApplication::exec();
}