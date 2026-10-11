#ifndef MODBUSSERVER_H
#define MODBUSSERVER_H

#include "controllerprotocol.h"
#include "inputregistermapper.h"

#include "esp_err.h"
#include "esp_netif.h"

#include <array>
#include <cstdint>
#include <optional>

namespace MixingStation::Firmware
{

using HoldingRegisterBank =  std::array<std::uint16_t, ControllerProtocol::RegisterCount>;

struct CommandRequest
{
    ControllerProtocol::Command command{ControllerProtocol::Command::None};

    std::uint16_t sequence{0};
};

class ModbusServer
{
public:
    ModbusServer();
    ~ModbusServer();

    ModbusServer(const ModbusServer &) = delete;
    ModbusServer &operator=(const ModbusServer &) = delete;

    [[nodiscard]] esp_err_t start(esp_netif_t *networkInterface);
    [[nodiscard]] esp_err_t stop();

    void publishSnapshot(const ControllerSnapshot &snapshot);

    [[nodiscard]] const InputRegisterBank &inputRegisters() const;
    [[nodiscard]] const HoldingRegisterBank &holdingRegisters() const;
    [[nodiscard]] bool isRunning() const;

    [[nodiscard]] std::optional<CommandRequest> takeCommandRequest();
    [[nodiscard]] std::optional<ControllerSetpoints> readSetpoints();

private:
    void *m_handle{nullptr};

    InputRegisterBank m_inputRegisters{};
    HoldingRegisterBank m_holdingRegisters{};

    std::uint16_t m_lastTakenCommandSequence{0};
};

} // namespace MixingStation::Firmware

#endif // MODBUSSERVER_H