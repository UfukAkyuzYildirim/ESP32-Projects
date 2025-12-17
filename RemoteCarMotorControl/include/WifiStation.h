#pragma once

namespace wifi_station {

// Connects ESP32 to a Wi-Fi network in STA mode.
// Uses credentials from config::kWifiSsid / config::kWifiPass.
void init();

} // namespace wifi_station
