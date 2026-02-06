#pragma once

#include <ESP32Servo.h>
#include "RadioSystem.h"

namespace CalibrationTest {
void setup(Servo &mFL, Servo &mFR, Servo &mRL, Servo &mRR, RadioSystem &radio);
void loop(Servo &mFL, Servo &mFR, Servo &mRL, Servo &mRR, RadioSystem &radio);
}
