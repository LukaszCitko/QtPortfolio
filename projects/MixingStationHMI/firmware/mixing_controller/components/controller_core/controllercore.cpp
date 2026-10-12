#include "controllercore.h"

#include <algorithm>

namespace
{

constexpr std::uint16_t WaterFillingEquipmentMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump1Running)
                                                  | static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Valve1Open);

constexpr std::uint16_t ConcentrateFlowEquipmentMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump2Running)
                                                     | static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Valve2Open);

constexpr std::uint16_t TransferEquipmentMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump3Running)
                                              | static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Valve3Open);

constexpr std::uint16_t MixerRunningMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::MixerRunning);
constexpr std::uint16_t MixerConnectedMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::MixerConnected);
constexpr std::uint16_t DrainValveMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::DrainValveOpen);

constexpr std::uint16_t ControlledEquipmentMask = WaterFillingEquipmentMask
                                                | ConcentrateFlowEquipmentMask
                                                | TransferEquipmentMask
                                                | MixerRunningMask
                                                | DrainValveMask;

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
    m_activeInputFaultMask = inputs.faultMask & ControllerProtocol::DefinedFaultMask;
    m_snapshot.faultMask |= m_activeInputFaultMask;

    if (inputs.mixerConnected)
    {
        m_snapshot.equipmentStatusMask |= MixerConnectedMask;
    }
    else
    {
        m_snapshot.equipmentStatusMask &= static_cast<std::uint16_t>(~MixerConnectedMask);
    }
}

ControllerSnapshot ControllerCore::snapshot() const
{
    return m_snapshot;
}

ControllerProtocol::CommandResult ControllerCore::handleCommand(ControllerProtocol::Command command, std::uint16_t sequence)
{
    if (sequence == m_snapshot.lastHandledCommandSequence)
    {
        m_snapshot.lastCommandResult = ControllerProtocol::CommandResult::DuplicateSequence;
        return m_snapshot.lastCommandResult;
    }

    ControllerProtocol::CommandResult result = ControllerProtocol::CommandResult::UnsupportedCommand;

    switch (command)
    {
    case ControllerProtocol::Command::StartBatch:
        if (m_snapshot.processState != ControllerProtocol::ProcessState::Idle
            || m_snapshot.mixtureVolumeDecilitres != 0U
            || (m_snapshot.equipmentStatusMask & MixerConnectedMask) == 0U)
        {
            result = ControllerProtocol::CommandResult::InvalidState;
            break;
        }

        transitionTo(ControllerProtocol::ProcessState::FillingWater);
        result = ControllerProtocol::CommandResult::Accepted;
        break;

    case ControllerProtocol::Command::PauseBatch:
        if (!stateCanBePaused(m_snapshot.processState))
        {
            result = ControllerProtocol::CommandResult::InvalidState;
            break;
        }

        m_stateBeforePause = m_snapshot.processState;
        m_snapshot.processState = ControllerProtocol::ProcessState::Paused;
        applyOutputsForState(ControllerProtocol::ProcessState::Paused);
        result = ControllerProtocol::CommandResult::Accepted;
        break;

    case ControllerProtocol::Command::ResumeBatch:
        if (m_snapshot.processState != ControllerProtocol::ProcessState::Paused)
        {
            result = ControllerProtocol::CommandResult::InvalidState;
            break;
        }

        if (stateRequiresMixer(m_stateBeforePause) && (m_snapshot.equipmentStatusMask & MixerConnectedMask) == 0U)
        {
            result = ControllerProtocol::CommandResult::InvalidState;
            break;
        }

        m_snapshot.processState = m_stateBeforePause;
        applyOutputsForState(m_stateBeforePause);
        result = ControllerProtocol::CommandResult::Accepted;
        break;

    case ControllerProtocol::Command::StartTransfer:
        if (m_snapshot.processState != ControllerProtocol::ProcessState::ReadyForTransfer)
        {
            result = ControllerProtocol::CommandResult::InvalidState;
            break;
        }

        transitionTo(ControllerProtocol::ProcessState::Transferring);
        result = ControllerProtocol::CommandResult::Accepted;
        break;

    case ControllerProtocol::Command::StartDrain:
        if ((m_snapshot.processState != ControllerProtocol::ProcessState::Idle
             && m_snapshot.processState != ControllerProtocol::ProcessState::Paused)
            || m_snapshot.mixtureVolumeDecilitres == 0U)
        {
            result = ControllerProtocol::CommandResult::InvalidState;
            break;
        }

        transitionTo(ControllerProtocol::ProcessState::Draining);
        result = ControllerProtocol::CommandResult::Accepted;
        break;

    case ControllerProtocol::Command::PrepareNextBatch:
        if (m_snapshot.processState != ControllerProtocol::ProcessState::Complete || m_snapshot.mixtureVolumeDecilitres != 0U)
        {
            result = ControllerProtocol::CommandResult::InvalidState;
            break;
        }

        transitionTo(ControllerProtocol::ProcessState::Idle);
        result = ControllerProtocol::CommandResult::Accepted;
        break;

    case ControllerProtocol::Command::ResetFaults:
        if (m_activeInputFaultMask != 0U || m_snapshot.faultMask == 0U)
        {
            result = ControllerProtocol::CommandResult::InvalidState;
            break;
        }

        m_snapshot.faultMask = 0;
        result = ControllerProtocol::CommandResult::Accepted;
        break;

    case ControllerProtocol::Command::None:
        result = ControllerProtocol::CommandResult::UnsupportedCommand;
        break;

    }

    m_snapshot.lastHandledCommandSequence = sequence;
    m_snapshot.lastCommandResult = result;

    return result;
}

void ControllerCore::tick(std::uint32_t elapsedMilliseconds)
{
    ++m_snapshot.heartbeat;

    const std::uint16_t relevantFaultMask = faultMaskForState(m_snapshot.processState);

    if ((m_snapshot.faultMask & relevantFaultMask) != 0U)
    {
        m_stateBeforePause = m_snapshot.processState;
        m_snapshot.processState = ControllerProtocol::ProcessState::Paused;
        applyOutputsForState(ControllerProtocol::ProcessState::Paused);
        return;
    }

    if (stateRequiresMixer(m_snapshot.processState) && (m_snapshot.equipmentStatusMask & MixerConnectedMask) == 0U)
    {
        m_stateBeforePause = m_snapshot.processState;
        m_snapshot.processState = ControllerProtocol::ProcessState::Paused;
        applyOutputsForState(ControllerProtocol::ProcessState::Paused);
        return;
    }

    switch (m_snapshot.processState)
    {
    case ControllerProtocol::ProcessState::FillingWater:
        if (m_snapshot.mixtureVolumeDecilitres < ProcessConfiguration::WaterTargetDecilitres)
        {
            m_snapshot.stageProgressPermille = static_cast<std::uint16_t>((static_cast<std::uint32_t>(m_snapshot.mixtureVolumeDecilitres) * 1000U) / ProcessConfiguration::WaterTargetDecilitres);
            return;
        }

        transitionTo(ControllerProtocol::ProcessState::DosingConcentrate);
        return;

    case ControllerProtocol::ProcessState::DosingConcentrate:
        if (m_snapshot.mixtureVolumeDecilitres < ProcessConfiguration::MixtureTargetDecilitres)
        {
            const std::uint16_t concentrateVolumeDecilitres = m_snapshot.mixtureVolumeDecilitres > ProcessConfiguration::WaterTargetDecilitres ? m_snapshot.mixtureVolumeDecilitres - ProcessConfiguration::WaterTargetDecilitres : 0U;
            m_snapshot.stageProgressPermille = static_cast<std::uint16_t>((static_cast<std::uint32_t>(concentrateVolumeDecilitres) * 1000U) / ProcessConfiguration::ConcentrateTargetDecilitres);
            return;
        }

        transitionTo(ControllerProtocol::ProcessState::TemperatureCheck);
        return;

    case ControllerProtocol::ProcessState::TemperatureCheck:
        if (m_snapshot.temperatureTenthsCelsius >= ProcessConfiguration::MinimumProcessTemperatureTenthsCelsius
            && m_snapshot.temperatureTenthsCelsius <= ProcessConfiguration::MaximumProcessTemperatureTenthsCelsius)
        {
            transitionTo(ControllerProtocol::ProcessState::Mixing);
        }
        return;

    case ControllerProtocol::ProcessState::Mixing:
        m_mixingElapsedMilliseconds = std::min(m_mixingElapsedMilliseconds + elapsedMilliseconds, ProcessConfiguration::MixingDurationMilliseconds);
        m_snapshot.stageProgressPermille = static_cast<std::uint16_t>((m_mixingElapsedMilliseconds * 1000U) / ProcessConfiguration::MixingDurationMilliseconds);

        if (m_mixingElapsedMilliseconds >= ProcessConfiguration::MixingDurationMilliseconds)
        {
            transitionTo(ControllerProtocol::ProcessState::ReadyForTransfer);
            m_snapshot.stageProgressPermille = 1000;
        }
        return;

    case ControllerProtocol::ProcessState::Transferring:
        if (m_snapshot.mixtureVolumeDecilitres > 0U)
        {
            const std::uint16_t transferredVolumeDecilitres = m_snapshot.mixtureVolumeDecilitres < ProcessConfiguration::MixtureTargetDecilitres ? ProcessConfiguration::MixtureTargetDecilitres - m_snapshot.mixtureVolumeDecilitres : 0U;
            m_snapshot.stageProgressPermille = static_cast<std::uint16_t>((static_cast<std::uint32_t>(transferredVolumeDecilitres) * 1000U) / ProcessConfiguration::MixtureTargetDecilitres);
            return;
        }

        transitionTo(ControllerProtocol::ProcessState::Complete);
        m_snapshot.stageProgressPermille = 1000;
        return;

    case ControllerProtocol::ProcessState::Draining:
        if (m_snapshot.mixtureVolumeDecilitres > 0U)
        {
            const std::uint16_t drainedVolumeDecilitres = m_snapshot.mixtureVolumeDecilitres < ProcessConfiguration::MixtureTargetDecilitres ? ProcessConfiguration::MixtureTargetDecilitres - m_snapshot.mixtureVolumeDecilitres : 0U;
            m_snapshot.stageProgressPermille = static_cast<std::uint16_t>((static_cast<std::uint32_t>(drainedVolumeDecilitres) * 1000U) / ProcessConfiguration::MixtureTargetDecilitres);
            return;
        }

        transitionTo(ControllerProtocol::ProcessState::Complete);
        m_snapshot.stageProgressPermille = 1000;
        return;

    case ControllerProtocol::ProcessState::Idle:
    case ControllerProtocol::ProcessState::ReadyForTransfer:
    case ControllerProtocol::ProcessState::Paused:
    case ControllerProtocol::ProcessState::Complete:
        return;
    }
}

void ControllerCore::transitionTo(ControllerProtocol::ProcessState state)
{
    m_snapshot.processState = state;
    m_snapshot.stageProgressPermille = 0;

    if (state == ControllerProtocol::ProcessState::Mixing)
    {
        m_mixingElapsedMilliseconds = 0;
    }

    applyOutputsForState(state);
}

void ControllerCore::applyOutputsForState(ControllerProtocol::ProcessState state)
{
    m_snapshot.pump1ActualRpm = 0;
    m_snapshot.pump2ActualRpm = 0;
    m_snapshot.pump3ActualRpm = 0;
    m_snapshot.mixerActualRpm = 0;
    m_snapshot.equipmentStatusMask &= static_cast<std::uint16_t>(~ControlledEquipmentMask);

    switch (state)
    {
    case ControllerProtocol::ProcessState::FillingWater:
        m_snapshot.pump1ActualRpm = m_setpoints.pump1TargetRpm;
        m_snapshot.equipmentStatusMask |= WaterFillingEquipmentMask;
        break;

    case ControllerProtocol::ProcessState::DosingConcentrate:
        m_snapshot.pump2ActualRpm = m_setpoints.pump2TargetRpm;
        m_snapshot.mixerActualRpm = ProcessConfiguration::DosingMixerRpm;
        m_snapshot.equipmentStatusMask |= ConcentrateFlowEquipmentMask | MixerRunningMask;
        break;

    case ControllerProtocol::ProcessState::TemperatureCheck:
    case ControllerProtocol::ProcessState::ReadyForTransfer:
        m_snapshot.mixerActualRpm = ProcessConfiguration::DosingMixerRpm;
        m_snapshot.equipmentStatusMask |= MixerRunningMask;
        break;

    case ControllerProtocol::ProcessState::Mixing:
        m_snapshot.mixerActualRpm = ProcessConfiguration::MixingMixerRpm;
        m_snapshot.equipmentStatusMask |= MixerRunningMask;
        break;

    case ControllerProtocol::ProcessState::Transferring:
        m_snapshot.pump3ActualRpm = m_setpoints.pump3TargetRpm;
        m_snapshot.equipmentStatusMask |= TransferEquipmentMask;
        break;

    case ControllerProtocol::ProcessState::Draining:
        m_snapshot.equipmentStatusMask |= DrainValveMask;
        break;

    case ControllerProtocol::ProcessState::Idle:
    case ControllerProtocol::ProcessState::Paused:
    case ControllerProtocol::ProcessState::Complete:
        break;
    }
}

bool ControllerCore::stateCanBePaused(ControllerProtocol::ProcessState state)
{
    return state == ControllerProtocol::ProcessState::FillingWater
        || state == ControllerProtocol::ProcessState::DosingConcentrate
        || state == ControllerProtocol::ProcessState::TemperatureCheck
        || state == ControllerProtocol::ProcessState::Mixing
        || state == ControllerProtocol::ProcessState::ReadyForTransfer
        || state == ControllerProtocol::ProcessState::Transferring
        || state == ControllerProtocol::ProcessState::Draining;
}

bool ControllerCore::stateRequiresMixer(ControllerProtocol::ProcessState state)
{
    return state == ControllerProtocol::ProcessState::FillingWater
        || state == ControllerProtocol::ProcessState::DosingConcentrate
        || state == ControllerProtocol::ProcessState::TemperatureCheck
        || state == ControllerProtocol::ProcessState::Mixing
        || state == ControllerProtocol::ProcessState::ReadyForTransfer;
}

std::uint16_t ControllerCore::faultMaskForState(ControllerProtocol::ProcessState state)
{
    switch (state)
    {
    case ControllerProtocol::ProcessState::FillingWater:
        return static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Pump1) | static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Valve1);

    case ControllerProtocol::ProcessState::DosingConcentrate:
        return static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Pump2)
             | static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Valve2)
             | static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Mixer);

    case ControllerProtocol::ProcessState::TemperatureCheck:
    case ControllerProtocol::ProcessState::Mixing:
    case ControllerProtocol::ProcessState::ReadyForTransfer:
        return static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Mixer);

    case ControllerProtocol::ProcessState::Transferring:
        return static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Pump3) | static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Valve3);

    case ControllerProtocol::ProcessState::Idle:
    case ControllerProtocol::ProcessState::Draining:
    case ControllerProtocol::ProcessState::Paused:
    case ControllerProtocol::ProcessState::Complete:
        return 0;
    }

    return 0;
}

} // namespace MixingStation::Firmware