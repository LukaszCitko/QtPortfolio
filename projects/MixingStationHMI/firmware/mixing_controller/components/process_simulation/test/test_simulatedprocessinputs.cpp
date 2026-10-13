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

    TEST_ASSERT_EQUAL_UINT16(99, inputs.mixtureVolumeDecilitres);
    TEST_ASSERT_EQUAL_UINT16(99, inputs.waterVolumeDecilitres);
    TEST_ASSERT_EQUAL_UINT16(0, inputs.concentrateVolumeDecilitres);
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

    TEST_ASSERT_EQUAL_UINT16(766, simulator.inputs().mixtureVolumeDecilitres);
    TEST_ASSERT_EQUAL_UINT16(ProcessConfiguration::WaterTargetDecilitres, simulator.inputs().waterVolumeDecilitres);
    TEST_ASSERT_EQUAL_UINT16(66, simulator.inputs().concentrateVolumeDecilitres);

    for (std::uint16_t cycle = 0; cycle < 100U && simulator.inputs().mixtureVolumeDecilitres < ProcessConfiguration::MixtureTargetDecilitres; ++cycle)
    {
        simulator.tick(snapshot, ProcessConfiguration::SimulationStepMilliseconds);
    }

    TEST_ASSERT_EQUAL_UINT16(ProcessConfiguration::MixtureTargetDecilitres, simulator.inputs().mixtureVolumeDecilitres);
    TEST_ASSERT_EQUAL_UINT16(ProcessConfiguration::WaterTargetDecilitres, simulator.inputs().waterVolumeDecilitres);
    TEST_ASSERT_EQUAL_UINT16(ProcessConfiguration::ConcentrateTargetDecilitres, simulator.inputs().concentrateVolumeDecilitres);
}

TEST_CASE("Complete simulated batch reaches ready, transfers and returns to idle", "[process_simulation]")
{
    MixingStation::Firmware::ControllerCore controller;
    MixingStation::Firmware::SimulatedProcessInputs simulator;

    MixingStation::Firmware::ProcessInputs processInputs = simulator.inputs();
    processInputs.mixerConnected = true;
    processInputs.temperatureTenthsCelsius = 330;

    controller.updateInputs(processInputs);

    const ControllerProtocol::CommandResult startResult = controller.handleCommand(ControllerProtocol::Command::StartBatch, 1);

    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::Accepted), static_cast<std::uint16_t>(startResult));

    for (std::uint16_t cycle = 0; cycle < 400U && controller.snapshot().processState != ControllerProtocol::ProcessState::ReadyForTransfer; ++cycle)
    {
        simulator.tick(controller.snapshot(), ProcessConfiguration::SimulationStepMilliseconds);

        processInputs = simulator.inputs();
        processInputs.mixerConnected = true;
        processInputs.temperatureTenthsCelsius = 330;

        controller.updateInputs(processInputs);
        controller.tick(ProcessConfiguration::SimulationStepMilliseconds);
    }

    MixingStation::Firmware::ControllerSnapshot snapshot = controller.snapshot();
    const std::uint16_t mixerRunningMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::MixerRunning);

    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::ProcessState::ReadyForTransfer), static_cast<std::uint16_t>(snapshot.processState));
    TEST_ASSERT_EQUAL_UINT16(ProcessConfiguration::MixtureTargetDecilitres, snapshot.mixtureVolumeDecilitres);
    TEST_ASSERT_EQUAL_UINT16(ProcessConfiguration::DosingMixerRpm, snapshot.mixerActualRpm);
    TEST_ASSERT_NOT_EQUAL(0, snapshot.equipmentStatusMask & mixerRunningMask);
    TEST_ASSERT_EQUAL_UINT16(ProcessConfiguration::WaterTargetDecilitres, snapshot.waterVolumeDecilitres);
    TEST_ASSERT_EQUAL_UINT16(ProcessConfiguration::ConcentrateTargetDecilitres, snapshot.concentrateVolumeDecilitres);

    const ControllerProtocol::CommandResult transferResult = controller.handleCommand(ControllerProtocol::Command::StartTransfer, 2);

    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::Accepted), static_cast<std::uint16_t>(transferResult));

    for (std::uint16_t cycle = 0; cycle < 200U && controller.snapshot().processState != ControllerProtocol::ProcessState::Complete; ++cycle)
    {
        simulator.tick(controller.snapshot(), ProcessConfiguration::SimulationStepMilliseconds);

        processInputs = simulator.inputs();
        processInputs.mixerConnected = true;
        processInputs.temperatureTenthsCelsius = 330;

        controller.updateInputs(processInputs);
        controller.tick(ProcessConfiguration::SimulationStepMilliseconds);
    }

    snapshot = controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::ProcessState::Complete), static_cast<std::uint16_t>(snapshot.processState));
    TEST_ASSERT_EQUAL_UINT16(0, snapshot.mixtureVolumeDecilitres);
    TEST_ASSERT_EQUAL_UINT16(0, snapshot.pump3ActualRpm);
    TEST_ASSERT_EQUAL_UINT16(0, snapshot.mixerActualRpm);
    TEST_ASSERT_EQUAL_UINT16(1000, snapshot.stageProgressPermille);

    const ControllerProtocol::CommandResult nextBatchResult = controller.handleCommand(ControllerProtocol::Command::PrepareNextBatch, 3);

    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::Accepted), static_cast<std::uint16_t>(nextBatchResult));
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::ProcessState::Idle), static_cast<std::uint16_t>(controller.snapshot().processState));
    TEST_ASSERT_EQUAL_UINT16(0, controller.snapshot().stageProgressPermille);
}
