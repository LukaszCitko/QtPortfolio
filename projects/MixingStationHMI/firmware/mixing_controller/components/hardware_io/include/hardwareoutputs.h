#ifndef HARDWAREOUTPUTS_H
#define HARDWAREOUTPUTS_H

#include "esp_err.h"

#include <cstdint>

namespace MixingStation::Firmware
{

class HardwareOutputs final
{
public:
    [[nodiscard]] esp_err_t initialize();

    [[nodiscard]] esp_err_t applyEquipmentStatus(
        std::uint16_t equipmentStatusMask);

private:
    bool m_initialized{false};
};

} // namespace MixingStation::Firmware

#endif // HARDWAREOUTPUTS_H

