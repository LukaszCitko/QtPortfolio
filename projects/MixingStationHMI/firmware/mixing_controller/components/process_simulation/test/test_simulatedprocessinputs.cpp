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
