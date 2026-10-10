#include "modbusserver.h"

#include "esp_modbus_slave.h"

namespace MixingStation::Firmware
{

ModbusServer::ModbusServer()
{
    m_holdingRegisters.fill(0);
    m_inputRegisters.fill(0);
}

ModbusServer::~ModbusServer()
{
    (void)stop();
}

esp_err_t ModbusServer::start(esp_netif_t *networkInterface)
{
    if (networkInterface == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    if (m_handle != nullptr) {
        return ESP_ERR_INVALID_STATE;
    }

    mb_communication_info_t communicationConfiguration{};

    communicationConfiguration.tcp_opts.port =
        ControllerProtocol::TcpPort;
    communicationConfiguration.tcp_opts.mode = MB_TCP;
    communicationConfiguration.tcp_opts.addr_type = MB_IPV4;
    communicationConfiguration.tcp_opts.ip_addr_table = nullptr;
    communicationConfiguration.tcp_opts.ip_netif_ptr = networkInterface;
    communicationConfiguration.tcp_opts.uid =
        static_cast<std::uint8_t>(ControllerProtocol::ServerAddress);

    esp_err_t result = mbc_slave_create_tcp(
        &communicationConfiguration,
        &m_handle);

    if (result != ESP_OK) {
        m_handle = nullptr;
        return result;
    }

    mb_register_area_descriptor_t inputArea{};
    inputArea.type = MB_PARAM_INPUT;
    inputArea.start_offset = 0;
    inputArea.address = m_inputRegisters.data();
    inputArea.size = sizeof(m_inputRegisters);
    inputArea.access = MB_ACCESS_RO;

    result = mbc_slave_set_descriptor(m_handle, inputArea);

    if (result != ESP_OK) {
        mbc_slave_delete(m_handle);
        m_handle = nullptr;
        return result;
    }

    mb_register_area_descriptor_t holdingArea{};
    holdingArea.type = MB_PARAM_HOLDING;
    holdingArea.start_offset = 0;
    holdingArea.address = m_holdingRegisters.data();
    holdingArea.size = sizeof(m_holdingRegisters);
    holdingArea.access = MB_ACCESS_RW;

    result = mbc_slave_set_descriptor(m_handle, holdingArea);

    if (result != ESP_OK) {
        mbc_slave_delete(m_handle);
        m_handle = nullptr;
        return result;
    }

    result = mbc_slave_start(m_handle);

    if (result != ESP_OK) {
        mbc_slave_delete(m_handle);
        m_handle = nullptr;
    }

    return result;
}

esp_err_t ModbusServer::stop()
{
    if (m_handle == nullptr) {
        return ESP_OK;
    }

    const esp_err_t stopResult = mbc_slave_stop(m_handle);
    const esp_err_t deleteResult = mbc_slave_delete(m_handle);

    m_handle = nullptr;

    if (stopResult != ESP_OK) {
        return stopResult;
    }

    return deleteResult;
}

void ModbusServer::publishSnapshot(const ControllerSnapshot &snapshot)
{
    const InputRegisterBank newRegisters =
        makeInputRegisterBank(snapshot);

    if (m_handle == nullptr) {
        m_inputRegisters = newRegisters;
        return;
    }

    if (mbc_slave_lock(m_handle) == ESP_OK) {
        m_inputRegisters = newRegisters;
        mbc_slave_unlock(m_handle);
    }
}

std::optional<CommandRequest> ModbusServer::takeCommandRequest()
{
    if (m_handle == nullptr) {
        return std::nullopt;
    }

    if (mbc_slave_lock(m_handle) != ESP_OK) {
        return std::nullopt;
    }

    const std::uint16_t rawCommand =
        m_holdingRegisters[
            ControllerProtocol::address(
                ControllerProtocol::HoldingRegister::CommandCode)];

    const std::uint16_t sequence =
        m_holdingRegisters[
            ControllerProtocol::address(
                ControllerProtocol::HoldingRegister::CommandSequence)];

    mbc_slave_unlock(m_handle);

    const auto command =
        static_cast<ControllerProtocol::Command>(rawCommand);

    if (command == ControllerProtocol::Command::None) {
        return std::nullopt;
    }

    if (sequence == m_lastTakenCommandSequence) {
        return std::nullopt;
    }

    m_lastTakenCommandSequence = sequence;

    return CommandRequest{
        command,
        sequence
    };
}

const InputRegisterBank &ModbusServer::inputRegisters() const
{
    return m_inputRegisters;
}

const HoldingRegisterBank &ModbusServer::holdingRegisters() const
{
    return m_holdingRegisters;
}

bool ModbusServer::isRunning() const
{
    return m_handle != nullptr;
}

} // namespace MixingStation::Firmware