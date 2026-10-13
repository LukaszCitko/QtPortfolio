#include "remoteprocesssynchronizer.h"

#include "communication/modbuscontrollerclient.h"
#include "controllerprotocol.h"
#include "mixer.h"
#include "mixingtank.h"
#include "processconfiguration.h"
#include "pump.h"
#include "valve.h"

namespace
{

bool equipmentBitIsSet(quint16 mask, ControllerProtocol::EquipmentStatusBit bit)
{
    return (mask & static_cast<quint16>(bit)) != 0U;
}

bool faultBitIsSet(quint16 mask, ControllerProtocol::FaultBit bit)
{
    return (mask & static_cast<quint16>(bit)) != 0U;
}

MixingTank::State tankStateForProcessState(int processState)
{
    switch (static_cast<ControllerProtocol::ProcessState>(processState))
    {
    case ControllerProtocol::ProcessState::Idle:
        return MixingTank::State::Empty;
    case ControllerProtocol::ProcessState::FillingWater:
    case ControllerProtocol::ProcessState::DosingConcentrate:
        return MixingTank::State::Filling;
    case ControllerProtocol::ProcessState::TemperatureCheck:
        return MixingTank::State::ReadyForMixing;
    case ControllerProtocol::ProcessState::Mixing:
        return MixingTank::State::Mixing;
    case ControllerProtocol::ProcessState::ReadyForTransfer:
        return MixingTank::State::ReadyForTransfer;
    case ControllerProtocol::ProcessState::Transferring:
        return MixingTank::State::Transferring;
    case ControllerProtocol::ProcessState::Draining:
        return MixingTank::State::Draining;
    case ControllerProtocol::ProcessState::Complete:
        return MixingTank::State::Complete;
    case ControllerProtocol::ProcessState::Paused:
        return MixingTank::State::Fault;
    }

    return MixingTank::State::Fault;
}

} // namespace

RemoteProcessSynchronizer::RemoteProcessSynchronizer(ModbusControllerClient *client, Pump *pump1, Valve *valve1, Pump *pump2, Valve *valve2, Pump *pump3, Valve *valve3, Valve *valve4, Mixer *mixer, MixingTank *tank, QObject *parent)
    : QObject(parent),
      m_client(client),
      m_pump1(pump1),
      m_valve1(valve1),
      m_pump2(pump2),
      m_valve2(valve2),
      m_pump3(pump3),
      m_valve3(valve3),
      m_valve4(valve4),
      m_mixer(mixer),
      m_tank(tank),
      m_lastActiveProcessState(static_cast<int>(ControllerProtocol::ProcessState::Idle))
{
    Q_ASSERT(m_client);
    Q_ASSERT(m_pump1);
    Q_ASSERT(m_valve1);
    Q_ASSERT(m_pump2);
    Q_ASSERT(m_valve2);
    Q_ASSERT(m_pump3);
    Q_ASSERT(m_valve3);
    Q_ASSERT(m_valve4);
    Q_ASSERT(m_mixer);
    Q_ASSERT(m_tank);

    connect(m_client, &ModbusControllerClient::snapshotChanged, this, &RemoteProcessSynchronizer::synchronizeSnapshot);
}

void RemoteProcessSynchronizer::synchronizeSnapshot()
{
    if (!m_client->snapshotValid())
    {
        return;
    }

    const quint16 equipmentMask = m_client->equipmentStatusMask();
    const quint16 faultMask = m_client->faultMask();
    const int processState = m_client->processState();

    const auto synchronizePump = [equipmentMask, faultMask](Pump *pump, ControllerProtocol::EquipmentStatusBit runningBit, ControllerProtocol::FaultBit faultBit, int actualRpm)
    {
        const bool fault = faultBitIsSet(faultMask, faultBit);
        const bool running = equipmentBitIsSet(equipmentMask, runningBit);

        if (fault)
        {
            if (!pump->hasFault())
            {
                pump->setFault();
            }
        }
        else
        {
            if (pump->hasFault())
            {
                pump->resetFault();
            }

            if (running && !pump->isRunning())
            {
                pump->start();
            }
            else if (!running && pump->isRunning())
            {
                pump->stop();
            }
        }

        pump->setActualRpm(static_cast<double>(actualRpm));
    };

    const auto synchronizeValve = [equipmentMask, faultMask](Valve *valve, ControllerProtocol::EquipmentStatusBit openBit, ControllerProtocol::FaultBit faultBit)
    {
        const bool fault = faultBitIsSet(faultMask, faultBit);
        const bool open = equipmentBitIsSet(equipmentMask, openBit);

        if (fault)
        {
            if (!valve->hasFault())
            {
                valve->setFault();
            }

            return;
        }

        if (valve->hasFault())
        {
            valve->resetFault();
        }

        if (open && !valve->isOpen())
        {
            valve->open();
        }
        else if (!open && valve->isOpen())
        {
            valve->close();
        }
    };

    synchronizePump(m_pump1, ControllerProtocol::EquipmentStatusBit::Pump1Running, ControllerProtocol::FaultBit::Pump1, m_client->pump1ActualRpm());
    synchronizePump(m_pump2, ControllerProtocol::EquipmentStatusBit::Pump2Running, ControllerProtocol::FaultBit::Pump2, m_client->pump2ActualRpm());
    synchronizePump(m_pump3, ControllerProtocol::EquipmentStatusBit::Pump3Running, ControllerProtocol::FaultBit::Pump3, m_client->pump3ActualRpm());

    synchronizeValve(m_valve1, ControllerProtocol::EquipmentStatusBit::Valve1Open, ControllerProtocol::FaultBit::Valve1);
    synchronizeValve(m_valve2, ControllerProtocol::EquipmentStatusBit::Valve2Open, ControllerProtocol::FaultBit::Valve2);
    synchronizeValve(m_valve3, ControllerProtocol::EquipmentStatusBit::Valve3Open, ControllerProtocol::FaultBit::Valve3);

    const bool drainOpen = equipmentBitIsSet(equipmentMask, ControllerProtocol::EquipmentStatusBit::DrainValveOpen);

    if (drainOpen && !m_valve4->isOpen())
    {
        m_valve4->open();
    }
    else if (!drainOpen && m_valve4->isOpen())
    {
        m_valve4->close();
    }

    const bool mixerConnected = equipmentBitIsSet(equipmentMask, ControllerProtocol::EquipmentStatusBit::MixerConnected);
    const bool mixerRunning = equipmentBitIsSet(equipmentMask, ControllerProtocol::EquipmentStatusBit::MixerRunning);
    const bool mixerFault = faultBitIsSet(faultMask, ControllerProtocol::FaultBit::Mixer);

    if (!mixerRunning && m_mixer->isRunning())
    {
        m_mixer->stop();
    }

    if (mixerConnected && !m_mixer->isConnected())
    {
        m_mixer->connect();
    }
    else if (!mixerConnected && m_mixer->isConnected())
    {
        m_mixer->disconnectDevice();
    }

    if (mixerFault)
    {
        if (!m_mixer->hasFault())
        {
            m_mixer->setFault();
        }
    }
    else
    {
        if (m_mixer->hasFault())
        {
            m_mixer->resetFault();
        }

        if (mixerRunning && !m_mixer->isRunning())
        {
            m_mixer->start();
        }
        else if (!mixerRunning && m_mixer->isRunning())
        {
            m_mixer->stop();
        }
    }

    if (mixerRunning)
    {
        const double targetRpm = processState == static_cast<int>(ControllerProtocol::ProcessState::Mixing) ? static_cast<double>(ProcessConfiguration::MixingMixerRpm) : static_cast<double>(ProcessConfiguration::DosingMixerRpm);
        m_mixer->setTargetRpm(targetRpm);
    }

    m_mixer->setActualRpm(static_cast<double>(m_client->mixerActualRpm()));

    const int displayedProcessState = effectiveProcessState(processState);
    MixingTank::State tankState = tankStateForProcessState(displayedProcessState);

    if (faultBitIsSet(faultMask, ControllerProtocol::FaultBit::TemperatureHigh))
    {
        tankState = MixingTank::State::Fault;
    }

    m_tank->synchronizeFromController(m_client->waterVolumeLitres(), m_client->concentrateVolumeLitres(), m_client->temperatureCelsius(), tankState);
}

int RemoteProcessSynchronizer::effectiveProcessState(int processState)
{
    if (processState == static_cast<int>(ControllerProtocol::ProcessState::Paused))
    {
        return m_lastActiveProcessState;
    }

    m_lastActiveProcessState = processState;
    return processState;
}
