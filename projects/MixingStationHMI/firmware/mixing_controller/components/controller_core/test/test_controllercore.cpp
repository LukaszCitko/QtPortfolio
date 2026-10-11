#include "controllercore.h"
#include "unity.h"

TEST_CASE("Controller starts in idle state", "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;

    const MixingStation::Firmware::ControllerSnapshot snapshot = controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::ProcessState::Idle), static_cast<std::uint16_t>(snapshot.processState));
    TEST_ASSERT_EQUAL_UINT16(0,snapshot.heartbeat);
    TEST_ASSERT_EQUAL_UINT16(0,snapshot.lastHandledCommandSequence);
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::None), static_cast<std::uint16_t>(snapshot.lastCommandResult));
}

TEST_CASE("Controller tick increments heartbeat", "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;

    controller.tick();
    controller.tick();

    const auto snapshot = controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(2, snapshot.heartbeat);
}
TEST_CASE( "Process inputs are included in controller snapshot", "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;

    MixingStation::Firmware::ProcessInputs inputs;
    inputs.mixtureVolumeDecilitres = 425;
    inputs.temperatureTenthsCelsius = 603;
    inputs.faultMask = static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Pump1);

    controller.updateInputs(inputs);
    const auto snapshot = controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(425, snapshot.mixtureVolumeDecilitres);
    TEST_ASSERT_EQUAL_UINT16(603, snapshot.temperatureTenthsCelsius);
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Pump1), snapshot.faultMask);
}
TEST_CASE( "Start batch command changes controller state", "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;
    auto setpoints = controller.setpoints();
    controller.updateSetpoints(setpoints);

    const ControllerProtocol::CommandResult result = controller.handleCommand(ControllerProtocol::Command::StartBatch, 1);
    const auto snapshot = controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(725, snapshot.pump1ActualRpm);
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::Accepted), static_cast<std::uint16_t>(result));
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::ProcessState::FillingWater), static_cast<std::uint16_t>(snapshot.processState));
    TEST_ASSERT_EQUAL_UINT16(1, snapshot.lastHandledCommandSequence);
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::Accepted), static_cast<std::uint16_t>(snapshot.lastCommandResult));
}

TEST_CASE("Start batch is rejected outside idle state", "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;

    const ControllerProtocol::CommandResult initialResult = controller.handleCommand(ControllerProtocol::Command::StartBatch, 1);

TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::Accepted), static_cast<std::uint16_t>(initialResult));

    const ControllerProtocol::CommandResult result = controller.handleCommand(ControllerProtocol::Command::StartBatch, 2);
    const auto snapshot = controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::InvalidState), static_cast<std::uint16_t>(result));
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::ProcessState::FillingWater), static_cast<std::uint16_t>(snapshot.processState));
    TEST_ASSERT_EQUAL_UINT16(2, snapshot.lastHandledCommandSequence);
}

TEST_CASE("Duplicate command sequence is rejected", "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;
    const ControllerProtocol::CommandResult initialResult = controller.handleCommand(ControllerProtocol::Command::StartBatch, 10);

    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::Accepted), static_cast<std::uint16_t>(initialResult));

    const ControllerProtocol::CommandResult result = controller.handleCommand(ControllerProtocol::Command::PauseBatch, 10);
    const auto snapshot = controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::DuplicateSequence), static_cast<std::uint16_t>(result));
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::ProcessState::FillingWater), static_cast<std::uint16_t>(snapshot.processState));
    TEST_ASSERT_EQUAL_UINT16(10, snapshot.lastHandledCommandSequence);
}

TEST_CASE("Water filling finishes after reaching 70 litres", "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;

    const auto startResult = controller.handleCommand(ControllerProtocol::Command::StartBatch, 1);
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::Accepted), static_cast<std::uint16_t>(startResult));

    MixingStation::Firmware::ProcessInputs inputs;
    inputs.mixtureVolumeDecilitres = 700;
    controller.updateInputs(inputs);
    controller.tick();

    const auto snapshot = controller.snapshot();
    const auto pump1Mask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump1Running);
    const auto valve1Mask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Valve1Open);

    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::ProcessState::DosingConcentrate), static_cast<std::uint16_t>(snapshot.processState));
    TEST_ASSERT_EQUAL_UINT16(0, snapshot.pump1ActualRpm);
    TEST_ASSERT_EQUAL_UINT16(0, snapshot.equipmentStatusMask & pump1Mask);
    TEST_ASSERT_EQUAL_UINT16(0, snapshot.equipmentStatusMask & valve1Mask);

}

TEST_CASE("Pump setpoints are limited to maximum RPM", "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;

    MixingStation::Firmware::ControllerSetpoints setpoints;
    setpoints.pump1TargetRpm = 1200;
    setpoints.pump2TargetRpm = 800;
    setpoints.pump3TargetRpm = 65535;

    controller.updateSetpoints(setpoints);

    const auto acceptedSetpoints = controller.setpoints();

    TEST_ASSERT_EQUAL_UINT16(ControllerProtocol::MaximumPumpRpm, acceptedSetpoints.pump1TargetRpm);
    TEST_ASSERT_EQUAL_UINT16(800, acceptedSetpoints.pump2TargetRpm);
    TEST_ASSERT_EQUAL_UINT16(ControllerProtocol::MaximumPumpRpm, acceptedSetpoints.pump3TargetRpm);
}

