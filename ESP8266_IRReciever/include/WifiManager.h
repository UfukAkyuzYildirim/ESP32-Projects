#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <ESP8266WiFi.h>
#include <Arduino.h>
#include "Config.h"

class WifiManager {
public:
    WifiManager();
    void begin();
    void loop(); // Yeniden baglanma mantigi icin gerekirse
    bool isConnected();
    String getIp();
    bool isSetupApActive() const;
    String getSetupApSsid() const;

    // WiFi bilgilerini flash'a kaydeder. Basarili olursa true.
    bool saveCredentials(const String& ssid, const String& password);

private:
    bool _fsReady;
    bool _setupApActive;
    String _ssid;
    String _password;
    String _setupApSsid;

    bool loadCredentials();
    void connectSta();
    void startSetupAp();
};

#endif
