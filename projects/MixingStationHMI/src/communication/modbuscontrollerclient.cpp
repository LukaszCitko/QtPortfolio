#include "modbuscontrollerclient.h"
#include "controllerprotocol.h"

#include <QModbusReply>
#include <QVariant>
#include <QList>

ModbusControllerClient::ModbusControllerClient(QObject *parent) : QObject(parent)
{
    m_client.setTimeout(1000);
    m_client.setNumberOfRetries(2);

    m_pollTimer.setInterval(500);
    m_reconnectTimer.setInterval(2000);

    connect(&m_pollTimer, &QTimer::timeout, this, &ModbusControllerClient::requestSnapshot);

    connect(&m_client, &QModbusDevice::stateChanged, this, [this](QModbusDevice::State state)
        {
            emit connectionChanged();

            if (state == QModbusDevice::ConnectedState)
            {
                m_reconnectTimer.stop();
                setLastError({});
                requestSnapshot();
                m_pollTimer.start();
                return;
            }

            if (state == QModbusDevice::UnconnectedState)
            {
                m_pollTimer.stop();
                m_readPending = false;

                if (m_commandPending)
                {
                    m_commandPending = false;
                    emit commandPendingChanged();
                }

                if (!m_disconnectRequested && !m_controllerHost.isEmpty() && !m_reconnectTimer.isActive())
                {
                    m_reconnectTimer.start();
                }

                if (m_snapshotValid)
                {
                    m_snapshotValid = false;
                    emit snapshotChanged();
                }
            }
        });

    connect(&m_client, &QModbusDevice::errorOccurred, this, [this](QModbusDevice::Error error)
        {
            if (error == QModbusDevice::NoError)
            {
                return;
            }

            setLastError(m_client.errorString());
        });
    connect(&m_reconnectTimer, &QTimer::timeout, this, [this]()
        {
            if (m_disconnectRequested || m_controllerHost.isEmpty() || m_client.state() != QModbusDevice::UnconnectedState)
            {
                return;
            }

            if (!m_client.connectDevice())
            {
                setLastError(m_client.errorString());
            }
        });
}

bool ModbusControllerClient::isConnected() const
{
    return m_client.state() == QModbusDevice::ConnectedState;
}

QString ModbusControllerClient::lastError() const
{
    return m_lastError;
}

bool ModbusControllerClient::snapshotValid() const
{
    return m_snapshotValid;
}

quint16 ModbusControllerClient::protocolVersion() const
{
    return m_protocolVersion;
}

quint16 ModbusControllerClient::heartbeat() const
{
    return m_heartbeat;
}

quint16 ModbusControllerClient::controllerSessionId() const
{
    return m_controllerSessionId;
}

quint16 ModbusControllerClient::runSequence() const
{
    return m_runSequence;
}

int ModbusControllerClient::runKind() const
{
    return m_runKind;
}

int ModbusControllerClient::processState() const
{
    return m_processState;
}

double ModbusControllerClient::stageProgress() const
{
    return m_stageProgress;
}

double ModbusControllerClient::mixtureVolumeLitres() const
{
    return m_mixtureVolumeLitres;
}


double ModbusControllerClient::waterVolumeLitres() const
{
    return m_waterVolumeLitres;
}

double ModbusControllerClient::concentrateVolumeLitres() const
{
    return m_concentrateVolumeLitres;
}

double ModbusControllerClient::temperatureCelsius() const
{
    return m_temperatureCelsius;
}

int ModbusControllerClient::pump1ActualRpm() const
{
    return m_pump1ActualRpm;
}

int ModbusControllerClient::pump2ActualRpm() const
{
    return m_pump2ActualRpm;
}

int ModbusControllerClient::pump3ActualRpm() const
{
    return m_pump3ActualRpm;
}

int ModbusControllerClient::mixerActualRpm() const
{
    return m_mixerActualRpm;
}

quint16 ModbusControllerClient::equipmentStatusMask() const
{
    return m_equipmentStatusMask;
}

quint16 ModbusControllerClient::faultMask() const
{
    return m_faultMask;
}

quint16 ModbusControllerClient::simulatedFaultMask() const
{
    return m_simulatedFaultMask;
}

quint16 ModbusControllerClient::lastHandledCommandSequence() const
{
    return m_lastHandledCommandSequence;
}

int ModbusControllerClient::lastCommandResult() const
{
    return m_lastCommandResult;
}

bool ModbusControllerClient::commandPending() const
{
    return m_commandPending;
}

bool ModbusControllerClient::connectToController(
    const QString &host)
{
    m_controllerHost = host.trimmed();
    m_disconnectRequested = false;

    if (host.trimmed().isEmpty())
    {
        setLastError(QStringLiteral("Controller address cannot be empty."));
        return false;
    }

    if (m_client.state()
        != QModbusDevice::UnconnectedState)
    {
        setLastError(QStringLiteral("Controller connection is already active."));
        return false;
    }

    m_client.setConnectionParameter(QModbusDevice::NetworkAddressParameter, m_controllerHost);

    m_client.setConnectionParameter(QModbusDevice::NetworkPortParameter, ControllerProtocol::TcpPort);

    setLastError({});

    if (!m_client.connectDevice())
    {
        setLastError(m_client.errorString());
        m_reconnectTimer.start();
        return false;
    }

    return true;
}

void ModbusControllerClient::disconnectFromController()
{
    m_disconnectRequested = true;
    m_reconnectTimer.stop();
    m_pollTimer.stop();
    m_client.disconnectDevice();
}

bool ModbusControllerClient::setPumpTargetRpm(int pumpNumber, int rpm)
{
    switch (pumpNumber)
    {
    case 1:
        return writePumpTargetRpm(pumpNumber, ControllerProtocol::address(ControllerProtocol::HoldingRegister::Pump1TargetRpm), rpm);

    case 2:
        return writePumpTargetRpm(pumpNumber, ControllerProtocol::address(ControllerProtocol::HoldingRegister::Pump2TargetRpm), rpm);

    case 3:
        return writePumpTargetRpm(pumpNumber, ControllerProtocol::address(ControllerProtocol::HoldingRegister::Pump3TargetRpm), rpm);

    default:
        setLastError(QStringLiteral("Pump number must be between 1 and 3."));
        return false;
    }
}
bool ModbusControllerClient::setSimulatedFault(quint16 faultBit, bool active)
{
    if (faultBit == 0U || (faultBit & static_cast<quint16>(~ControllerProtocol::DefinedFaultMask)) != 0U)
    {
        setLastError(QStringLiteral("Requested simulated fault bit is not supported."));
        return false;
    }

    const quint16 newFaultMask = active ? static_cast<quint16>(m_simulatedFaultMask | faultBit) : static_cast<quint16>(m_simulatedFaultMask & static_cast<quint16>(~faultBit));
    return setSimulatedFaultMask(newFaultMask);
}

bool ModbusControllerClient::setSimulatedFaultMask(quint16 faultMask)
{
    if (!isConnected())
    {
        setLastError(QStringLiteral("Cannot write simulated faults while disconnected."));
        return false;
    }

    const quint16 undefinedFaultBits = faultMask & static_cast<quint16>(~ControllerProtocol::DefinedFaultMask);

    if (undefinedFaultBits != 0U)
    {
        setLastError(QStringLiteral("Simulated fault mask contains unsupported bits."));
        return false;
    }

    const QList<quint16> values{faultMask};
    const QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, ControllerProtocol::address(ControllerProtocol::HoldingRegister::SimulatedFaultMask), values);
    QModbusReply *reply = m_client.sendWriteRequest(request, ControllerProtocol::ServerAddress);

    if (reply == nullptr)
    {
        setLastError(m_client.errorString());
        return false;
    }

    const auto handleReply = [this, reply, faultMask]()
    {
        const bool success = reply->error() == QModbusDevice::NoError;

        if (success)
        {
            setLastError({});

            if (m_simulatedFaultMask != faultMask)
            {
                m_simulatedFaultMask = faultMask;
                emit simulatedFaultMaskChanged();
            }
        }
        else
        {
            setLastError(reply->errorString());
        }

        emit simulatedFaultMaskWriteFinished(faultMask, success);
        reply->deleteLater();
    };

    if (reply->isFinished())
    {
        handleReply();
        return true;
    }

    connect(reply, &QModbusReply::finished, this, handleReply);
    return true;
}

bool ModbusControllerClient::writePumpTargetRpm(int pumpNumber, quint16 registerAddress, int rpm)
{
    if (!isConnected())
    {
        setLastError(QStringLiteral("Cannot write pump RPM while disconnected."));
        return false;
    }

    if (rpm < 0 || rpm > static_cast<int>(ControllerProtocol::MaximumPumpRpm))
    {
        setLastError(QStringLiteral("Pump RPM must be between 0 and %1.").arg(ControllerProtocol::MaximumPumpRpm));
        return false;
    }

    const QList<quint16> values{static_cast<quint16>(rpm)};
    const QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, registerAddress, values);
    QModbusReply *reply = m_client.sendWriteRequest(request, ControllerProtocol::ServerAddress);

    if (reply == nullptr)
    {
        setLastError(m_client.errorString());
        return false;
    }

    const auto handleReply = [this, reply, pumpNumber, rpm]()
    {
        const bool success = reply->error() == QModbusDevice::NoError;

        if (success)
        {
            setLastError({});
        }
        else
        {
            setLastError(reply->errorString());
        }

        emit pumpTargetRpmWriteFinished(pumpNumber, rpm, success);
        reply->deleteLater();
    };

    if (reply->isFinished())
    {
        handleReply();
        return true;
    }

    connect(reply, &QModbusReply::finished, this, handleReply);
    return true;
}

bool ModbusControllerClient::temperatureSensorValid() const
{
    const quint16 validMask = static_cast<quint16>(ControllerProtocol::EquipmentStatusBit::TemperatureSensorValid);
    return m_snapshotValid && (m_equipmentStatusMask & validMask) != 0U;
}

bool ModbusControllerClient::startBatch()
{
    return sendCommand(static_cast<quint16>(ControllerProtocol::Command::StartBatch));
}

bool ModbusControllerClient::pause()
{
    return sendCommand(static_cast<quint16>(ControllerProtocol::Command::PauseBatch));
}

bool ModbusControllerClient::resume()
{
    return sendCommand(static_cast<quint16>(ControllerProtocol::Command::ResumeBatch));
}

bool ModbusControllerClient::startTransfer()
{
    return sendCommand(static_cast<quint16>(ControllerProtocol::Command::StartTransfer));
}

bool ModbusControllerClient::resetFaults()
{
    return sendCommand(static_cast<quint16>(ControllerProtocol::Command::ResetFaults));
}

bool ModbusControllerClient::startDrain()
{
    return sendCommand(static_cast<quint16>(ControllerProtocol::Command::StartDrain));
}

bool ModbusControllerClient::prepareNextBatch()
{
    return sendCommand(static_cast<quint16>(ControllerProtocol::Command::PrepareNextBatch));
}

bool ModbusControllerClient::sendCommand(quint16 commandCode)
{
    if (!isConnected())
    {
        setLastError(QStringLiteral("Cannot send a command while disconnected."));
        return false;
    }

    if (m_commandPending)
    {
        setLastError(QStringLiteral("The previous command is still pending."));
        return false;
    }

    const quint16 commandSequence = m_nextCommandSequence;

    const QList<quint16> values{commandCode, commandSequence};

    const QModbusDataUnit request(
        QModbusDataUnit::HoldingRegisters,
        ControllerProtocol::address(ControllerProtocol::HoldingRegister::CommandCode),
        values);

    QModbusReply *reply = m_client.sendWriteRequest(request, ControllerProtocol::ServerAddress);

    if (!reply)
    {
        setLastError(m_client.errorString());
        return false;
    }

    m_pendingCommandSequence = commandSequence;
    m_commandPending = true;

    ++m_nextCommandSequence;

    if (m_nextCommandSequence == 0)
    {
        m_nextCommandSequence = 1;
    }

    setLastError({});
    emit commandPendingChanged();

    const auto handleReply = [this, reply]()
    {
        if (reply->error()
            != QModbusDevice::NoError)
        {
            m_commandPending = false;

            setLastError(reply->errorString());
            emit commandPendingChanged();
        }

        reply->deleteLater();
    };

    if (reply->isFinished())
    {
        handleReply();
        return true;
    }

    connect(reply, &QModbusReply::finished, this, handleReply);

    return true;
}

void ModbusControllerClient::requestSnapshot()
{
    if (!isConnected() || m_readPending)
    {
        return;
    }

    const QModbusDataUnit request(
        QModbusDataUnit::InputRegisters,
        ControllerProtocol::address(ControllerProtocol::InputRegister::ProtocolVersion), ControllerProtocol::RegisterCount);

    QModbusReply *reply = m_client.sendReadRequest(request, ControllerProtocol::ServerAddress);

    if (!reply)
    {
        setLastError(m_client.errorString());
        return;
    }

    m_readPending = true;

    const auto handleReply = [this, reply]()
    {
        m_readPending = false;

        if (reply->error() != QModbusDevice::NoError)
        {
            setLastError(reply->errorString());

            if (m_snapshotValid)
            {
                m_snapshotValid = false;
                emit snapshotChanged();
            }

            reply->deleteLater();
            return;
        }

        processSnapshot(reply->result());
        reply->deleteLater();
    };

    if (reply->isFinished())
    {
        handleReply();
        return;
    }

    connect(reply, &QModbusReply::finished, this, handleReply);
}

void ModbusControllerClient::processSnapshot(const QModbusDataUnit &registers)
{
    if (registers.valueCount() < ControllerProtocol::RegisterCount)
    {
        setLastError(QStringLiteral("Controller returned an incomplete register bank."));
        return;
    }

    const auto registerValue = [&registers](ControllerProtocol::InputRegister registerId)
    {
        return registers.value(ControllerProtocol::address(registerId));
    };

    const quint16 receivedProtocolVersion = registerValue(ControllerProtocol::InputRegister::ProtocolVersion);

    if (receivedProtocolVersion != ControllerProtocol::ProtocolVersion)
    {
        m_snapshotValid = false;

        setLastError(QStringLiteral("Controller protocol version does not match the HMI."));

        emit snapshotChanged();
        return;
    }

    m_protocolVersion = receivedProtocolVersion;

    m_heartbeat = registerValue(ControllerProtocol::InputRegister::ControllerHeartbeat);
    m_controllerSessionId = registerValue(ControllerProtocol::InputRegister::ControllerSessionId);
    m_runSequence = registerValue(ControllerProtocol::InputRegister::RunSequence);
    m_runKind = static_cast<int>(registerValue(ControllerProtocol::InputRegister::RunKind));
    m_processState = static_cast<int>(registerValue(ControllerProtocol::InputRegister::ProcessState));

    const quint16 progressPermille = registerValue(ControllerProtocol::InputRegister::StageProgressPermille);
    m_stageProgress = static_cast<double>(progressPermille) / 1000.0;

    const quint16 volumeDecilitres = registerValue(ControllerProtocol::InputRegister::MixtureVolumeDecilitres);
    m_mixtureVolumeLitres = static_cast<double>(volumeDecilitres) / 10.0;

    const quint16 waterVolumeDecilitres = registerValue(ControllerProtocol::InputRegister::WaterVolumeDecilitres);
    m_waterVolumeLitres = static_cast<double>(waterVolumeDecilitres) / 10.0;

    const quint16 concentrateVolumeDecilitres = registerValue(ControllerProtocol::InputRegister::ConcentrateVolumeDecilitres);
    m_concentrateVolumeLitres = static_cast<double>(concentrateVolumeDecilitres) / 10.0;

    const quint16 temperatureTenths = registerValue(ControllerProtocol::InputRegister::TemperatureTenthsCelsius);
    m_temperatureCelsius = static_cast<double>(temperatureTenths) / 10.0;

    m_pump1ActualRpm = static_cast<int>(registerValue(ControllerProtocol::InputRegister::Pump1ActualRpm));
    m_pump2ActualRpm = static_cast<int>(registerValue(ControllerProtocol::InputRegister::Pump2ActualRpm));
    m_pump3ActualRpm = static_cast<int>(registerValue(ControllerProtocol::InputRegister::Pump3ActualRpm));
    m_mixerActualRpm = static_cast<int>(registerValue(ControllerProtocol::InputRegister::MixerActualRpm));
    m_equipmentStatusMask = registerValue(ControllerProtocol::InputRegister::EquipmentStatusMask);
    m_faultMask =registerValue(ControllerProtocol::InputRegister::FaultMask);
    m_lastHandledCommandSequence =registerValue(ControllerProtocol::InputRegister::LastHandledCommandSequence);
    m_lastCommandResult =static_cast<int>(registerValue(ControllerProtocol::InputRegister::LastCommandResult));

    if (m_commandPending && m_lastHandledCommandSequence == m_pendingCommandSequence)
    {
        m_commandPending = false;
        emit commandPendingChanged();
    }

    if (!m_commandPending)
    {
        m_nextCommandSequence = static_cast<quint16>(m_lastHandledCommandSequence + 1);

        if (m_nextCommandSequence == 0)
        {
            m_nextCommandSequence = 1;
        }
    }

    m_snapshotValid = true;

    setLastError({});
    emit snapshotChanged();
}

void ModbusControllerClient::setLastError(const QString &message)
{
    if (m_lastError == message)
    {
        return;
    }

    m_lastError = message;
    emit lastErrorChanged();
}