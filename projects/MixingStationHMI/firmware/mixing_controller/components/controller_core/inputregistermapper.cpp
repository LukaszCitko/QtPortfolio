#include "inputregistermapper.h"

namespace MixingStation::Firmware
{

InputRegisterBank makeInputRegisterBank(
    const ControllerSnapshot &snapshot)
{
    InputRegisterBank registers{};

    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::ProtocolVersion)] = ControllerProtocol::ProtocolVersion;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::ControllerHeartbeat)] =  snapshot.heartbeat;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::ProcessState)] = static_cast<std::uint16_t>(snapshot.processState);
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::StageProgressPermille)] = snapshot.stageProgressPermille;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::ControllerSessionId)] = snapshot.controllerSessionId;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::RunSequence)] = snapshot.runSequence;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::RunKind)] = static_cast<std::uint16_t>(snapshot.runKind);
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::MixtureVolumeDecilitres)] = snapshot.mixtureVolumeDecilitres;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::TemperatureTenthsCelsius)] = snapshot.temperatureTenthsCelsius;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::WaterVolumeDecilitres)] = snapshot.waterVolumeDecilitres;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::ConcentrateVolumeDecilitres)] = snapshot.concentrateVolumeDecilitres;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::Pump1ActualRpm)] = snapshot.pump1ActualRpm;
    registers[ControllerProtocol::address( ControllerProtocol::InputRegister::Pump2ActualRpm)] = snapshot.pump2ActualRpm;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::Pump3ActualRpm)] = snapshot.pump3ActualRpm;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::MixerActualRpm)] = snapshot.mixerActualRpm;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::EquipmentStatusMask)] = snapshot.equipmentStatusMask;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::FaultMask)] = snapshot.faultMask;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::LastHandledCommandSequence)] = snapshot.lastHandledCommandSequence;
    registers[ControllerProtocol::address(ControllerProtocol::InputRegister::LastCommandResult)] = static_cast<std::uint16_t>(snapshot.lastCommandResult);

    return registers;
}

} // namespace MixingStation::Firmware
