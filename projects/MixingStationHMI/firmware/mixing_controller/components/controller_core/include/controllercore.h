#ifndef CONTROLLERCORE_H
#define CONTROLLERCORE_H

#include "controllerprotocol.h"

#include <cstdint>

namespace MixingStation::Firmware
{
struct ProcessInputs
{
    std::uint16_t mixtureVolumeDecilitres{0};
    std::uint16_t temperatureTenthsCelsius{250};
    std::uint16_t faultMask{0};
};

struct ControllerSetpoints
{
    std::uint16_t pump1TargetRpm{600};
    std::uint16_t pump2TargetRpm{400};
    std::uint16_t pump3TargetRpm{600};
};

struct ControllerSnapshot
{
    std::uint16_t heartbeat{0};

    ControllerProtocol::ProcessState processState{ControllerProtocol::ProcessState::Idle};
    std::uint16_t stageProgressPermille{0};
    std::uint16_t mixtureVolumeDecilitres{0};
    std::uint16_t temperatureTenthsCelsius{0};

    std::uint16_t pump1ActualRpm{0};
    std::uint16_t pump2ActualRpm{0};
    std::uint16_t pump3ActualRpm{0};
    std::uint16_t mixerActualRpm{0};

    std::uint16_t equipmentStatusMask{0};
    std::uint16_t faultMask{0};
    std::uint16_t lastHandledCommandSequence{0};

    ControllerProtocol::CommandResult lastCommandResult{ControllerProtocol::CommandResult::None};
};

class ControllerCore
{
public:
    void updateInputs(const ProcessInputs &inputs);
    void updateSetpoints(const ControllerSetpoints &setpoints);

    [[nodiscard]] ControllerSetpoints setpoints() const;
    [[nodiscard]] ControllerSnapshot snapshot() const;
    [[nodiscard]] ControllerProtocol::CommandResult handleCommand(ControllerProtocol::Command command, std::uint16_t sequence);
    void tick();

private:
    ControllerSnapshot m_snapshot;
    ControllerSetpoints m_setpoints;
};

} // namespace MixingStation::Firmware

#endif // CONTROLLERCORE_H
