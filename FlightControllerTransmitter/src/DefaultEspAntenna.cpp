#include "DefaultEspAntenna.h"

volatile bool DefaultEspAntenna::hasData = false;
volatile uint8_t DefaultEspAntenna::lastByte = 0;
DefaultEspAntenna* DefaultEspAntenna::instance = nullptr;

DefaultEspAntenna::DefaultEspAntenna(const uint8_t peerMac[6], bool isSender)
    : sender(isSender) {
    memcpy(peer, peerMac, 6);
}

bool DefaultEspAntenna::begin() {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init FAILED");
        return false;
    }

    instance = this;

    if (sender) {
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, peer, 6);
        peerInfo.channel = 0;  
        peerInfo.encrypt = false;
        
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Peer eklenemedi");
            return false;
        }
    } else {
        esp_now_register_recv_cb(DefaultEspAntenna::onReceive);
    }

    Serial.print("ESP-NOW Basarili. MAC: ");
    Serial.println(WiFi.macAddress());
    return true;
}


bool DefaultEspAntenna::sendByte(uint8_t data) {
    return send(&data, sizeof(data));
}


bool DefaultEspAntenna::send(const void* data, size_t len) {
    if (!sender) return false;
    esp_err_t res = esp_now_send(peer, (uint8_t*)data, len);
    return res == ESP_OK;
}

bool DefaultEspAntenna::isAvailable() const {
    return hasData;
}

uint8_t DefaultEspAntenna::readByte() {
    hasData = false;
    return lastByte;
}

void DefaultEspAntenna::onReceive(const uint8_t* /*mac*/, const uint8_t* data, int len) {
    if (len <= 0) return;
    lastByte = data[0];
    hasData = true;
}