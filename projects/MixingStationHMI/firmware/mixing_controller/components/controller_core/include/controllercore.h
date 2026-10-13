#ifndef CONTROLLERCORE_H
#define CONTROLLERCORE_H

#include "controllerprotocol.h"
#include "processconfiguration.h"

#include <cstdint>

namespace MixingStation::Firmware
{
struct ProcessInputs
{
    std::uint16_t mixtureVolumeDecilitres{0};
    std::uint16_t temperatureTenthsCelsius{250};
    std::uint16_t faultMask{0};
    std::uint16_t waterVolumeDecilitres{0};
    std::uint16_t concentrateVolumeDecilitres{0};
    bool mixerConnected{false};
};

struct ControllerSetpoints
{
    std::uint16_t pump1TargetRpm{ProcessConfiguration::Pump1DefaultRpm};
    std::uint16_t pump2TargetRpm{ProcessConfiguration::Pump2DefaultRpm};
    std::uint16_t pump3TargetRpm{ProcessConfiguration::Pump3DefaultRpm};
};

struct ControllerSnapshot
{
    std::uint16_t heartbeat{0};
    std::uint16_t controllerSessionId{0};
    std::uint16_t runSequence{0};

    ControllerProtocol::RunKind runKind{ControllerProtocol::RunKind::None};

    ControllerProtocol::ProcessState processState{ControllerProtocol::ProcessState::Idle};
    std::uint16_t stageProgressPermille{0};
    std::uint16_t mixtureVolumeDecilitres{0};
    std::uint16_t temperatureTenthsCelsius{0};
    std::uint16_t waterVolumeDecilitres{0};
    std::uint16_t concentrateVolumeDecilitres{0};

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
    void setControllerSessionId(std::uint16_t sessionId);
    void updateInputs(const ProcessInputs &inputs);
    void updateSetpoints(const ControllerSetpoints &setpoints);
    void tick(std::uint32_t elapsedMilliseconds = ProcessConfiguration::SimulationStepMilliseconds);

    [[nodiscard]] ControllerSetpoints setpoints() const;
    [[nodiscard]] ControllerSnapshot snapshot() const;
    [[nodiscard]] ControllerProtocol::CommandResult handleCommand(ControllerProtocol::Command command, std::uint16_t sequence);
    [[nodiscard]] static std::uint16_t faultMaskForState(ControllerProtocol::ProcessState state);

private:
    void transitionTo(ControllerProtocol::ProcessState state);
    void applyOutputsForState(ControllerProtocol::ProcessState state);
    void beginRun(ControllerProtocol::RunKind runKind);

    [[nodiscard]] static bool stateCanBePaused(ControllerProtocol::ProcessState state);
    [[nodiscard]] static bool stateRequiresMixer(ControllerProtocol::ProcessState state);

    ControllerSnapshot m_snapshot;
    ControllerSetpoints m_setpoints;
    ControllerProtocol::ProcessState m_stateBeforePause{ControllerProtocol::ProcessState::Idle};
    std::uint32_t m_mixingElapsedMilliseconds{0};
    std::uint16_t m_activeInputFaultMask{0};
};

} // namespace MixingStation::Firmware

#endif // CONTROLLERCORE_H
