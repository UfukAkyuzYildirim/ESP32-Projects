#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include "driver/gpio.h"
#include "driver/ledc.h"

class MotorController {
public:
    MotorController(gpio_num_t right_pwm, gpio_num_t left_pwm, gpio_num_t right_enable, gpio_num_t left_enable);
    void init();
    void turnRight();
    void turnLeft();
    void stop();

private:
    gpio_num_t _right_pwm, _left_pwm, _right_enable, _left_enable;
    ledc_channel_config_t _r_pwm_channel, _l_pwm_channel;
    static constexpr uint32_t PWM_FREQ = 5000;
    static constexpr ledc_timer_bit_t PWM_RES = LEDC_TIMER_8_BIT;
    static constexpr uint32_t PWM_DUTY_25 = (255 * 25) / 100;
};

#endif // MOTOR_CONTROLLER_H
