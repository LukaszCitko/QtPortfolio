#include "controllerprotocol.h"
#include "processconfiguration.h"
#include "simulatedprocessinputs.h"
#include "unity.h"

TEST_CASE("Water simulation follows pump RPM and elapsed time", "[process_simulation]")
{
    MixingStation::Firmware::SimulatedProcessInputs simulator;
    MixingStation::Firmware::ControllerSnapshot snapshot;

    snapshot.equipmentStatusMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump1Running);
    snapshot.pump1ActualRpm = ProcessConfiguration::Pump1DefaultRpm;

    constexpr std::uint16_t SimulationCyclesPerSecond = 1000U / ProcessConfiguration::SimulationStepMilliseconds;

    for (std::uint16_t cycle = 0; cycle < SimulationCyclesPerSecond; ++cycle)
    {
        simulator.tick(snapshot, ProcessConfiguration::SimulationStepMilliseconds);
    }

    const MixingStation::Firmware::ProcessInputs inputs = simulator.inputs();

    TEST_ASSERT_EQUAL_UINT16(120, inputs.mixtureVolumeDecilitres);
}

TEST_CASE("Concentrate simulation follows pump RPM and stops at mixture target", "[process_simulation]")
{
    MixingStation::Firmware::SimulatedProcessInputs simulator;
    MixingStation::Firmware::ControllerSnapshot snapshot;

    snapshot.equipmentStatusMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump1Running);
    snapshot.pump1ActualRpm = ProcessConfiguration::Pump1DefaultRpm;

    for (std::uint16_t cycle = 0; cycle < 100U && simulator.inputs().mixtureVolumeDecilitres < ProcessConfiguration::WaterTargetDecilitres; ++cycle)
    {
        simulator.tick(snapshot, ProcessConfiguration::SimulationStepMilliseconds);
    }

    TEST_ASSERT_EQUAL_UINT16(ProcessConfiguration::WaterTargetDecilitres, simulator.inputs().mixtureVolumeDecilitres);

    snapshot.equipmentStatusMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump2Running);
    snapshot.pump1ActualRpm = 0;
    snapshot.pump2ActualRpm = ProcessConfiguration::Pump2DefaultRpm;

    constexpr std::uint16_t SimulationCyclesPerSecond = 1000U / ProcessConfiguration::SimulationStepMilliseconds;

    for (std::uint16_t cycle = 0; cycle < SimulationCyclesPerSecond; ++cycle)
    {
        simulator.tick(snapshot, ProcessConfiguration::SimulationStepMilliseconds);
    }

    TEST_ASSERT_EQUAL_UINT16(780, simulator.inputs().mixtureVolumeDecilitres);

    for (std::uint16_t cycle = 0; cycle < 100U && simulator.inputs().mixtureVolumeDecilitres < ProcessConfiguration::MixtureTargetDecilitres; ++cycle)
    {
        simulator.tick(snapshot, ProcessConfiguration::SimulationStepMilliseconds);
    }

    TEST_ASSERT_EQUAL_UINT16(ProcessConfiguration::MixtureTargetDecilitres, simulator.inputs().mixtureVolumeDecilitres);
}
