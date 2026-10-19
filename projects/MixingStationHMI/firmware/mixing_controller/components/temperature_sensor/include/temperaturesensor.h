#ifndef TEMPERATURESENSOR_H
#define TEMPERATURESENSOR_H

#include "ds18b20.h"
#include "esp_err.h"
#include "onewire_bus.h"

#include <cstdint>

namespace MixingStation::Firmware
{

class TemperatureSensor final
{
public:
    [[nodiscard]] esp_err_t initialize();
    [[nodiscard]] esp_err_t reinitialize();
    [[nodiscard]] esp_err_t readTemperature(float &temperatureCelsius);

private:
    onewire_bus_handle_t m_bus{nullptr};
    ds18b20_device_handle_t m_device{nullptr};
    bool m_initialized{false};
};

} // namespace MixingStation::Firmware

#endif // TEMPERATURESENSOR_H