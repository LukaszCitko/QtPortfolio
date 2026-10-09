#include "controllercore.h"

namespace MixingStation::Firmware
{

ControllerSnapshot ControllerCore::snapshot() const
{
    return m_snapshot;
}

void ControllerCore::tick()
{
    ++m_snapshot.heartbeat;
}

} // namespace MixingStation::Firmware
