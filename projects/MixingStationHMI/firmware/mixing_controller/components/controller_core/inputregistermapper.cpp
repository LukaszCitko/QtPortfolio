#include "inputregistermapper.h"

namespace MixingStation::Firmware
{

InputRegisterBank makeInputRegisterBank(
    const ControllerSnapshot &snapshot)
{
    InputRegisterBank registers{};

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::ProtocolVersion)] =
        ControllerProtocol::ProtocolVersion;

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::ControllerHeartbeat)] =
        snapshot.heartbeat;

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::ProcessState)] =
        static_cast<std::uint16_t>(snapshot.processState);

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::StageProgressPermille)] =
        snapshot.stageProgressPermille;

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::MixtureVolumeDecilitres)] =
        snapshot.mixtureVolumeDecilitres;

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::TemperatureTenthsCelsius)] =
        snapshot.temperatureTenthsCelsius;

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::Pump1ActualRpm)] =
        snapshot.pump1ActualRpm;

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::Pump2ActualRpm)] =
        snapshot.pump2ActualRpm;

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::Pump3ActualRpm)] =
        snapshot.pump3ActualRpm;

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::MixerActualRpm)] =
        snapshot.mixerActualRpm;

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::EquipmentStatusMask)] =
        snapshot.equipmentStatusMask;

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::FaultMask)] =
        snapshot.faultMask;

    registers[ControllerProtocol::address(
        ControllerProtocol::InputRegister::LastHandledCommandSequence)] =
        snapshot.lastHandledCommandSequence;

    return registers;
}

} // namespace MixingStation::Firmware
