#ifndef SIMULATEDPROCESSINPUTS_H
#define SIMULATEDPROCESSINPUTS_H

#include "controllercore.h"

#include <cstdint>

namespace MixingStation::Firmware
{

class SimulatedProcessInputs final
{
public:
    void tick(std::uint16_t equipmentStatusMask);
    [[nodiscard]] ProcessInputs inputs() const;

private:
    ProcessInputs m_inputs;
};

} // namespace MixingStation::Firmware

#endif // SIMULATEDPROCESSINPUTS_H