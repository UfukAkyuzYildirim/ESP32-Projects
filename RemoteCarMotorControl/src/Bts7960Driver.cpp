#include "Bts7960Driver.h"

#include "Config.h"

#include "driver/ledc.h"
#include "esp_err.h"

namespace {
void set_pwm(ledc_channel_t channel, uint32_t duty) {
    ledc_set_duty(config::kLedcSpeedMode, channel, duty);
    ledc_update_duty(config::kLedcSpeedMode, channel);
}
} // namespace

Bts7960Driver::Bts7960Driver(gpio_num_t rightPwmPin, gpio_num_t leftPwmPin, gpio_num_t rightEnablePin, gpio_num_t leftEnablePin)
    : rightPwmPin_(rightPwmPin)
    , leftPwmPin_(leftPwmPin)
    , rightEnablePin_(rightEnablePin)
    , leftEnablePin_(leftEnablePin) {}

void Bts7960Driver::init() {
    // 1) Configure GPIO for enable pins
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << rightEnablePin_) | (1ULL << leftEnablePin_),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Enable the driver outputs
    ESP_ERROR_CHECK(gpio_set_level(rightEnablePin_, 1));
    ESP_ERROR_CHECK(gpio_set_level(leftEnablePin_, 1));

    // 2) Configure LEDC timer
    ledc_timer_config_t pwm_timer = {
        .speed_mode = config::kLedcSpeedMode,
        .duty_resolution = config::kPwmResolution,
        .timer_num = config::kLedcTimer,
        .freq_hz = config::kPwmFreqHz,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&pwm_timer));

    // 3) Configure right PWM channel
    ledc_channel_config_t right_ch = {
        .gpio_num = rightPwmPin_,
        .speed_mode = config::kLedcSpeedMode,
        .channel = config::kRightPwmChannel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = config::kLedcTimer,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&right_ch));

    // 4) Configure left PWM channel
    ledc_channel_config_t left_ch = {
        .gpio_num = leftPwmPin_,
        .speed_mode = config::kLedcSpeedMode,
        .channel = config::kLeftPwmChannel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = config::kLedcTimer,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&left_ch));

    // Start in a safe state
    stop();
}

uint32_t Bts7960Driver::clampDuty(uint32_t duty) {
    const uint32_t maxDuty = (1u << static_cast<uint32_t>(config::kPwmResolution)) - 1u;
    return (duty > maxDuty) ? maxDuty : duty;
}

void Bts7960Driver::turnRight(uint32_t duty) {
    duty = clampDuty(duty);
    // Drive right PWM, left PWM off
    set_pwm(config::kRightPwmChannel, duty);
    set_pwm(config::kLeftPwmChannel, 0);
}

void Bts7960Driver::turnLeft(uint32_t duty) {
    duty = clampDuty(duty);
    // Drive left PWM, right PWM off
    set_pwm(config::kLeftPwmChannel, duty);
    set_pwm(config::kRightPwmChannel, 0);
}

void Bts7960Driver::stop() {
    // Coast stop: both PWM 0.
    set_pwm(config::kRightPwmChannel, 0);
    set_pwm(config::kLeftPwmChannel, 0);
}
