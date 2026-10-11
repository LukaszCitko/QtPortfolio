#include "controllerprotocol.h"
#include "modbuscontrollerclient.h"

#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    const QStringList arguments = application.arguments();

    const QString controllerHost = arguments.size() > 1
            ? arguments.at(1) : QStringLiteral("127.0.0.1");

    ModbusControllerClient controllerClient;

    bool commandSent = false;

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

    QObject::connect(&controllerClient, &ModbusControllerClient::snapshotChanged, &application, [&controllerClient, &commandSent]()
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
                << "| Temperature:" << controllerClient.temperatureCelsius() << "C"
                << "| P1:" << controllerClient.pump1ActualRpm() << "RPM"
                << "| P2:" << controllerClient.pump2ActualRpm() << "RPM"
                << "| P3:" << controllerClient.pump3ActualRpm() << "RPM"
                << "| M1:" << controllerClient.mixerActualRpm() << "RPM"
                << "| Equipment mask:" << controllerClient.equipmentStatusMask()
                << "| Fault mask:"  << controllerClient.faultMask()
                << "| Command sequence:" << controllerClient.lastHandledCommandSequence()
                << "| Command result:" << controllerClient.lastCommandResult();

            if (!commandSent)
            {
                commandSent = controllerClient.startBatch();

                if (commandSent)
                {
                    qInfo() << "StartBatch command sent.";
                }
                else
                {
                    qWarning() << "StartBatch command could not be sent:" << controllerClient.lastError();
                }
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