#ifndef SIMULATEDPROCESSINPUTS_H
#define SIMULATEDPROCESSINPUTS_H

#include "controllercore.h"

#include <cstdint>

namespace MixingStation::Firmware
{

class SimulatedProcessInputs final
{
public:
    void tick(const ControllerSnapshot &snapshot, std::uint32_t elapsedMilliseconds);
    [[nodiscard]] ProcessInputs inputs() const;

private:
    ProcessInputs m_inputs;
    std::uint32_t m_volumeRemainder{0};
};

} // namespace MixingStation::Firmware

#endif // SIMULATEDPROCESSINPUTS_H