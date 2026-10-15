#ifndef CONTROLLERPROTOCOL_H
#define CONTROLLERPROTOCOL_H

#include <cstdint>

namespace ControllerProtocol {

// Development server uses a non-privileged TCP port.
inline constexpr std::uint16_t TcpPort = 1502;
inline constexpr std::uint8_t ServerAddress = 1;
inline constexpr std::uint16_t ProtocolVersion = 5;
inline constexpr std::uint16_t RegisterCount = 64;
inline constexpr std::uint16_t MaximumPumpRpm = 1000;

// Commands sent from the HMI to the process controller.
enum class Command : std::uint16_t
{
    None = 0,
    StartBatch = 1,
    PauseBatch = 2,
    ResumeBatch = 3,
    StartTransfer = 4,
    ResetFaults = 5,
    StartDrain = 6,
    PrepareNextBatch = 7
};

// Results reported after processing an HMI command.
enum class CommandResult : std::uint16_t
{
    None = 0,
    Accepted = 1,
    DuplicateSequence = 2,
    InvalidState = 3,
    UnsupportedCommand = 4
};

// Process states reported by the controller.
enum class ProcessState : std::uint16_t
{
    Idle = 0,
    FillingWater = 1,
    DosingConcentrate = 2,
    TemperatureCheck = 3,
    Mixing = 4,
    ReadyForTransfer = 5,
    Transferring = 6,
    Draining = 7,
    Paused = 8,
    Complete = 9
};

// Identifies the type of process associated with the current run sequence.
enum class RunKind : std::uint16_t
{
    None = 0,
    Batch = 1,
    Drain = 2
};

// Holding registers are written by the HMI.
enum class HoldingRegister : std::uint16_t
{
    CommandCode = 0,
    CommandSequence = 1,

    Pump1TargetRpm = 10,
    Pump2TargetRpm = 11,
    Pump3TargetRpm = 12,
    MixerTargetRpm = 13,
    SimulatedFaultMask = 20
};

// Input registers are read by the HMI.
enum class InputRegister : std::uint16_t
{
    ProtocolVersion = 0,
    ControllerHeartbeat = 1,
    ProcessState = 2,
    StageProgressPermille = 3,

    ControllerSessionId = 4,
    RunSequence = 5,
    RunKind = 6,

    MixtureVolumeDecilitres = 10,
    TemperatureTenthsCelsius = 11,
    WaterVolumeDecilitres = 12,
    ConcentrateVolumeDecilitres = 13,

    Pump1ActualRpm = 20,
    Pump2ActualRpm = 21,
    Pump3ActualRpm = 22,
    MixerActualRpm = 23,

    EquipmentStatusMask = 30,
    FaultMask = 31,
    LastHandledCommandSequence = 32,
    LastCommandResult = 33
};

// Individual bits stored in EquipmentStatusMask.
enum class EquipmentStatusBit : std::uint16_t
{
    Pump1Running = 1U << 0,
    Valve1Open = 1U << 1,
    Pump2Running = 1U << 2,
    Valve2Open = 1U << 3,
    Pump3Running = 1U << 4,
    Valve3Open = 1U << 5,
    MixerRunning = 1U << 6,
    MixerConnected = 1U << 7,
    DrainValveOpen = 1U << 8,
    TemperatureSensorValid = 1U << 9
};

// Individual bits stored in FaultMask.
enum class FaultBit : std::uint16_t
{
    Pump1 = 1U << 0,
    Valve1 = 1U << 1,
    Pump2 = 1U << 2,
    Valve2 = 1U << 3,
    Pump3 = 1U << 4,
    Valve3 = 1U << 5,
    Mixer = 1U << 6,
    TemperatureHigh = 1U << 7,
    TemperatureSensor = 1U << 8
};

inline constexpr std::uint16_t DefinedFaultMask = static_cast<std::uint16_t>(FaultBit::Pump1)
                                                | static_cast<std::uint16_t>(FaultBit::Valve1)
                                                | static_cast<std::uint16_t>(FaultBit::Pump2)
                                                | static_cast<std::uint16_t>(FaultBit::Valve2)
                                                | static_cast<std::uint16_t>(FaultBit::Pump3)
                                                | static_cast<std::uint16_t>(FaultBit::Valve3)
                                                | static_cast<std::uint16_t>(FaultBit::Mixer)
                                                | static_cast<std::uint16_t>(FaultBit::TemperatureHigh)
                                                | static_cast<std::uint16_t>(FaultBit::TemperatureSensor);

constexpr std::uint16_t address(HoldingRegister registerId)
{
    return static_cast<std::uint16_t>(registerId);
}

constexpr std::uint16_t address(InputRegister registerId)
{
    return static_cast<std::uint16_t>(registerId);
}

} // namespace ControllerProtocol

#endif // CONTROLLERPROTOCOL_H
