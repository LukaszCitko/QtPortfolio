#include "controllerprotocol.h"

#include <QCoreApplication>
#include <QDebug>
#include <QModbusDataUnit>
#include <QModbusReply>
#include <QModbusTcpClient>
#include <QTimer>
#include <QVariant>
#include <QList>

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    QModbusTcpClient client;
    QTimer pollingTimer;

    pollingTimer.setInterval(1000);

    client.setConnectionParameter(
        QModbusDevice::NetworkAddressParameter,
        QStringLiteral("127.0.0.1"));

    client.setConnectionParameter(
        QModbusDevice::NetworkPortParameter,
        ControllerProtocol::TcpPort);

    QObject::connect(
        &pollingTimer,
        &QTimer::timeout,
        &application,
        [&client, &application]() {
            const QModbusDataUnit request(
                QModbusDataUnit::InputRegisters,
                ControllerProtocol::address(
                    ControllerProtocol::InputRegister::ProtocolVersion),
                2);

            QModbusReply *reply = client.sendReadRequest(
                request,
                ControllerProtocol::ServerAddress);

            if (!reply) {
                qWarning() << "Failed to send Modbus read request:"
                           << client.errorString();
                return;
            }

            QObject::connect(
                reply,
                &QModbusReply::finished,
                &application,
                [reply]() {
                    if (reply->error() != QModbusDevice::NoError) {
                        qWarning() << "Modbus reply error:"
                                   << reply->errorString();
                        reply->deleteLater();
                        return;
                    }

                    const QModbusDataUnit result = reply->result();

                    const quint16 protocolVersion = result.value(0);
                    const quint16 heartbeat = result.value(1);

                    qInfo() << "Protocol version:" << protocolVersion
                            << "| Heartbeat:" << heartbeat;

                    reply->deleteLater();
                });
        });

    QObject::connect(
        &client,
        &QModbusDevice::stateChanged,
        &application,
        [&client, &pollingTimer, &application](
            QModbusDevice::State state) {
            if (state == QModbusDevice::ConnectedState) {
                qInfo() << "Connected to controller simulator.";

                const QList<quint16> commandValues {
                    static_cast<quint16>(
                        ControllerProtocol::Command::StartBatch),
                    1
                };

                const QModbusDataUnit commandRequest(
                    QModbusDataUnit::HoldingRegisters,
                    ControllerProtocol::address(
                        ControllerProtocol::HoldingRegister::CommandCode),
                    commandValues);

                QModbusReply *reply = client.sendWriteRequest(
                    commandRequest,
                    ControllerProtocol::ServerAddress);

                if (!reply) {
                    qWarning() << "Failed to send test command:"
                               << client.errorString();
                } else {
                    QObject::connect(
                        reply,
                        &QModbusReply::finished,
                        &application,
                        [reply]() {
                            if (reply->error() == QModbusDevice::NoError) {
                                qInfo() << "Test command write confirmed.";
                            } else {
                                qWarning() << "Test command write failed:"
                                           << reply->errorString();
                            }

                            reply->deleteLater();
                        });
                }

                pollingTimer.start();
                return;
            }


            if (state == QModbusDevice::UnconnectedState) {
                pollingTimer.stop();
                qInfo() << "Disconnected from controller simulator.";
            }
        });

    if (!client.connectDevice()) {
        qCritical() << "Failed to connect to controller simulator:"
                    << client.errorString();
        return 1;
    }

    qInfo() << "Connecting to controller simulator...";

    return application.exec();
}
