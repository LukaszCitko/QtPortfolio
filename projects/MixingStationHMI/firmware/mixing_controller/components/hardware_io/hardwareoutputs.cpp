#include "hardwareoutputs.h"
#include "controllerprotocol.h"
#include "driver/gpio.h"

namespace
{

constexpr gpio_num_t Pump1OutputPin = GPIO_NUM_1;
constexpr gpio_num_t Pump2OutputPin = GPIO_NUM_2;
constexpr gpio_num_t Pump3OutputPin = GPIO_NUM_42;
constexpr gpio_num_t MixerOutputPin = GPIO_NUM_41;

constexpr gpio_num_t FaultPump1OutputPin = GPIO_NUM_6;
constexpr gpio_num_t FaultPump2OutputPin = GPIO_NUM_7;
constexpr gpio_num_t FaultPump3OutputPin = GPIO_NUM_15;
constexpr gpio_num_t FaultMixerOutputPin = GPIO_NUM_5;

constexpr std::uint64_t OutputPinMask =
                          (1ULL << static_cast<unsigned>(Pump1OutputPin))
                        | (1ULL << static_cast<unsigned>(Pump2OutputPin))
                        | (1ULL << static_cast<unsigned>(Pump3OutputPin))
                        | (1ULL << static_cast<unsigned>(MixerOutputPin))
                        | (1ULL << static_cast<unsigned>(FaultPump1OutputPin))
                        | (1ULL << static_cast<unsigned>(FaultPump2OutputPin))
                        | (1ULL << static_cast<unsigned>(FaultPump3OutputPin))
                        | (1ULL << static_cast<unsigned>(FaultMixerOutputPin));

constexpr std::uint16_t Pump1RunningMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump1Running);
constexpr std::uint16_t Pump2RunningMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump2Running);
constexpr std::uint16_t Pump3RunningMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::Pump3Running);
constexpr std::uint16_t MixerRunningMask = static_cast<std::uint16_t>(ControllerProtocol::EquipmentStatusBit::MixerRunning);

constexpr std::uint16_t Pump1FaultMask = static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Pump1);
constexpr std::uint16_t Pump2FaultMask = static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Pump2);
constexpr std::uint16_t Pump3FaultMask = static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Pump3);
constexpr std::uint16_t MixerFaultMask = static_cast<std::uint16_t>(ControllerProtocol::FaultBit::Mixer);

} // namespace

namespace MixingStation::Firmware
{

esp_err_t HardwareOutputs::initialize()
{
    gpio_config_t configuration{};

    configuration.pin_bit_mask = OutputPinMask;

    configuration.mode = GPIO_MODE_OUTPUT;
    configuration.pull_up_en = GPIO_PULLUP_DISABLE;
    configuration.pull_down_en = GPIO_PULLDOWN_DISABLE;
    configuration.intr_type = GPIO_INTR_DISABLE;

    const esp_err_t configurationResult = gpio_config(&configuration);

    if (configurationResult != ESP_OK)
    {
        return configurationResult;
    }

    const esp_err_t pump1InitialLevelResult = gpio_set_level(Pump1OutputPin, 0);

    if (pump1InitialLevelResult != ESP_OK)
    {
        return pump1InitialLevelResult;
    }

    const esp_err_t pump2InitialLevelResult = gpio_set_level(Pump2OutputPin, 0);

    if (pump2InitialLevelResult != ESP_OK)
    {
        return pump2InitialLevelResult;
    }

    const esp_err_t pump3InitialLevelResult = gpio_set_level(Pump3OutputPin, 0);

    if (pump3InitialLevelResult != ESP_OK)
    {
        return pump3InitialLevelResult;
    }

    const esp_err_t mixerInitialLevelResult = gpio_set_level(MixerOutputPin, 0);

    if (mixerInitialLevelResult != ESP_OK)
    {
        return mixerInitialLevelResult;
    }

    const esp_err_t pump1FaultInitialLevelResult = gpio_set_level(FaultPump1OutputPin, 0);

    if (pump1FaultInitialLevelResult != ESP_OK)
    {
        return pump1FaultInitialLevelResult;
    }

    const esp_err_t pump2FaultInitialLevelResult = gpio_set_level(FaultPump2OutputPin, 0);

    if (pump2FaultInitialLevelResult != ESP_OK)
    {
        return pump2FaultInitialLevelResult;
    }

    const esp_err_t pump3FaultInitialLevelResult = gpio_set_level(FaultPump3OutputPin, 0);

    if (pump3FaultInitialLevelResult != ESP_OK)
    {
        return pump3FaultInitialLevelResult;
    }

    const esp_err_t mixerFaultInitialLevelResult = gpio_set_level(FaultMixerOutputPin, 0);

    if (mixerFaultInitialLevelResult != ESP_OK)
    {
        return mixerFaultInitialLevelResult;
    }
    m_initialized = true;
    return ESP_OK;
}

esp_err_t HardwareOutputs::applyEquipmentStatus(std::uint16_t equipmentStatusMask)
{
    if (!m_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    const bool pump1Running = (equipmentStatusMask & Pump1RunningMask) != 0U;
    const bool pump2Running = (equipmentStatusMask & Pump2RunningMask) != 0U;
    const bool pump3Running = (equipmentStatusMask & Pump3RunningMask) != 0U;
    const bool mixerRunning = (equipmentStatusMask & MixerRunningMask) != 0U;

    const esp_err_t pump1Result = gpio_set_level(Pump1OutputPin, pump1Running ? 1 : 0);

    if (pump1Result != ESP_OK)
    {
        return pump1Result;
    }

    const esp_err_t pump2Result = gpio_set_level(Pump2OutputPin, pump2Running ? 1 : 0);

    if (pump2Result != ESP_OK)
    {
        return pump2Result;
    }

    const esp_err_t pump3Result = gpio_set_level(Pump3OutputPin, pump3Running ? 1 : 0);

    if (pump3Result != ESP_OK)
    {
        return pump3Result;
    }

    return gpio_set_level(MixerOutputPin, mixerRunning ? 1 : 0);
}

esp_err_t HardwareOutputs::applyFaultStatus(std::uint16_t faultMask)
{
    if (!m_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    const bool pump1Fault = (faultMask & Pump1FaultMask) != 0U;
    const bool pump2Fault = (faultMask & Pump2FaultMask) != 0U;
    const bool pump3Fault = (faultMask & Pump3FaultMask) != 0U;
    const bool mixerFault = (faultMask & MixerFaultMask) != 0U;

    const esp_err_t pump1Result = gpio_set_level(FaultPump1OutputPin, pump1Fault ? 1 : 0);

    if (pump1Result != ESP_OK)
    {
        return pump1Result;
    }

    const esp_err_t pump2Result = gpio_set_level(FaultPump2OutputPin, pump2Fault ? 1 : 0);

    if (pump2Result != ESP_OK)
    {
        return pump2Result;
    }

    const esp_err_t pump3Result = gpio_set_level(FaultPump3OutputPin, pump3Fault ? 1 : 0);

    if (pump3Result != ESP_OK)
    {
        return pump3Result;
    }

    return gpio_set_level(FaultMixerOutputPin, mixerFault ? 1 : 0);
}

} // namespace MixingStation::Firmware

