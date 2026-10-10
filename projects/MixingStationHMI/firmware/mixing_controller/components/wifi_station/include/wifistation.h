#ifndef WIFISTATION_H
#define WIFISTATION_H

#include"esp_netif.h"

namespace MixingStation::Firmware
{

	[[nodiscard]] esp_netif_t *connectWifiStation();

} //namespace MixingStation::Firmware

#endif // WIFISTATION_H
