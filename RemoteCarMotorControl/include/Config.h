#pragma once

#include <cstdint>

#include "driver/gpio.h"
#include "driver/ledc.h"

namespace config {

// On-board LED (most ESP32 DevKit boards: GPIO2). Change if your board differs.
inline constexpr gpio_num_t kStatusLedPin = GPIO_NUM_2;

// BTS7960B wiring (same as existing projects in this repo)
inline constexpr gpio_num_t kRightEnablePin = GPIO_NUM_25; // R_EN
inline constexpr gpio_num_t kRightPwmPin    = GPIO_NUM_26; // R_PWM
inline constexpr gpio_num_t kLeftEnablePin  = GPIO_NUM_27; // L_EN
inline constexpr gpio_num_t kLeftPwmPin     = GPIO_NUM_14; // L_PWM

// PWM configuration
inline constexpr uint32_t kPwmFreqHz = 5000;
inline constexpr ledc_timer_bit_t kPwmResolution = LEDC_TIMER_8_BIT; // 0..255

// LEDC mapping
inline constexpr ledc_mode_t kLedcSpeedMode = LEDC_LOW_SPEED_MODE;
inline constexpr ledc_timer_t kLedcTimer = LEDC_TIMER_0;
inline constexpr ledc_channel_t kRightPwmChannel = LEDC_CHANNEL_0;
inline constexpr ledc_channel_t kLeftPwmChannel  = LEDC_CHANNEL_1;

// Wi-Fi credentials (edit these for your network)
inline constexpr char kWifiSsid[] = "";
inline constexpr char kWifiPass[] = "";

// Optional static IP (leave disabled unless you know your LAN subnet)
inline constexpr bool kUseStaticIp = true;

// Example for a typical home LAN: 192.168.1.0/24
inline constexpr uint8_t kStaticIp[4]      = {192, 168, 1, 201};
inline constexpr uint8_t kStaticGateway[4] = {192, 168, 1, 1};
inline constexpr uint8_t kStaticNetmask[4] = {255, 255, 255, 0};
inline constexpr uint8_t kStaticDns[4]     = {192, 168, 1, 1};

} // namespace config
