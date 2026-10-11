#include "simulatedprocessinputs.h"

#include "controllerprotocol.h"

#include <algorithm>
#include <limits>

namespace
{

constexpr std::uint16_t Pump1RunningMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump1Running);

} // namespace

namespace MixingStation::Firmware
{

void SimulatedProcessInputs::tick(const ControllerSnapshot &snapshot, std::uint32_t elapsedMilliseconds)
{
    if ((snapshot.equipmentStatusMask & Pump1RunningMask) == 0U || snapshot.pump1ActualRpm == 0U)
    {
        return;
    }

    const std::uint32_t scaledVolume = m_volumeRemainder + static_cast<std::uint32_t>(ProcessConfiguration::WaterFlowAtMaximumRpmDecilitresPerSecond) * snapshot.pump1ActualRpm * elapsedMilliseconds;

    const std::uint32_t scalingDivisor = static_cast<std::uint32_t>(ControllerProtocol::MaximumPumpRpm) * 1000U;
    const std::uint32_t volumeIncrease = scaledVolume / scalingDivisor;

    m_volumeRemainder = scaledVolume % scalingDivisor;

    const std::uint32_t nextVolume = static_cast<std::uint32_t>(m_inputs.mixtureVolumeDecilitres) + volumeIncrease;
    const std::uint32_t maximumVolume = std::numeric_limits<std::uint16_t>::max();

    m_inputs.mixtureVolumeDecilitres = static_cast<std::uint16_t>(std::min(nextVolume, maximumVolume));
}

ProcessInputs SimulatedProcessInputs::inputs() const
{
    return m_inputs;
}

} // namespace MixingStation::Firmware