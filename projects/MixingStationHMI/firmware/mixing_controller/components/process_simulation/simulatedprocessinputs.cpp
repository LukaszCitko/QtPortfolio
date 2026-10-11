#include "simulatedprocessinputs.h"

#include "controllerprotocol.h"

#include <algorithm>
#include <limits>

namespace
{

constexpr std::uint16_t VolumeIncreasePerCycleDecilitres = 50;
constexpr std::uint16_t Pump1RunningMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump1Running);

} // namespace

namespace MixingStation::Firmware
{

void SimulatedProcessInputs::tick(std::uint16_t equipmentStatusMask)
{
    if ((equipmentStatusMask & Pump1RunningMask) == 0U)
    {
        return;
    }

    const std::uint32_t nextVolume = static_cast<std::uint32_t>(m_inputs.mixtureVolumeDecilitres) + VolumeIncreasePerCycleDecilitres;
    const std::uint32_t maximumVolume = std::numeric_limits<std::uint16_t>::max();

    m_inputs.mixtureVolumeDecilitres = static_cast<std::uint16_t>(std::min(nextVolume, maximumVolume));
}

ProcessInputs SimulatedProcessInputs::inputs() const
{
    return m_inputs;
}

} // namespace MixingStation::Firmware