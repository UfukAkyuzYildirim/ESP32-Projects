#include "WifiManager.h"

WifiManager::WifiManager(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
}

void WifiManager::begin() {
    Serial.println();
    Serial.print("WiFi Baglaniliyor: ");
    Serial.println(_ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi Baglandi!");
        Serial.print("IP Adresi: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("");
        Serial.println("WiFi Baglantisi Basarisiz!");
    }
}

void WifiManager::loop() {
    // Ileride kopma durumunda reconnect mantigi eklenebilir
    if (WiFi.status() != WL_CONNECTED) {
        // Basit bir reconnect
        // WiFi.reconnect();
    }
}

bool WifiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WifiManager::getIp() {
    return WiFi.localIP().toString();
}
