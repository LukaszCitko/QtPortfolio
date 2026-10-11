#include <algorithm>
#include "controllercore.h"

namespace
{

constexpr std::uint16_t WaterFillingEquipmentMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump1Running)
                                                  | static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Valve1Open);

constexpr std::uint16_t ConcentrateFlowEquipmentMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump2Running)
                                                     | static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Valve2Open);

constexpr std::uint16_t MixerEquipmentMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::MixerRunning)
                                           | static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::MixerConnected);

} // namespace

namespace MixingStation::Firmware
{

    void ControllerCore::updateSetpoints(const ControllerSetpoints &setpoints)
{
    m_setpoints.pump1TargetRpm = std::min(setpoints.pump1TargetRpm, ControllerProtocol::MaximumPumpRpm);
    m_setpoints.pump2TargetRpm = std::min(setpoints.pump2TargetRpm, ControllerProtocol::MaximumPumpRpm);
    m_setpoints.pump3TargetRpm = std::min(setpoints.pump3TargetRpm, ControllerProtocol::MaximumPumpRpm);
}

ControllerSetpoints ControllerCore::setpoints() const
{
    return m_setpoints;
}

void ControllerCore::updateInputs(const ProcessInputs &inputs)
{
    m_snapshot.mixtureVolumeDecilitres = inputs.mixtureVolumeDecilitres;
    m_snapshot.temperatureTenthsCelsius = inputs.temperatureTenthsCelsius;
    m_snapshot.faultMask = inputs.faultMask;
}

ControllerSnapshot ControllerCore::snapshot() const
{
    return m_snapshot;
}

ControllerProtocol::CommandResult ControllerCore::handleCommand(
    ControllerProtocol::Command command,
    std::uint16_t sequence)
{
    if (sequence == m_snapshot.lastHandledCommandSequence) {
        m_snapshot.lastCommandResult = ControllerProtocol::CommandResult::DuplicateSequence;
        return m_snapshot.lastCommandResult;
    }

    ControllerProtocol::CommandResult result = ControllerProtocol::CommandResult::UnsupportedCommand;

    switch (command) {
    case ControllerProtocol::Command::StartBatch:
        if (m_snapshot.processState != ControllerProtocol::ProcessState::Idle) {
            result = ControllerProtocol::CommandResult::InvalidState;
            break;
        }

        m_snapshot.processState = ControllerProtocol::ProcessState::FillingWater;
        m_snapshot.stageProgressPermille = 0;
        m_snapshot.pump1ActualRpm = m_setpoints.pump1TargetRpm;

        m_snapshot.equipmentStatusMask |= WaterFillingEquipmentMask;
        result = ControllerProtocol::CommandResult::Accepted;
    break;

    case ControllerProtocol::Command::None:
    case ControllerProtocol::Command::PauseBatch:
    case ControllerProtocol::Command::ResumeBatch:
    case ControllerProtocol::Command::StartTransfer:
    case ControllerProtocol::Command::ResetFaults:
    case ControllerProtocol::Command::StartDrain:
    case ControllerProtocol::Command::PrepareNextBatch:
        result = ControllerProtocol::CommandResult::UnsupportedCommand;
    break;
    }

    m_snapshot.lastHandledCommandSequence = sequence;
    m_snapshot.lastCommandResult = result;

    return result;
}

void ControllerCore::tick()
{
    ++m_snapshot.heartbeat;

    if (m_snapshot.processState == ControllerProtocol::ProcessState::FillingWater)
    {
        if (m_snapshot.mixtureVolumeDecilitres < ProcessConfiguration::WaterTargetDecilitres)
        {
            m_snapshot.stageProgressPermille = static_cast<std::uint16_t>((static_cast<std::uint32_t>(m_snapshot.mixtureVolumeDecilitres) * 1000U) / ProcessConfiguration::WaterTargetDecilitres);
            return;
        }

        m_snapshot.processState = ControllerProtocol::ProcessState::DosingConcentrate;
        m_snapshot.stageProgressPermille = 0;

        m_snapshot.pump1ActualRpm = 0;
        m_snapshot.pump2ActualRpm = m_setpoints.pump2TargetRpm;
        m_snapshot.mixerActualRpm = ProcessConfiguration::DosingMixerRpm;

        m_snapshot.equipmentStatusMask &= static_cast<std::uint16_t>(~WaterFillingEquipmentMask);
        m_snapshot.equipmentStatusMask |= ConcentrateFlowEquipmentMask;
        m_snapshot.equipmentStatusMask |= MixerEquipmentMask;
        return;
    }

    if (m_snapshot.processState != ControllerProtocol::ProcessState::DosingConcentrate)
    {
        return;
    }

    if (m_snapshot.mixtureVolumeDecilitres < ProcessConfiguration::MixtureTargetDecilitres)
    {
        const std::uint16_t concentrateVolumeDecilitres = m_snapshot.mixtureVolumeDecilitres > ProcessConfiguration::WaterTargetDecilitres ? m_snapshot.mixtureVolumeDecilitres - ProcessConfiguration::WaterTargetDecilitres : 0;
        m_snapshot.stageProgressPermille = static_cast<std::uint16_t>((static_cast<std::uint32_t>(concentrateVolumeDecilitres) * 1000U) / ProcessConfiguration::ConcentrateTargetDecilitres);
        return;
    }

    m_snapshot.processState = ControllerProtocol::ProcessState::TemperatureCheck;
    m_snapshot.stageProgressPermille = 0;
    m_snapshot.pump2ActualRpm = 0;
    m_snapshot.equipmentStatusMask &= static_cast<std::uint16_t>(~ConcentrateFlowEquipmentMask);
}
} // namespace MixingStation::Firmware
