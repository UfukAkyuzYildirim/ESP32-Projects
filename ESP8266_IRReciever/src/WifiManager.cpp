#include "WifiManager.h"

#include <LittleFS.h>
#include <ArduinoJson.h>

static const char* kWifiConfigPath = "/wifi.json";

WifiManager::WifiManager()
    : _fsReady(false),
      _setupApActive(false) {}

bool WifiManager::loadCredentials() {
    if (!_fsReady) {
        _fsReady = LittleFS.begin();
        if (!_fsReady) {
            Serial.println("LittleFS mount basarisiz! WiFi config okunamadi.");
            return false;
        }
    }

    if (!LittleFS.exists(kWifiConfigPath)) {
        return false;
    }

    File f = LittleFS.open(kWifiConfigPath, "r");
    if (!f) {
        Serial.println("WiFi config dosyasi acilamadi.");
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) {
        Serial.println("WiFi config JSON parse hatasi.");
        return false;
    }

    if (!doc["ssid"].is<const char*>() || !doc["pass"].is<const char*>()) {
        return false;
    }

    _ssid = String((const char*)doc["ssid"]);
    _password = String((const char*)doc["pass"]);

    _ssid.trim();
    return !_ssid.isEmpty();
}

bool WifiManager::saveCredentials(const String& ssid, const String& password) {
    if (!_fsReady) {
        _fsReady = LittleFS.begin();
        if (!_fsReady) {
            Serial.println("LittleFS mount basarisiz! WiFi config yazilamadi.");
            return false;
        }
    }

    String cleanSsid = ssid;
    cleanSsid.trim();
    if (cleanSsid.isEmpty()) {
        return false;
    }

    JsonDocument doc;
    doc["ssid"] = cleanSsid;
    doc["pass"] = password;

    File f = LittleFS.open(kWifiConfigPath, "w");
    if (!f) {
        Serial.println("WiFi config dosyasi yazmak icin acilamadi.");
        return false;
    }

    if (serializeJson(doc, f) == 0) {
        f.close();
        Serial.println("WiFi config dosyasina yazma basarisiz.");
        return false;
    }
    f.close();

    _ssid = cleanSsid;
    _password = password;
    return true;
}

void WifiManager::connectSta() {
    _setupApActive = false;
    _setupApSsid = "";

    Serial.println();
    Serial.print("WiFi Baglaniliyor: ");
    Serial.println(_ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid.c_str(), _password.c_str());

    const unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < WIFI_CONNECT_TIMEOUT_MS) {
        delay(250);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi Baglandi!");
        Serial.print("IP Adresi: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println();
        Serial.println("WiFi Baglantisi Basarisiz!");
    }
}

void WifiManager::startSetupAp() {
    _setupApActive = true;

    String chip = String(ESP.getChipId(), HEX);
    chip.toUpperCase();
    _setupApSsid = String(SETUP_AP_SSID_PREFIX) + chip;

    WiFi.mode(WIFI_AP);

    const char* pass = SETUP_AP_PASSWORD;
    bool ok = false;
    if (pass && String(pass).length() >= 8) {
        ok = WiFi.softAP(_setupApSsid.c_str(), pass);
    } else {
        ok = WiFi.softAP(_setupApSsid.c_str());
    }

    Serial.println();
    if (ok) {
        Serial.println("Setup AP acildi.");
        Serial.print("AP SSID: ");
        Serial.println(_setupApSsid);
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("Setup AP acilamadi!");
    }
}

void WifiManager::begin() {
    // LittleFS mount + config oku
    _fsReady = LittleFS.begin();
    if (!_fsReady) {
        Serial.println("LittleFS mount basarisiz! Setup AP ile devam edilecek.");
        startSetupAp();
        return;
    }

    if (loadCredentials()) {
        connectSta();
        if (!isConnected()) {
            startSetupAp();
        }
    } else {
        Serial.println("WiFi config bulunamadi. Setup AP aciliyor...");
        startSetupAp();
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
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    if (_setupApActive) {
        return WiFi.softAPIP().toString();
    }
    return String("0.0.0.0");
}

bool WifiManager::isSetupApActive() const {
    return _setupApActive;
}

String WifiManager::getSetupApSsid() const {
    return _setupApSsid;
}
