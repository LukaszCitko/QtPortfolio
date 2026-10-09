#ifndef CONTROLLERCORE_H
#define CONTROLLERCORE_H

#include "controllerprotocol.h"

#include <cstdint>

namespace MixingStation::Firmware
{

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
};

class ControllerCore
{
public:
    [[nodiscard]] ControllerSnapshot snapshot() const;

    void tick();

private:
    ControllerSnapshot m_snapshot;
};

} // namespace MixingStation::Firmware

#endif // CONTROLLERCORE_H
