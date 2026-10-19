#include "temperaturesensor.h"

namespace
{

constexpr int TemperatureSensorPin = 4;
constexpr std::uint32_t MaximumReceiveBytes = 10;

} // namespace

namespace MixingStation::Firmware
{

esp_err_t TemperatureSensor::initialize()
{
    if (m_initialized)
    {
        return ESP_OK;
    }

    onewire_bus_config_t busConfiguration{};

    busConfiguration.bus_gpio_num = TemperatureSensorPin;
    busConfiguration.flags.en_pull_up = false;

    onewire_bus_rmt_config_t rmtConfiguration{};

    rmtConfiguration.max_rx_bytes = MaximumReceiveBytes;

    const esp_err_t busResult = onewire_new_bus_rmt(&busConfiguration, &rmtConfiguration, &m_bus);

    if (busResult != ESP_OK)
    {
        return busResult;
    }

    ds18b20_config_t sensorConfiguration{};

    const esp_err_t deviceResult = ds18b20_new_device_from_bus(m_bus, &sensorConfiguration, &m_device);

    if (deviceResult != ESP_OK)
    {
        onewire_bus_del(m_bus);
        m_bus = nullptr;
        return deviceResult;
    }

    m_initialized = true;
    return ESP_OK;
}

esp_err_t TemperatureSensor::reinitialize()
{
    m_initialized = false;

    if (m_device != nullptr)
    {
        const esp_err_t deviceResult = ds18b20_del_device(m_device);

        if (deviceResult != ESP_OK)
        {
            return deviceResult;
        }
        m_device = nullptr;
    }

    if (m_bus != nullptr)
    {
        const esp_err_t busResult = onewire_bus_del(m_bus);

        if (busResult != ESP_OK)
        {
            return busResult;
        }
        m_bus = nullptr;
    }

    return initialize();
}

esp_err_t TemperatureSensor::readTemperature(float &temperatureCelsius)
{
    if (!m_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    const esp_err_t conversionResult = ds18b20_trigger_temperature_conversion(m_device);

    if (conversionResult != ESP_OK)
    {
        return conversionResult;
    }

    return ds18b20_get_temperature(m_device, &temperatureCelsius);
}


} // namespace MixingStation::Firmware