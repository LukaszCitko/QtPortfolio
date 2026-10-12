#ifndef HARDWAREINPUTS_H
#define HARDWAREINPUTS_H

#include "esp_err.h"

#include <cstdint>

namespace MixingStation::Firmware
{

class HardwareInputs final
{
public:
    [[nodiscard]] esp_err_t initialize();
    void tick(std::uint32_t elapsedMilliseconds);

    [[nodiscard]] bool mixerConnected() const;

private:
    bool m_initialized{false};
    bool m_lastRawButtonPressed{false};
    bool m_stableButtonPressed{false};
    bool m_mixerConnected{false};
    std::uint32_t m_stableTimeMilliseconds{0};
};

} // namespace MixingStation::Firmware

#endif // HARDWAREINPUTS_H