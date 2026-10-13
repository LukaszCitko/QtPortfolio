#include "controllerprotocol.h"
#include "inputregistermapper.h"

#include "unity.h"

TEST_CASE("Controller snapshot is mapped to input registers", "[input_register_mapper]")
{
    MixingStation::Firmware::ControllerSnapshot snapshot;

    snapshot.heartbeat = 42;
    snapshot.controllerSessionId = 18472;
    snapshot.runSequence = 7;
    snapshot.runKind = ControllerProtocol::RunKind::Batch;
    snapshot.processState = ControllerProtocol::ProcessState::Mixing;
    snapshot.stageProgressPermille = 725;
    snapshot.mixtureVolumeDecilitres = 876;
    snapshot.temperatureTenthsCelsius = 603;

    snapshot.pump1ActualRpm = 310;
    snapshot.pump2ActualRpm = 220;
    snapshot.pump3ActualRpm = 450;
    snapshot.mixerActualRpm = 800;

    snapshot.equipmentStatusMask = 0x0055;
    snapshot.faultMask = 0x0080;
    snapshot.lastHandledCommandSequence = 27;
    snapshot.lastCommandResult = ControllerProtocol::CommandResult::Accepted;

    const MixingStation::Firmware::InputRegisterBank registers =
        MixingStation::Firmware::makeInputRegisterBank(snapshot);

    TEST_ASSERT_EQUAL_UINT16(ControllerProtocol::ProtocolVersion, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::ProtocolVersion)]);
    TEST_ASSERT_EQUAL_UINT16(42, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::ControllerHeartbeat)]);
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::ProcessState::Mixing), registers[ControllerProtocol::address(ControllerProtocol::InputRegister::ProcessState)]);
    TEST_ASSERT_EQUAL_UINT16(725, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::StageProgressPermille)]);
    TEST_ASSERT_EQUAL_UINT16(18472, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::ControllerSessionId)]);
    TEST_ASSERT_EQUAL_UINT16(7, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::RunSequence)]);
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::RunKind::Batch), registers[ControllerProtocol::address(ControllerProtocol::InputRegister::RunKind)]);
    TEST_ASSERT_EQUAL_UINT16(876, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::MixtureVolumeDecilitres)]);
    TEST_ASSERT_EQUAL_UINT16(603, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::TemperatureTenthsCelsius)]);
    TEST_ASSERT_EQUAL_UINT16(310, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::Pump1ActualRpm)]);
    TEST_ASSERT_EQUAL_UINT16(220, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::Pump2ActualRpm)]);
    TEST_ASSERT_EQUAL_UINT16(450, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::Pump3ActualRpm)]);
    TEST_ASSERT_EQUAL_UINT16(800, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::MixerActualRpm)]);
    TEST_ASSERT_EQUAL_HEX16(0x0055, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::EquipmentStatusMask)]);
    TEST_ASSERT_EQUAL_HEX16(0x0080, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::FaultMask)]);
    TEST_ASSERT_EQUAL_UINT16(27, registers[ControllerProtocol::address(ControllerProtocol::InputRegister::LastHandledCommandSequence)]);
    TEST_ASSERT_EQUAL_UINT16(static_cast<std::uint16_t>(ControllerProtocol::CommandResult::Accepted), registers[ControllerProtocol::address(ControllerProtocol::InputRegister::LastCommandResult)]);
}
