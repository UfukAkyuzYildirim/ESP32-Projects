#include "RadioSystem.h"

// Static instance'ı başlat
RadioSystem* RadioSystem::instance = nullptr;

RadioSystem::RadioSystem() {
    lastRecvTime = 0;
    // Varsayılan Güvenli Başlangıç
    currentData.x = 0;
    currentData.y = 0;
    currentData.sw = 0;
}

bool RadioSystem::begin() {
    instance = this; // Callback içinden erişebilmek için
    
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        return false; // Başlatılamadı
    }
    
    esp_now_register_recv_cb(RadioSystem::onDataRecv);
    return true;
}

void RadioSystem::onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    // Veri boyutu doğruysa struct içine kopyala
    if (instance && len == sizeof(DronePacket)) {
        memcpy(&instance->currentData, incomingData, sizeof(DronePacket));
        instance->lastRecvTime = millis();
    }
}

bool RadioSystem::isConnectionAlive() {
    // 1 saniye (1000ms) boyunca veri gelmezse bağlantı kopmuş say
    return (millis() - lastRecvTime < 1000);
}

// --- YENİ FONKSİYONLAR ---

int RadioSystem::getLiftSpeed() {
    // Bağlantı koptuysa 0 döndür (Risk alma)
    if (!isConnectionAlive()) return 0;
    return currentData.y;
}

int RadioSystem::getRollRate() {
    if (!isConnectionAlive()) return 0;
    return currentData.x;
}

bool RadioSystem::isSwitchOn() {
    if (!isConnectionAlive()) return false; // Koptuysa kapat
    return (currentData.sw == 1);
}

int RadioSystem::getX() {
    if (!isConnectionAlive()) return 0;
    return currentData.x; 
}

int RadioSystem::getY() {
    if (!isConnectionAlive()) return 0;
    return currentData.y; 
}

// --- ESKİ FONKSİYONLAR (Uyumluluk İçin - İçi Boş veya Yönlendirilmiş) ---
int RadioSystem::getThrottle() { return map(currentData.y, -500, 500, 0, 1000); } // Kabaca çevirir
int RadioSystem::getRoll() { return map(currentData.x, -500, 500, 0, 1000); }
int RadioSystem::getSwitch() { return currentData.sw; }
int RadioSystem::getRawY() { return currentData.y; }