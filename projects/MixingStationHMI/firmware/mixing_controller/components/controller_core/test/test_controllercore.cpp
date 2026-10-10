#include "controllercore.h"

#include "unity.h"

TEST_CASE(
    "Controller starts in idle state",
    "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;

    const MixingStation::Firmware::ControllerSnapshot snapshot =
        controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(
        static_cast<std::uint16_t>(
            ControllerProtocol::ProcessState::Idle),
        static_cast<std::uint16_t>(
            snapshot.processState));

    TEST_ASSERT_EQUAL_UINT16(
        0,
        snapshot.heartbeat);

    TEST_ASSERT_EQUAL_UINT16(
        0,
        snapshot.lastHandledCommandSequence);

    TEST_ASSERT_EQUAL_UINT16(
        static_cast<std::uint16_t>(
            ControllerProtocol::CommandResult::None),
        static_cast<std::uint16_t>(
            snapshot.lastCommandResult));
}

TEST_CASE(
    "Controller tick increments heartbeat",
    "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;

    controller.tick();
    controller.tick();

    const auto snapshot = controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(
        2,
        snapshot.heartbeat);
}

TEST_CASE(
    "Start batch command changes controller state",
    "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;

    const ControllerProtocol::CommandResult result =
        controller.handleCommand(
            ControllerProtocol::Command::StartBatch,
            1);

    const auto snapshot = controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(
        static_cast<std::uint16_t>(
            ControllerProtocol::CommandResult::Accepted),
        static_cast<std::uint16_t>(result));

    TEST_ASSERT_EQUAL_UINT16(
        static_cast<std::uint16_t>(
            ControllerProtocol::ProcessState::FillingWater),
        static_cast<std::uint16_t>(
            snapshot.processState));

    TEST_ASSERT_EQUAL_UINT16(
        1,
        snapshot.lastHandledCommandSequence);

    TEST_ASSERT_EQUAL_UINT16(
        static_cast<std::uint16_t>(
            ControllerProtocol::CommandResult::Accepted),
        static_cast<std::uint16_t>(
            snapshot.lastCommandResult));
}

TEST_CASE(
    "Start batch is rejected outside idle state",
    "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;

    const ControllerProtocol::CommandResult initialResult =
    controller.handleCommand(
        ControllerProtocol::Command::StartBatch,
        1);

TEST_ASSERT_EQUAL_UINT16(
    static_cast<std::uint16_t>(
        ControllerProtocol::CommandResult::Accepted),
    static_cast<std::uint16_t>(initialResult));


    const ControllerProtocol::CommandResult result =
        controller.handleCommand(
            ControllerProtocol::Command::StartBatch,
            2);

    const auto snapshot = controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(
        static_cast<std::uint16_t>(
            ControllerProtocol::CommandResult::InvalidState),
        static_cast<std::uint16_t>(result));

    TEST_ASSERT_EQUAL_UINT16(
        static_cast<std::uint16_t>(
            ControllerProtocol::ProcessState::FillingWater),
        static_cast<std::uint16_t>(
            snapshot.processState));

    TEST_ASSERT_EQUAL_UINT16(
        2,
        snapshot.lastHandledCommandSequence);
}

TEST_CASE(
    "Duplicate command sequence is rejected",
    "[controller_core]")
{
    MixingStation::Firmware::ControllerCore controller;

    const ControllerProtocol::CommandResult initialResult =
    controller.handleCommand(
        ControllerProtocol::Command::StartBatch,
        10);

TEST_ASSERT_EQUAL_UINT16(
    static_cast<std::uint16_t>(
        ControllerProtocol::CommandResult::Accepted),
    static_cast<std::uint16_t>(initialResult));


    const ControllerProtocol::CommandResult result =
        controller.handleCommand(
            ControllerProtocol::Command::PauseBatch,
            10);

    const auto snapshot = controller.snapshot();

    TEST_ASSERT_EQUAL_UINT16(
        static_cast<std::uint16_t>(
            ControllerProtocol::CommandResult::DuplicateSequence),
        static_cast<std::uint16_t>(result));

    TEST_ASSERT_EQUAL_UINT16(
        static_cast<std::uint16_t>(
            ControllerProtocol::ProcessState::FillingWater),
        static_cast<std::uint16_t>(
            snapshot.processState));

    TEST_ASSERT_EQUAL_UINT16(
        10,
        snapshot.lastHandledCommandSequence);
}

