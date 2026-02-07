#ifndef DSHOT_MOTOR_SYSTEM_H
#define DSHOT_MOTOR_SYSTEM_H

#include <Arduino.h>
#include <driver/rmt.h>
#include "DroneConfig.h"

class DShotMotorSystem {
public:
    DShotMotorSystem();
    bool begin();
    void disarm();
    void writeAllUs(int microseconds);
    void writeMotor(int index, int microseconds);

private:
    struct ChannelCfg {
        rmt_channel_t channel;
        gpio_num_t pin;
    };

    ChannelCfg channels[4];
    bool armed;

    bool setupChannel(const ChannelCfg &cfg);
    void sendDshotValue(const ChannelCfg &cfg, uint16_t value);
    uint16_t usToDshot(int microseconds) const;
};

#endif
