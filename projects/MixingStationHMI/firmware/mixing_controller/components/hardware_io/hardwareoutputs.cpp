#include "hardwareoutputs.h"
#include "controllerprotocol.h"
#include "driver/gpio.h"

namespace
{

constexpr gpio_num_t Pump1OutputPin = GPIO_NUM_1;

constexpr std::uint16_t Pump1RunningMask =
    static_cast<std::uint16_t>(
        ControllerProtocol::EquipmentStatusBit::
            Pump1Running);

} // namespace

namespace MixingStation::Firmware
{

esp_err_t HardwareOutputs::initialize()
{
    gpio_config_t configuration{};

    configuration.pin_bit_mask =
        1ULL << static_cast<unsigned>(
            Pump1OutputPin);

    configuration.mode = GPIO_MODE_OUTPUT;
    configuration.pull_up_en =
        GPIO_PULLUP_DISABLE;
    configuration.pull_down_en =
        GPIO_PULLDOWN_DISABLE;
    configuration.intr_type =
        GPIO_INTR_DISABLE;

    const esp_err_t configurationResult =
        gpio_config(&configuration);

    if (configurationResult != ESP_OK)
    {
        return configurationResult;
    }

    const esp_err_t initialLevelResult =
        gpio_set_level(Pump1OutputPin, 0);

    if (initialLevelResult != ESP_OK)
    {
        return initialLevelResult;
    }

    m_initialized = true;
    return ESP_OK;
}

esp_err_t HardwareOutputs::applyEquipmentStatus(
    std::uint16_t equipmentStatusMask)
{
    if (!m_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    const bool pump1Running =
        (equipmentStatusMask & Pump1RunningMask)
        != 0U;

    return gpio_set_level(
        Pump1OutputPin,
        pump1Running ? 1 : 0);
}

} // namespace MixingStation::Firmware

