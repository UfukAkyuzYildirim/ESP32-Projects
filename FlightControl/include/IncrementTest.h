#pragma once

#include <ESP32Servo.h>
#include "RadioSystem.h"

namespace IncrementTest {
void setup(Servo &mFL, Servo &mFR, Servo &mRL, Servo &mRR, RadioSystem &radio, int &currentPwm, bool &isStickReady);
void loop(Servo &mFL, Servo &mFR, Servo &mRL, Servo &mRR, RadioSystem &radio, int &currentPwm, bool &isStickReady);
}
