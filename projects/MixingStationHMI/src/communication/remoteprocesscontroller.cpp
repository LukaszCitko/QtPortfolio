#include "remoteprocesscontroller.h"

#include "controllerprotocol.h"
#include "modbuscontrollerclient.h"
#include "processconfiguration.h"

#include <cmath>

RemoteProcessController::RemoteProcessController(ModbusControllerClient *client, QObject *parent)
    : QObject(parent),
      m_client(client),
      m_previousState(static_cast<int>(ControllerProtocol::ProcessState::Idle)),
      m_lastActiveStageIndex(0)
{
    Q_ASSERT(m_client);

    connect(m_client, &ModbusControllerClient::snapshotChanged, this, &RemoteProcessController::handleSnapshotChanged);
    connect(m_client, &ModbusControllerClient::connectionChanged, this, &RemoteProcessController::handleConnectionChanged);
}

bool RemoteProcessController::connected() const
{
    return m_client->isConnected() && m_client->snapshotValid();
}

int RemoteProcessController::state() const
{
    if (!m_client->snapshotValid())
    {
        return static_cast<int>(ControllerProtocol::ProcessState::Idle);
    }

    return m_client->processState();
}

QString RemoteProcessController::stateText() const
{
    switch (static_cast<ControllerProtocol::ProcessState>(state()))
    {
    case ControllerProtocol::ProcessState::Idle:
        return QStringLiteral("IDLE");
    case ControllerProtocol::ProcessState::FillingWater:
        return QStringLiteral("FILLING WATER");
    case ControllerProtocol::ProcessState::DosingConcentrate:
        return QStringLiteral("DOSING CONCENTRATE");
    case ControllerProtocol::ProcessState::TemperatureCheck:
        return QStringLiteral("TEMPERATURE CHECK");
    case ControllerProtocol::ProcessState::Mixing:
        return QStringLiteral("MIXING");
    case ControllerProtocol::ProcessState::ReadyForTransfer:
        return QStringLiteral("READY FOR TRANSFER");
    case ControllerProtocol::ProcessState::Transferring:
        return QStringLiteral("TRANSFERRING");
    case ControllerProtocol::ProcessState::Draining:
        return QStringLiteral("DRAINING");
    case ControllerProtocol::ProcessState::Paused:
        return QStringLiteral("PAUSED");
    case ControllerProtocol::ProcessState::Complete:
        return QStringLiteral("COMPLETE");
    }

    return QStringLiteral("UNKNOWN");
}

int RemoteProcessController::stageIndex() const
{
    if (state() == static_cast<int>(ControllerProtocol::ProcessState::Paused))
    {
        return m_lastActiveStageIndex;
    }

    return stageIndexForState(state());
}

int RemoteProcessController::mixingSecondsRemaining() const
{
    const double durationSeconds = static_cast<double>(ProcessConfiguration::MixingDurationMilliseconds) / 1000.0;
    const double remainingSeconds = durationSeconds * (1.0 - m_client->stageProgress());
    return static_cast<int>(std::ceil(remainingSeconds > 0.0 ? remainingSeconds : 0.0));
}

QString RemoteProcessController::batchId() const
{
    return m_batchId;
}

int RemoteProcessController::runKind() const
{
    if (!m_client->snapshotValid())
    {
        return static_cast<int>(ControllerProtocol::RunKind::None);
    }

    return m_client->runKind();
}

bool RemoteProcessController::recoveryRequired() const
{
    return m_recoveryRequired;
}

bool RemoteProcessController::completeMechanicalRecovery()
{
    const int idleState = static_cast<int>(ControllerProtocol::ProcessState::Idle);

    if (!m_recoveryRequired || !connected() || state() != idleState)
    {
        return false;
    }

    m_recoveryRequired = false;
    emit recoveryRequiredChanged();
    emit startConditionsChanged();
    return true;
}

bool RemoteProcessController::transferAllowed() const
{
    if (!connected() || state() != static_cast<int>(ControllerProtocol::ProcessState::ReadyForTransfer))
    {
        return false;
    }

    const double minimumTemperature = static_cast<double>(ProcessConfiguration::MinimumProcessTemperatureTenthsCelsius) / 10.0;
    const double maximumTemperature = static_cast<double>(ProcessConfiguration::MaximumProcessTemperatureTenthsCelsius) / 10.0;
    return m_client->temperatureCelsius() >= minimumTemperature && m_client->temperatureCelsius() <= maximumTemperature;
}

bool RemoteProcessController::tankEmpty() const
{
    return connected() && m_client->mixtureVolumeLitres() <= 0.0;
}

bool RemoteProcessController::devicesHealthy() const
{
    return connected() && m_client->faultMask() == 0U;
}

bool RemoteProcessController::mixerReady() const
{
    const quint16 mixerConnectedMask = static_cast<quint16>(ControllerProtocol::EquipmentStatusBit::MixerConnected);
    return connected() && (m_client->equipmentStatusMask() & mixerConnectedMask) != 0U;
}

bool RemoteProcessController::valvesClosed() const
{
    const quint16 valveMask = static_cast<quint16>(ControllerProtocol::EquipmentStatusBit::Valve1Open)
                              | static_cast<quint16>(ControllerProtocol::EquipmentStatusBit::Valve2Open)
                              | static_cast<quint16>(ControllerProtocol::EquipmentStatusBit::Valve3Open)
                              | static_cast<quint16>(ControllerProtocol::EquipmentStatusBit::DrainValveOpen);

    return connected() && (m_client->equipmentStatusMask() & valveMask) == 0U;
}

bool RemoteProcessController::tryStartBatch(const QString &operatorName)
{
    if (operatorName.trimmed().isEmpty() || state() != static_cast<int>(ControllerProtocol::ProcessState::Idle) || m_recoveryRequired)
    {
        return false;
    }

    if (!tankEmpty() || !devicesHealthy() || !mixerReady() || !valvesClosed())
    {
        return false;
    }

    return m_client->startBatch();
}

bool RemoteProcessController::pause()
{
    return m_client->pause();
}

bool RemoteProcessController::resume()
{
    return m_client->resume();
}

bool RemoteProcessController::startTransfer()
{
    return m_client->startTransfer();
}

bool RemoteProcessController::prepareNextBatch()
{
    return m_client->prepareNextBatch();
}

void RemoteProcessController::handleSnapshotChanged()
{
    if (!m_client->snapshotValid())
    {
        emit stateChanged();
        emit mixingTimeChanged();
        emit transferAllowedChanged();
        emit startConditionsChanged();
        return;
    }

    const int currentState = m_client->processState();
    const int idleState = static_cast<int>(ControllerProtocol::ProcessState::Idle);
    const int completeState = static_cast<int>(ControllerProtocol::ProcessState::Complete);
    const quint16 currentControllerSessionId = m_client->controllerSessionId();

    const bool interruptedRunWasActive = !m_batchId.isEmpty() && m_previousState != idleState && m_previousState != completeState;
    const bool controllerSessionChanged = m_lastControllerSessionId != 0U && currentControllerSessionId != 0U && currentControllerSessionId != m_lastControllerSessionId;

    if (controllerSessionChanged && interruptedRunWasActive && !m_recoveryRequired)
    {
        m_recoveryRequired = true;
        emit recoveryRequiredChanged();
        emit startConditionsChanged();
    }

    m_lastControllerSessionId = currentControllerSessionId;

    const int pausedState = static_cast<int>(ControllerProtocol::ProcessState::Paused);
    const int drainingState = static_cast<int>(ControllerProtocol::ProcessState::Draining);
    const int currentRunKind = m_client->runKind();
    const int batchRunKind = static_cast<int>(ControllerProtocol::RunKind::Batch);
    const int drainRunKind = static_cast<int>(ControllerProtocol::RunKind::Drain);
    const bool runIdentityAvailable = m_client->controllerSessionId() != 0U && m_client->runSequence() != 0U && (currentRunKind == batchRunKind || currentRunKind == drainRunKind);

    QString currentRunId;

    if (runIdentityAvailable)
    {
        currentRunId = QStringLiteral("esp-%1-%2").arg(m_client->controllerSessionId()).arg(m_client->runSequence());
    }

    if (m_batchId != currentRunId)
    {
        m_batchId = currentRunId;
        emit batchIdChanged();
    }

    if (!m_firstSnapshotReceived)
    {
        m_firstSnapshotReceived = true;
        emit initialSnapshotReceived();
    }

    if (m_restoredSnapshotPending)
    {
        m_restoredSnapshotPending = false;
        emit connectionRestored();
    }

    if (currentState != pausedState && currentState != drainingState)
    {
        m_lastActiveStageIndex = stageIndexForState(currentState);
    }

    m_previousState = currentState;

    emit stateChanged();
    emit mixingTimeChanged();
    emit transferAllowedChanged();
    emit startConditionsChanged();
}

void RemoteProcessController::handleConnectionChanged()
{
    if (m_client->isConnected())
    {
        if (m_connectedOnce && m_connectionInterrupted)
        {
            m_restoredSnapshotPending = true;
        }

        m_connectedOnce = true;
        m_connectionInterrupted = false;
    }
    else if (m_connectedOnce)
    {
        m_connectionInterrupted = true;
    }

    emit connectionChanged();
    emit startConditionsChanged();
}

int RemoteProcessController::stageIndexForState(int processState)
{
    switch (static_cast<ControllerProtocol::ProcessState>(processState))
    {
    case ControllerProtocol::ProcessState::Idle:
        return 0;
    case ControllerProtocol::ProcessState::FillingWater:
        return 1;
    case ControllerProtocol::ProcessState::DosingConcentrate:
        return 2;
    case ControllerProtocol::ProcessState::TemperatureCheck:
        return 3;
    case ControllerProtocol::ProcessState::Mixing:
        return 4;
    case ControllerProtocol::ProcessState::ReadyForTransfer:
    case ControllerProtocol::ProcessState::Transferring:
    case ControllerProtocol::ProcessState::Complete:
        return 5;
    case ControllerProtocol::ProcessState::Draining:
    case ControllerProtocol::ProcessState::Paused:
        return -1;
    }

    return -1;
}
