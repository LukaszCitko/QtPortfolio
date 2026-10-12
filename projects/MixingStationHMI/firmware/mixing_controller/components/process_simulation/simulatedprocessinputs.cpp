#include "simulatedprocessinputs.h"

#include "controllerprotocol.h"
#include "processconfiguration.h"

#include <algorithm>

namespace
{

constexpr std::uint16_t Pump1RunningMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump1Running);
constexpr std::uint16_t Pump2RunningMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump2Running);
constexpr std::uint16_t Pump3RunningMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump3Running);
constexpr std::uint16_t DrainValveOpenMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::DrainValveOpen);

void addSimulatedVolume(MixingStation::Firmware::ProcessInputs &inputs, std::uint32_t &volumeRemainder, std::uint16_t maximumFlowDecilitresPerSecond, std::uint16_t actualRpm, std::uint32_t elapsedMilliseconds, std::uint16_t targetVolumeDecilitres)
{
    if (actualRpm == 0U || inputs.mixtureVolumeDecilitres >= targetVolumeDecilitres)
    {
        volumeRemainder = 0;
        return;
    }

    const std::uint64_t scaledVolume = volumeRemainder + static_cast<std::uint64_t>(maximumFlowDecilitresPerSecond) * actualRpm * elapsedMilliseconds;
    const std::uint64_t scalingDivisor = static_cast<std::uint64_t>(ControllerProtocol::MaximumPumpRpm) * 1000U;
    const std::uint32_t calculatedIncrease = static_cast<std::uint32_t>(scaledVolume / scalingDivisor);

    volumeRemainder = static_cast<std::uint32_t>(scaledVolume % scalingDivisor);

    const std::uint32_t remainingVolume = targetVolumeDecilitres - inputs.mixtureVolumeDecilitres;
    const std::uint32_t acceptedIncrease = std::min(calculatedIncrease, remainingVolume);

    inputs.mixtureVolumeDecilitres = static_cast<std::uint16_t>(inputs.mixtureVolumeDecilitres + acceptedIncrease);

    if (inputs.mixtureVolumeDecilitres >= targetVolumeDecilitres)
    {
        volumeRemainder = 0;
    }
}

void removeSimulatedVolume(MixingStation::Firmware::ProcessInputs &inputs, std::uint32_t &volumeRemainder, std::uint16_t maximumFlowDecilitresPerSecond, std::uint16_t actualRpm, std::uint32_t elapsedMilliseconds)
{
    if (actualRpm == 0U || inputs.mixtureVolumeDecilitres == 0U)
    {
        volumeRemainder = 0;
        return;
    }

    const std::uint64_t scaledVolume = volumeRemainder + static_cast<std::uint64_t>(maximumFlowDecilitresPerSecond) * actualRpm * elapsedMilliseconds;
    const std::uint64_t scalingDivisor = static_cast<std::uint64_t>(ControllerProtocol::MaximumPumpRpm) * 1000U;
    const std::uint32_t calculatedDecrease = static_cast<std::uint32_t>(scaledVolume / scalingDivisor);

    volumeRemainder = static_cast<std::uint32_t>(scaledVolume % scalingDivisor);

    const std::uint32_t acceptedDecrease = std::min(calculatedDecrease, static_cast<std::uint32_t>(inputs.mixtureVolumeDecilitres));

    inputs.mixtureVolumeDecilitres = static_cast<std::uint16_t>(inputs.mixtureVolumeDecilitres - acceptedDecrease);

    if (inputs.mixtureVolumeDecilitres == 0U)
    {
        volumeRemainder = 0;
    }
}

} // namespace

namespace MixingStation::Firmware
{

void SimulatedProcessInputs::tick(const ControllerSnapshot &snapshot, std::uint32_t elapsedMilliseconds)
{
    if ((snapshot.equipmentStatusMask & Pump1RunningMask) != 0U)
    {
        addSimulatedVolume(m_inputs, m_waterVolumeRemainder, ProcessConfiguration::WaterFlowAtMaximumRpmDecilitresPerSecond, snapshot.pump1ActualRpm, elapsedMilliseconds, ProcessConfiguration::WaterTargetDecilitres);
        return;
    }

    if ((snapshot.equipmentStatusMask & Pump2RunningMask) != 0U)
    {
        addSimulatedVolume(m_inputs, m_concentrateVolumeRemainder, ProcessConfiguration::ConcentrateFlowAtMaximumRpmDecilitresPerSecond, snapshot.pump2ActualRpm, elapsedMilliseconds, ProcessConfiguration::MixtureTargetDecilitres);
        return;
    }

    if ((snapshot.equipmentStatusMask & Pump3RunningMask) != 0U)
    {
        removeSimulatedVolume(m_inputs, m_outflowVolumeRemainder, ProcessConfiguration::ProductFlowAtMaximumRpmDecilitresPerSecond, snapshot.pump3ActualRpm, elapsedMilliseconds);
        return;
    }

    if ((snapshot.equipmentStatusMask & DrainValveOpenMask) != 0U)
    {
        removeSimulatedVolume(m_inputs, m_outflowVolumeRemainder, ProcessConfiguration::DrainFlowDecilitresPerSecond, ControllerProtocol::MaximumPumpRpm, elapsedMilliseconds);
    }
}

ProcessInputs SimulatedProcessInputs::inputs() const
{
    return m_inputs;
}

} // namespace MixingStation::Firmware