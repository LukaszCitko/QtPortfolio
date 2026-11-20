#ifndef COMMANDOUTCOMETRACKER_H
#define COMMANDOUTCOMETRACKER_H

#include <QtGlobal>
#include <optional>

class CommandOutcomeTracker
{
public:
    enum class Outcome { Accepted, Rejected, Unknown };

    struct Result {
        quint16 code;
        quint16 sequence;
        Outcome outcome;
        int controllerResult;
        bool late;
    };

    bool begin(quint16 code, quint16 sequence);
    std::optional<Result> acknowledge(quint16 sequence, int controllerResult);
    std::optional<Result> expire();
    std::optional<Result> disconnect();
    bool pending() const;
    bool pending(quint16 sequence) const;
    void cancelPending();

private:
    struct Command {
        quint16 code;
        quint16 sequence;
    };

    std::optional<Command> m_pending;
    std::optional<Command> m_unknown;
};

#endif
