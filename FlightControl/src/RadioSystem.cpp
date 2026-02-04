#include "RadioSystem.h"

RadioSystem* RadioSystem::instance = nullptr;

RadioSystem::RadioSystem() {
    lastRecvTime = 0;
    currentData.x = 500;
    currentData.y = 0; 
    currentData.sw = 0;
}

bool RadioSystem::begin() {
    instance = this;
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) return false;
    esp_now_register_recv_cb(RadioSystem::onDataRecv);
    return true;
}

void RadioSystem::onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    if (instance) {
        memcpy(&instance->currentData, incomingData, sizeof(DronePacket));
        instance->lastRecvTime = millis();
    }
}

bool RadioSystem::isConnectionAlive() {
    return (millis() - lastRecvTime < CONNECTION_TIMEOUT);
}

int RadioSystem::getThrottle() {
    return map(currentData.y, 0, 1000, 1000, 1500);
}

int RadioSystem::getRoll() {
    return currentData.x;
}

int RadioSystem::getSwitch() {
    return currentData.sw;
}
int RadioSystem::getRawY() {
    return currentData.y;
}