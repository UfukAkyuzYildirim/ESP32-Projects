#include "MotorSystem.h"

MotorSystem::MotorSystem() {
    armed = false;
}

void MotorSystem::begin() {
    mFL.attach(PIN_FL, MIN_PULSE, MAX_PULSE);
    mFR.attach(PIN_FR, MIN_PULSE, MAX_PULSE);
    mRL.attach(PIN_RL, MIN_PULSE, MAX_PULSE);
    mRR.attach(PIN_RR, MIN_PULSE, MAX_PULSE);
    disarm();
}

void MotorSystem::arm() {
    armed = true;
}

void MotorSystem::disarm() {
    armed = false;
    mFL.writeMicroseconds(MIN_PULSE);
    mFR.writeMicroseconds(MIN_PULSE);
    mRL.writeMicroseconds(MIN_PULSE);
    mRR.writeMicroseconds(MIN_PULSE);
}

bool MotorSystem::isArmed() {
    return armed;
}

void MotorSystem::writeAll(int pwm) {
    if (!armed) {
        disarm();
        return;
    }
    pwm = constrain(pwm, MIN_PULSE, MAX_PULSE);

    mFL.writeMicroseconds(pwm);
    mFR.writeMicroseconds(pwm);
    mRL.writeMicroseconds(pwm);
    mRR.writeMicroseconds(pwm);
}

void MotorSystem::writeMixed(int fl, int fr, int rl, int rr) {
    if (!armed) return;

    mFL.writeMicroseconds(fl);
    mFR.writeMicroseconds(fr);
    mRL.writeMicroseconds(rl);
    mRR.writeMicroseconds(rr);
}