#ifndef INPUTREGISTERMAPPER_H
#define INPUTREGISTERMAPPER_H

#include "controllercore.h"
#include "controllerprotocol.h"

#include <array>
#include <cstdint>

namespace MixingStation::Firmware
{

using InputRegisterBank = std::array<std::uint16_t, ControllerProtocol::RegisterCount>;

[[nodiscard]] InputRegisterBank makeInputRegisterBank(
    const ControllerSnapshot &snapshot);

} // namespace MixingStation::Firmware

#endif // INPUTREGISTERMAPPER_H
