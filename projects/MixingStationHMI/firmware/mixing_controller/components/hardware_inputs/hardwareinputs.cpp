#include "hardwareinputs.h"

#include "driver/gpio.h"

namespace
{

constexpr gpio_num_t MixerConnectionButtonPin = GPIO_NUM_40;
constexpr std::uint32_t ButtonDebounceMilliseconds = 50;

} // namespace

namespace MixingStation::Firmware
{

esp_err_t HardwareInputs::initialize()
{
    gpio_config_t configuration{};

    configuration.pin_bit_mask = 1ULL << static_cast<unsigned>(MixerConnectionButtonPin);
    configuration.mode = GPIO_MODE_INPUT;
    configuration.pull_up_en = GPIO_PULLUP_ENABLE;
    configuration.pull_down_en = GPIO_PULLDOWN_DISABLE;
    configuration.intr_type = GPIO_INTR_DISABLE;

    const esp_err_t configurationResult = gpio_config(&configuration);

    if (configurationResult != ESP_OK)
    {
        return configurationResult;
    }

    const bool initialButtonPressed = gpio_get_level(MixerConnectionButtonPin) == 0;

    m_lastRawButtonPressed = initialButtonPressed;
    m_stableButtonPressed = initialButtonPressed;
    m_stableTimeMilliseconds = 0;
    m_mixerConnected = false;
    m_initialized = true;

    return ESP_OK;
}

void HardwareInputs::tick(std::uint32_t elapsedMilliseconds)
{
    if (!m_initialized)
    {
        return;
    }

    const bool rawButtonPressed = gpio_get_level(MixerConnectionButtonPin) == 0;

    if (rawButtonPressed != m_lastRawButtonPressed)
    {
        m_lastRawButtonPressed = rawButtonPressed;
        m_stableTimeMilliseconds = 0;
        return;
    }

    if (m_stableTimeMilliseconds < ButtonDebounceMilliseconds)
    {
        m_stableTimeMilliseconds += elapsedMilliseconds;

        if (m_stableTimeMilliseconds < ButtonDebounceMilliseconds)
        {
            return;
        }
    }

    if (rawButtonPressed == m_stableButtonPressed)
    {
        return;
    }

    m_stableButtonPressed = rawButtonPressed;

    if (m_stableButtonPressed)
    {
        m_mixerConnected = !m_mixerConnected;
    }
}

bool HardwareInputs::mixerConnected() const
{
    return m_mixerConnected;
}

} // namespace MixingStation::Firmware