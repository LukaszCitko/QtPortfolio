#include "controllerprotocol.h"

#include <QCoreApplication>
#include <QDebug>
#include <QModbusDataUnit>
#include <QModbusTcpServer>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    const bool acknowledgeCommands = !application.arguments().contains(QStringLiteral("--no-ack"));
    const bool delayAcknowledgement = application.arguments().contains(QStringLiteral("--late-ack"));
    const bool freezeHeartbeat = application.arguments().contains(QStringLiteral("--freeze-heartbeat"));

    QModbusTcpServer server;
    QModbusDataUnitMap registerMap;

    registerMap.insert(QModbusDataUnit::InputRegisters, QModbusDataUnit(QModbusDataUnit::InputRegisters, 0,ControllerProtocol::RegisterCount));

    registerMap.insert(QModbusDataUnit::HoldingRegisters, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, ControllerProtocol::RegisterCount));

    if (!server.setMap(registerMap)) {
        qCritical() << "Failed to create the Modbus register map.";
        return 1;
    }

    QObject::connect(&server, &QModbusServer::dataWritten, &application, [&server, &application, acknowledgeCommands, delayAcknowledgement](QModbusDataUnit::RegisterType table, int address, int size)
                    {
                        if (table != QModbusDataUnit::HoldingRegisters) return;

                        const int commandAddress = ControllerProtocol::address(ControllerProtocol::HoldingRegister::CommandCode);

                        if (address != commandAddress || size < 2) return;


                        quint16 commandCode = 0;
                        quint16 commandSequence = 0;

                        const bool commandRead = server.data(QModbusDataUnit::HoldingRegisters,
                                                             ControllerProtocol::address(ControllerProtocol::HoldingRegister::CommandCode),
                                                             &commandCode);

                        const bool sequenceRead = server.data(QModbusDataUnit::HoldingRegisters,
                                                              ControllerProtocol::address(ControllerProtocol::HoldingRegister::CommandSequence),
                                                              &commandSequence);

                        if (!commandRead || !sequenceRead) {
                            qWarning() << "Failed to read command registers.";
                            return;
                        }

                        qInfo() << "Received command:" << commandCode
                                << "| Sequence:" << commandSequence;

                        if (!acknowledgeCommands) {
                            qInfo() << "Leaving command unacknowledged for timeout test.";
                            return;
                        }

                        const auto acknowledge = [&server, commandSequence]() {
                            const bool resultWritten = server.setData(QModbusDataUnit::InputRegisters, ControllerProtocol::address(ControllerProtocol::InputRegister::LastCommandResult), static_cast<quint16>(ControllerProtocol::CommandResult::Accepted));

                            if (!resultWritten) {
                                qWarning() << "Failed to write command result.";
                                return;
                            }

                            const bool acknowledged = server.setData(QModbusDataUnit::InputRegisters, ControllerProtocol::address(ControllerProtocol::InputRegister::LastHandledCommandSequence), commandSequence);

                            if (!acknowledged) {
                                qWarning() << "Failed to acknowledge command sequence.";
                            }
                        };

                        if (delayAcknowledgement) {
                            QTimer::singleShot(12000, &application, acknowledge);
                        } else {
                            acknowledge();
                        }

    });


    server.setServerAddress(ControllerProtocol::ServerAddress);

    server.setConnectionParameter(QModbusDevice::NetworkAddressParameter, QStringLiteral("127.0.0.1"));

    server.setConnectionParameter(QModbusDevice::NetworkPortParameter, ControllerProtocol::TcpPort);

    server.setData(QModbusDataUnit::InputRegisters, ControllerProtocol::address(ControllerProtocol::InputRegister::ProtocolVersion), ControllerProtocol::ProtocolVersion);

    if (!server.connectDevice()) {
        qCritical() << "Failed to start Modbus TCP server:"
                    << server.errorString();
        return 1;
    }

    qInfo() << "Controller simulator listening on"
            << "127.0.0.1:" << ControllerProtocol::TcpPort;

    std::uint16_t heartbeat = 0;

    QTimer heartbeatTimer;

    QObject::connect(&heartbeatTimer, &QTimer::timeout, &application, [&server, &heartbeat]()
                    {
                        ++heartbeat;

                        const bool written = server.setData(QModbusDataUnit::InputRegisters,
                                                            ControllerProtocol::address(ControllerProtocol::InputRegister::ControllerHeartbeat),
                                                            heartbeat);

                        if (!written) qWarning() << "Failed to update controller heartbeat.";

    });

    if (!freezeHeartbeat) {
        heartbeatTimer.start(1000);
    } else if (application.arguments().contains(QStringLiteral("--resume-heartbeat"))) {
        QTimer::singleShot(8000, &application, [&heartbeatTimer]() { heartbeatTimer.start(1000); });
    }


    return application.exec();
}
