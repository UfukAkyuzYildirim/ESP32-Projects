#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <ESP8266WiFi.h>
#include "Config.h"

class WifiManager {
public:
    WifiManager(const char* ssid, const char* password);
    void begin();
    void loop(); // Yeniden baglanma mantigi icin gerekirse
    bool isConnected();
    String getIp();

private:
    const char* _ssid;
    const char* _password;
};

#endif
