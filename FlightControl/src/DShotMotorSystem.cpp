#include "DShotMotorSystem.h"

DShotMotorSystem::DShotMotorSystem() {
    armed = false;
    // Pin Configuration (FL, FR, RL, RR)
    channels[0] = {RMT_CHANNEL_0, static_cast<gpio_num_t>(PIN_FL)};
    channels[1] = {RMT_CHANNEL_1, static_cast<gpio_num_t>(PIN_FR)};
    channels[2] = {RMT_CHANNEL_2, static_cast<gpio_num_t>(PIN_RL)};
    channels[3] = {RMT_CHANNEL_3, static_cast<gpio_num_t>(PIN_RR)};
}

bool DShotMotorSystem::setupChannel(const ChannelCfg &cfg) {
    rmt_config_t config = {};
    config.rmt_mode = RMT_MODE_TX;
    config.channel = cfg.channel;
    config.gpio_num = cfg.pin;
    config.mem_block_num = 1;
    
    // ESP32-S3 için Clock Kaynağı ve Divider Ayarı
    // 80MHz APB Clock / 4 = 20MHz (1 tick = 50ns)
    config.clk_div = 4; 
    
    config.tx_config.loop_en = false;
    config.tx_config.carrier_en = false;
    config.tx_config.idle_output_en = true;
    config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;

    if (rmt_config(&config) != ESP_OK) return false;
    if (rmt_driver_install(cfg.channel, 0, 0) != ESP_OK) return false;
    return true;
}

bool DShotMotorSystem::begin() {
    bool ok = true;
    for (auto &c : channels) {
        ok = ok && setupChannel(c);
    }
    // Must send 0 initially (Arming sequence)
    disarm();
    delay(100); 
    return ok;
}

void DShotMotorSystem::disarm() {
    armed = false;
    // DShot Command 0 = Disarm / Motor Stop
    for (auto &c : channels) {
        sendDshotValue(c, 0);
    }
}

uint16_t DShotMotorSystem::usToDshot(int microseconds) const {
    // Map 1000-2000 range to DShot 48-2047
    // 0-47 are special commands (0=Stop, 1-47=Beep, etc.)
    // So for throttle, it must be at least 48.
    
    if (microseconds <= MIN_PULSE) return 0; // If below min, send STOP(0)

    int usClamped = constrain(microseconds, MIN_PULSE, MAX_PULSE);
    uint16_t v = map(usClamped, 1000, 2000, 48, 2047);
    return v;
}

void DShotMotorSystem::sendDshotValue(const ChannelCfg &cfg, uint16_t value) {
    // 1. Frame Preparation
    uint16_t packet = value;
    
    // Telemetry bit (Bit 4) - 0 for now
    packet = (packet << 1); 

    // 2. Checksum Calculation (XOR)
    // CRC = (Value ^ (Value >> 4) ^ (Value >> 8)) & 0x0F
    unsigned int csum = (packet ^ (packet >> 4) ^ (packet >> 8)) & 0x0F;
    
    // Append Checksum to the last 4 bits
    packet = (packet << 4) | csum;

    // 3. RMT Signal Generation (DShot300: 3.33us bit time)
    // 1 Tick = 50ns (20MHz clock)
    // Bit 1: High 2.2us (44 tick), Low 1.1us (22 tick)
    // Bit 0: High 1.1us (22 tick), Low 2.2us (44 tick)
    
    rmt_item32_t items[16]; // 16 bitlik paket
    
    for (int i = 0; i < 16; i++) {
        // En yüksek bitten (MSB) başla
        bool bit = (packet & (0x8000 >> i)) ? true : false;
        
        if (bit) { // "1" Gönder
            items[i] = {{{ 44, 1, 22, 0 }}}; // High 44, Low 22
        } else {   // "0" Gönder
            items[i] = {{{ 22, 1, 44, 0 }}}; // High 22, Low 44
        }
    }

    rmt_write_items(cfg.channel, items, 16, true);
}

void DShotMotorSystem::writeAllUs(int microseconds) {
    // If the signal is low (specifically < 1020us), force send 0 (STOP/Disarm).
    // This prevents the ESC from entering "Throttle High" protection mode or spinning accidentally.
    if (microseconds < 1020) {
        disarm();
    } else {
        uint16_t val = usToDshot(microseconds);
        for (auto &c : channels) {
            sendDshotValue(c, val);
        }
        armed = true;
    }
}

void DShotMotorSystem::writeMotor(int index, int microseconds) {
    if (index < 0 || index > 3) return;
    
    // Güvenlik sınırları
    if (microseconds < 1000) microseconds = 1000;
    if (microseconds > 2000) microseconds = 2000;
    
    // Eğer motorlar kilitliyse (Disarm) ve gaz kapalıysa 0 yolla
    if (!armed && microseconds < 1050) { 
        sendDshotValue(channels[index], 0);
        return;
    }
    
    uint16_t val = usToDshot(microseconds);
    sendDshotValue(channels[index], val);
}