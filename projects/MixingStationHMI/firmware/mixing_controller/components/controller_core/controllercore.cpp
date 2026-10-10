#include "controllercore.h"

namespace MixingStation::Firmware
{

ControllerSnapshot ControllerCore::snapshot() const
{
    return m_snapshot;
}

ControllerProtocol::CommandResult ControllerCore::handleCommand(
    ControllerProtocol::Command command,
    std::uint16_t sequence)
{
    if (sequence == m_snapshot.lastHandledCommandSequence) {
        m_snapshot.lastCommandResult =
            ControllerProtocol::CommandResult::DuplicateSequence;

        return m_snapshot.lastCommandResult;
    }

    ControllerProtocol::CommandResult result =
        ControllerProtocol::CommandResult::UnsupportedCommand;

    switch (command) {
    case ControllerProtocol::Command::StartBatch:
        if (m_snapshot.processState !=
            ControllerProtocol::ProcessState::Idle) {
            result = ControllerProtocol::CommandResult::InvalidState;
            break;
        }

        m_snapshot.processState =
            ControllerProtocol::ProcessState::FillingWater;
        m_snapshot.stageProgressPermille = 0;
        result = ControllerProtocol::CommandResult::Accepted;
        break;

    case ControllerProtocol::Command::None:
    case ControllerProtocol::Command::PauseBatch:
    case ControllerProtocol::Command::ResumeBatch:
    case ControllerProtocol::Command::StartTransfer:
    case ControllerProtocol::Command::ResetFaults:
    case ControllerProtocol::Command::StartDrain:
    case ControllerProtocol::Command::PrepareNextBatch:
        result =
            ControllerProtocol::CommandResult::UnsupportedCommand;
        break;
    }

    m_snapshot.lastHandledCommandSequence = sequence;
    m_snapshot.lastCommandResult = result;

    return result;
}

void ControllerCore::tick()
{
    ++m_snapshot.heartbeat;
}

} // namespace MixingStation::Firmware
