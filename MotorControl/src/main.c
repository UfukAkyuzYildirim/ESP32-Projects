#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#define R_EN GPIO_NUM_25
#define R_PWM GPIO_NUM_26
#define L_EN GPIO_NUM_27
#define L_PWM GPIO_NUM_14
#define R_IS GPIO_NUM_4
#define L_IS GPIO_NUM_5
#define RightTouch GPIO_NUM_33 // Sağ dokunmatik tuş
#define LeftTouch GPIO_NUM_32  // Sol dokunmatik tuş

// PWM frekansı ve çözünürlüğü
#define PWM_FREQ 5000                    // 5 kHz
#define PWM_RES LEDC_TIMER_8_BIT         // 8-bit çözünürlük (0-255)
#define PWM_DUTY_25 (255 * 25 / 100)     // %25 duty cycle

// PWM kanalları
#define R_PWM_CHANNEL LEDC_CHANNEL_0
#define L_PWM_CHANNEL LEDC_CHANNEL_1

void app_main() {
    // Çıkış pinlerini yapılandır
    gpio_config_t output_conf = {
        .pin_bit_mask = (1ULL << R_EN) | (1ULL << R_PWM) | (1ULL << L_EN) | (1ULL << L_PWM) | (1ULL << R_IS) | (1ULL << L_IS),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&output_conf);

    // Giriş pinlerini yapılandır
    gpio_config_t input_conf = {
        .pin_bit_mask = (1ULL << RightTouch) | (1ULL << LeftTouch),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE, // Pull-down direnç etkin
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&input_conf);

    // PWM zamanlayıcısını yapılandır
    ledc_timer_config_t pwm_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RES,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQ
    };
    ledc_timer_config(&pwm_timer);

    // Sağ PWM kanalını yapılandır
    ledc_channel_config_t r_pwm_channel = {
        .gpio_num = R_PWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = R_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0, // Başlangıçta durdur
        .hpoint = 0
    };
    ledc_channel_config(&r_pwm_channel);

    // Sol PWM kanalını yapılandır
    ledc_channel_config_t l_pwm_channel = {
        .gpio_num = L_PWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = L_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0, // Başlangıçta durdur
        .hpoint = 0
    };
    ledc_channel_config(&l_pwm_channel);

    // Motoru başlat
    gpio_set_level(R_EN, 1); // Sağ yön aktif
    gpio_set_level(L_EN, 1); // Sol yön aktif
    gpio_set_level(R_IS, 0); // R_IS LOW
    gpio_set_level(L_IS, 0); // L_IS LOW

    while (1) {
        if (gpio_get_level(RightTouch) == 1) {
            // Sağ dokunmatik tuş aktifse sağa dön
            ledc_set_duty(LEDC_LOW_SPEED_MODE, R_PWM_CHANNEL, PWM_DUTY_25);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, R_PWM_CHANNEL);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, L_PWM_CHANNEL, 0);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, L_PWM_CHANNEL);
        } else if (gpio_get_level(LeftTouch) == 1) {
            // Sol dokunmatik tuş aktifse sola dön
            ledc_set_duty(LEDC_LOW_SPEED_MODE, L_PWM_CHANNEL, PWM_DUTY_25);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, L_PWM_CHANNEL);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, R_PWM_CHANNEL, 0);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, R_PWM_CHANNEL);
        } else {
            // Hiçbir tuşa basılmıyorsa dur
            ledc_set_duty(LEDC_LOW_SPEED_MODE, R_PWM_CHANNEL, 0);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, R_PWM_CHANNEL);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, L_PWM_CHANNEL, 0);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, L_PWM_CHANNEL);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 100 ms gecikme
    }
}