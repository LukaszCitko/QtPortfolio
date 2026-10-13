#include "controllerprotocol.h"
#include "modbuscontrollerclient.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    const QStringList arguments = application.arguments();

    const QString controllerHost = arguments.size() > 1
            ? arguments.at(1) : QStringLiteral("127.0.0.1");

    ModbusControllerClient controllerClient;

    bool rpmWriteRequested = false;
    bool transferRequested = false;
    bool nextBatchRequested = false;
    bool faultInjectionRequested = false;
    bool faultSourceActive = false;
    bool faultClearScheduled = false;
    bool faultSourceCleared = false;
    bool faultResetRequested = false;
    bool resumeRequested = false;


    QObject::connect(&controllerClient, &ModbusControllerClient::connectionChanged, &application, [&controllerClient]()
        {
            if (controllerClient.isConnected())
            {
                qInfo() << "Connected to controller.";
                return;
            }

            qInfo() << "Disconnected from controller.";
        });

    QObject::connect(&controllerClient, &ModbusControllerClient::lastErrorChanged, &application, [&controllerClient]()
        {
            if (controllerClient.lastError().isEmpty())
            {
                return;
            }

            qWarning() << "Controller communication error:"  << controllerClient.lastError();
        });
    QObject::connect(&controllerClient, &ModbusControllerClient::commandPendingChanged, &application, [&controllerClient]()
        {
            if (controllerClient.commandPending())
            {
                qInfo() << "Waiting for controller command acknowledgement.";
                return;
            }

            qInfo() << "Controller handled sequence:" << controllerClient.lastHandledCommandSequence()
                    << "| Result:" << controllerClient.lastCommandResult();
        });
    QObject::connect(&controllerClient, &ModbusControllerClient::pumpTargetRpmWriteFinished, &application, [&controllerClient](int pumpNumber, int requestedRpm, bool success)
        {
             qInfo() << "Pump target RPM write finished. Pump:" << pumpNumber << "| RPM:" << requestedRpm << "| Success:" << success;

             if (!success || pumpNumber != 1)
             {
                 return;
             }

             const bool startCommandSent = controllerClient.startBatch();

             if (startCommandSent)
             {
                 qInfo() << "StartBatch command sent after RPM confirmation.";
             }
             else
             {
                 qWarning() << "StartBatch command could not be sent:" << controllerClient.lastError();
             }
        });
    QObject::connect(&controllerClient, &ModbusControllerClient::simulatedFaultMaskWriteFinished, &application, [&faultSourceActive, &faultSourceCleared](quint16 faultMask, bool success)
        {
             qInfo() << "Simulated fault mask write finished. Mask:" << faultMask << "| Success:" << success;

             if (!success)
             {
                 return;
             }

             const quint16 pump2FaultMask = static_cast<quint16>(ControllerProtocol::FaultBit::Pump2);

             if (faultMask == pump2FaultMask)
             {
                 faultSourceActive = true;
             }

             if (faultMask == 0)
             {
                 faultSourceCleared = true;
             }
        });
    QObject::connect(&controllerClient, &ModbusControllerClient::snapshotChanged, &application, [&application, &controllerClient, &rpmWriteRequested, &transferRequested, &nextBatchRequested, &faultInjectionRequested, &faultSourceActive, &faultClearScheduled, &faultSourceCleared, &faultResetRequested, &resumeRequested]()
        {
            if (!controllerClient.snapshotValid())
            {
                qWarning() << "Controller snapshot is not valid.";
                return;
            }
            qInfo()
                << "Protocol:" << controllerClient.protocolVersion()
                << "| Heartbeat:" << controllerClient.heartbeat()
                << "| State:"  << controllerClient.processState()
                << "| Progress:" << controllerClient.stageProgress()
                << "| Volume:"  << controllerClient.mixtureVolumeLitres() << "L"
                << "| Water:" << controllerClient.waterVolumeLitres() << "L"
                << "| Concentrate:" << controllerClient.concentrateVolumeLitres() << "L"
                << "| Temperature:" << controllerClient.temperatureCelsius() << "C"
                << "| P1:" << controllerClient.pump1ActualRpm() << "RPM"
                << "| P2:" << controllerClient.pump2ActualRpm() << "RPM"
                << "| P3:" << controllerClient.pump3ActualRpm() << "RPM"
                << "| M1:" << controllerClient.mixerActualRpm() << "RPM"
                << "| Equipment mask:" << controllerClient.equipmentStatusMask()
                << "| Fault mask:"  << controllerClient.faultMask()
                << "| Command sequence:" << controllerClient.lastHandledCommandSequence()
                << "| Command result:" << controllerClient.lastCommandResult();

            if (!rpmWriteRequested)
            {
                rpmWriteRequested = true;

                if (controllerClient.setPumpTargetRpm(1, 725))
                {
                    qInfo() << "P1 target RPM write requested.";
                }
                else
                {
                    qWarning() << "P1 target RPM could not be written:" << controllerClient.lastError();
                }
            }
            const int processState = controllerClient.processState();
            const quint16 pump2FaultMask = static_cast<quint16>(ControllerProtocol::FaultBit::Pump2);

            if (processState == static_cast<int>(ControllerProtocol::ProcessState::DosingConcentrate) && controllerClient.mixtureVolumeLitres() >= 75.0 && !faultInjectionRequested)
            {
                if (controllerClient.setSimulatedFaultMask(pump2FaultMask))
                {
                    faultInjectionRequested = true;
                    qInfo() << "P2 fault injection requested.";
                }
                else
                {
                    qWarning() << "P2 fault could not be injected:" << controllerClient.lastError();
                }
            }

            if (processState == static_cast<int>(ControllerProtocol::ProcessState::Paused) && faultSourceActive && (controllerClient.faultMask() & pump2FaultMask) != 0U && !faultClearScheduled)
            {
                faultClearScheduled = true;
                qInfo() << "P2 fault detected. Keeping the fault active for two seconds.";

                QTimer::singleShot(2000, &application, [&controllerClient]()
                                   {
                                       if (controllerClient.setSimulatedFaultMask(0))
                                       {
                                           qInfo() << "P2 fault source clear requested.";
                                       }
                                       else
                                       {
                                           qWarning() << "P2 fault source could not be cleared:" << controllerClient.lastError();
                                       }
                                   });
            }

            if (processState == static_cast<int>(ControllerProtocol::ProcessState::Paused) && faultSourceCleared && (controllerClient.faultMask() & pump2FaultMask) != 0U && !faultResetRequested && !controllerClient.commandPending())
            {
                if (controllerClient.resetFaults())
                {
                    faultResetRequested = true;
                    qInfo() << "ResetFaults command sent.";
                }
                else
                {
                    qWarning() << "ResetFaults command could not be sent:" << controllerClient.lastError();
                }
            }

            if (processState == static_cast<int>(ControllerProtocol::ProcessState::Paused) && faultResetRequested && controllerClient.faultMask() == 0U && !resumeRequested && !controllerClient.commandPending())
            {
                if (controllerClient.resume())
                {
                    resumeRequested = true;
                    qInfo() << "ResumeBatch command sent after fault reset.";
                }
                else
                {
                    qWarning() << "ResumeBatch command could not be sent:" << controllerClient.lastError();
                }
            }

            if (controllerClient.processState() == static_cast<int>(ControllerProtocol::ProcessState::ReadyForTransfer) && !transferRequested && !controllerClient.commandPending())
            {
                if (controllerClient.startTransfer())
                {
                    transferRequested = true;
                    qInfo() << "StartTransfer command sent.";
                }
                else
                {
                    qWarning() << "StartTransfer command could not be sent:" << controllerClient.lastError();
                }
            }

            if (controllerClient.processState() == static_cast<int>(ControllerProtocol::ProcessState::Complete) && !nextBatchRequested && !controllerClient.commandPending())
            {
                if (controllerClient.prepareNextBatch())
                {
                    nextBatchRequested = true;
                    qInfo() << "PrepareNextBatch command sent.";
                }
                else
                {
                    qWarning() << "PrepareNextBatch command could not be sent:" << controllerClient.lastError();
                }
            }

            if (controllerClient.processState() == static_cast<int>(ControllerProtocol::ProcessState::Idle) && nextBatchRequested && !controllerClient.commandPending())
            {
                qInfo() << "Complete Modbus batch cycle finished successfully.";
                application.quit();
            }

        });

    if (!controllerClient.connectToController(controllerHost))
    {
        qCritical() << "Could not start controller connection:" << controllerClient.lastError();
        return 1;
    }

    qInfo()
        << "Connecting to controller at" << controllerHost
        << "on port" << ControllerProtocol::TcpPort << "...";

    return application.exec();
}