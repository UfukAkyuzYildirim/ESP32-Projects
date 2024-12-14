#include "MotorController.h"
#include "driver/ledc.h"

MotorController::MotorController(gpio_num_t right_pwm, gpio_num_t left_pwm, gpio_num_t right_enable, gpio_num_t left_enable)
    : _right_pwm(right_pwm), _left_pwm(left_pwm), _right_enable(right_enable), _left_enable(left_enable) {}

void MotorController::init() {
    // GPIO pinlerini yapılandır
    gpio_config_t output_conf = {
        .pin_bit_mask = (1ULL << _right_enable) | (1ULL << _left_enable),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&output_conf);

    // PWM zamanlayıcısını yapılandır
    ledc_timer_config_t pwm_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RES,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK, // Eklenen alan
    };
    ledc_timer_config(&pwm_timer);

    // Sağ PWM kanalını yapılandır
    _r_pwm_channel = {
        .gpio_num = _right_pwm,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
        .flags = {} // Varsayılan olarak boş bırakılır
    };
    ledc_channel_config(&_r_pwm_channel);

    // Sol PWM kanalını yapılandır
    _l_pwm_channel = {
        .gpio_num = _left_pwm,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
        .flags = {} // Varsayılan olarak boş bırakılır
    };
    ledc_channel_config(&_l_pwm_channel);

    // Motor enable pinlerini aktif hale getir
    gpio_set_level(_right_enable, 1);
    gpio_set_level(_left_enable, 1);
}

void MotorController::turnRight(int duty) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}

void MotorController::turnLeft(int duty) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void MotorController::stop() {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}