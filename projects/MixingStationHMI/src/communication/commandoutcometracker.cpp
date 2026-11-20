#include "commandoutcometracker.h"
#include "controllerprotocol.h"

namespace
{
std::optional<CommandOutcomeTracker::Outcome> classifyResult(int result)
{
    using CommandResult = ControllerProtocol::CommandResult;

    switch (static_cast<CommandResult>(result)) {
    case CommandResult::Accepted:
        return CommandOutcomeTracker::Outcome::Accepted;
    case CommandResult::DuplicateSequence:
    case CommandResult::InvalidState:
    case CommandResult::UnsupportedCommand:
        return CommandOutcomeTracker::Outcome::Rejected;
    case CommandResult::None:
        return std::nullopt;
    }

    return std::nullopt;
}
}

bool CommandOutcomeTracker::begin(quint16 code, quint16 sequence)
{
    if (code == 0 || sequence == 0 || m_pending || (m_unknown && m_unknown->sequence == sequence))
        return false;

    m_pending = Command{code, sequence};
    return true;
}

std::optional<CommandOutcomeTracker::Result> CommandOutcomeTracker::acknowledge(quint16 sequence, int controllerResult)
{
    const std::optional<Outcome> outcome = classifyResult(controllerResult);
    if (!outcome)
        return std::nullopt;

    if (m_pending && m_pending->sequence == sequence) {
        const Command command = *m_pending;
        m_pending.reset();
        return Result{command.code, command.sequence, *outcome, controllerResult, false};
    }

    if (m_unknown && m_unknown->sequence == sequence) {
        const Command command = *m_unknown;
        m_unknown.reset();
        return Result{command.code, command.sequence, *outcome, controllerResult, true};
    }

    return std::nullopt;
}

std::optional<CommandOutcomeTracker::Result> CommandOutcomeTracker::expire()
{
    if (!m_pending)
        return std::nullopt;

    const Command command = *m_pending;
    m_pending.reset();
    m_unknown = command;
    return Result{command.code, command.sequence, Outcome::Unknown, 0, false};
}

std::optional<CommandOutcomeTracker::Result> CommandOutcomeTracker::disconnect()
{
    return expire();
}

bool CommandOutcomeTracker::pending() const
{
    return m_pending.has_value();
}

void CommandOutcomeTracker::cancelPending()
{
    m_pending.reset();
}

bool CommandOutcomeTracker::pending(quint16 sequence) const
{
    return m_pending && m_pending->sequence == sequence;
}
