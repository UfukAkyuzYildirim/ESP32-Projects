#include "ImuTest.h"
#include "ImuSystem.h"
#include "DroneConfig.h"
#include <Adafruit_NeoPixel.h>

#ifndef LED_PIN
#define LED_PIN LED_PIN_ONBOARD
#endif

namespace {
void printImu(ImuSystem &imu) {
    DroneAngles ang = imu.getAngles();
    Serial.print("Euler (deg) => Roll: ");
    Serial.print(ang.roll);
    Serial.print(" | Pitch: ");
    Serial.print(ang.pitch);
    Serial.print(" | Yaw: ");
    Serial.println(ang.yaw);

    uint8_t sys, g, a, m;
    imu.getCalibration(sys, g, a, m);
    Serial.print("Calib Sys/G/A/M: ");
    Serial.print(sys);
    Serial.print("/");
    Serial.print(g);
    Serial.print("/");
    Serial.print(a);
    Serial.print("/");
    Serial.println(m);
}

enum class Scenario { Idle, CalibSeq, Figure8, Done };

struct State {
    Scenario scenario = Scenario::Idle;
    size_t stage = 0;
    unsigned long stageStart = 0;
    unsigned long lastPrint = 0;
    bool ledOn = false;
    bool rgbOn = false;
};

State state;
Adafruit_NeoPixel rgb(1, LED_RGB_PIN, NEO_GRB + NEO_KHZ800);

void setLed(bool on) {
    digitalWrite(LED_PIN, on ? HIGH : LOW);
    state.ledOn = on;
}

void setRgb(bool on) {
    if (on) {
        rgb.fill(rgb.Color(0, 0, 60));
    } else {
        rgb.clear();
    }
    rgb.show();
    state.rgbOn = on;
}

void blinkLedTimes(int times, int onMs = 150, int offMs = 150) {
    for (int i = 0; i < times; ++i) {
        setLed(true);
        delay(onMs);
        setLed(false);
        delay(offMs);
    }
}

void blinkRgbTimes(int times, int onMs = 150, int offMs = 150) {
    for (int i = 0; i < times; ++i) {
        setRgb(true);
        delay(onMs);
        setRgb(false);
        delay(offMs);
    }
}
} // namespace

bool ImuTest::setup(ImuSystem &imu, RadioSystem &radio) {
    Serial.begin(115200);
    Serial.println("--- IMU TEST ---");

    pinMode(LED_PIN, OUTPUT);
    setLed(false);

    rgb.begin();
    rgb.setBrightness(32);
    setRgb(true);
    Serial.print("RGB test on pin ");
    Serial.println(LED_RGB_PIN);

    Serial.print("LED test on pin ");
    Serial.println(LED_PIN);
    blinkLedTimes(3, 200, 200);

    if (!imu.begin()) {
        Serial.println("? IMU baslatilamadi!");
        return false;
    }
    Serial.println("? IMU hazir. Veriler geliyor...");

    state = {};
    state.scenario = Scenario::Idle;
    state.stageStart = 0;
    radio.isConnectionAlive();
    return true;
}

bool ImuTest::loop(ImuSystem &imu, RadioSystem &radio) {
    unsigned long now = millis();
    static int lastSw = 0;
    static unsigned long pressWindowStart = 0;
    static int pressCount = 0;
    static unsigned long lastPressTime = 0;
    bool requestFlight = false;

    int sw = radio.getSwitch();
    if (sw != lastSw && sw == 1) {
        unsigned long t = millis();
        if (pressWindowStart == 0 || t - pressWindowStart > 1500) {
            pressWindowStart = t;
            pressCount = 0;
        }
        pressCount++;
        lastPressTime = t;

        setRgb(!state.rgbOn);
    }
    lastSw = sw;

    const unsigned long decisionDelay = 600;
    const bool canStart = (state.scenario == Scenario::Idle || state.scenario == Scenario::Done);
    if (pressCount > 0 && canStart && lastPressTime > 0 && (now - lastPressTime) > decisionDelay) {
        if (pressCount >= 4) {
            Serial.println("[START] 4x buton: 8 cizme testi basliyor");
            blinkLedTimes(3);
            state = {};
            state.scenario = Scenario::Figure8;
            state.stage = 0;
            state.stageStart = 0;
        } else if (pressCount == 3) {
            Serial.println("[START] 3x buton: Kalibrasyon sekansi basliyor");
            state = {};
            state.scenario = Scenario::CalibSeq;
            state.stage = 0;
            state.stageStart = 0;
        } else if (pressCount == 1) {
            Serial.println("[START] 1x buton: Ucus moduna geciliyor");
            requestFlight = true;
        }
        pressWindowStart = 0;
        pressCount = 0;
        lastPressTime = 0;
        return requestFlight;
    }

    if (state.scenario == Scenario::Done) {
        if (state.ledOn) setLed(false);
        if (state.rgbOn) setRgb(false);
        delay(10);
        return false;
    }

    if (state.scenario == Scenario::Idle) {
        if (now - state.lastPrint >= 1000) {
            Serial.println("Buton 1x: Ucus | 3x: Kalibrasyon | 4x: 8 cizme");
            printImu(imu);
            state.lastPrint = now;
        }
        delay(10);
        return false;
    }

    const unsigned long poseDuration = 10000;
    const char* calibPrompts[] = {
        "---------- SAGA YATIR (10SN)",
        "---------- SOLA YATIR (10SN)",
        "---------- BURNU KALDIR (10SN)",
        "---------- ARKAYI KALDIR (10SN)",
        "---------- SABIT TUT (10SN)",
    };

    auto startCalibStage = [&](size_t idx) {
        blinkRgbTimes(5, 200, 200);
        setRgb(true);
        state.stageStart = millis();
        Serial.println(calibPrompts[idx]);
    };

    if (state.scenario == Scenario::CalibSeq) {
        const size_t promptCount = sizeof(calibPrompts) / sizeof(calibPrompts[0]);

        if (state.stage == 0 && state.stageStart == 0) {
            startCalibStage(0);
            return false;
        }

        if (now - state.stageStart >= poseDuration) {
            state.stage++;
            if (state.stage >= promptCount) {
                Serial.println("[BITTI] Kalibrasyon sekansi tamamlandi");
                blinkRgbTimes(5, 80, 80);
                setRgb(false);
                state.scenario = Scenario::Done;
                setLed(false);
            } else {
                startCalibStage(state.stage);
                return false;
            }
        }
    }

    auto startFigStage = [&](size_t idx) {
        blinkRgbTimes(5, 200, 200);
        setRgb(true);
        state.stageStart = millis();
        Serial.print("---------- 8 CIZ (10SN) #");
        Serial.println(idx + 1);
    };

    if (state.scenario == Scenario::Figure8) {
        const unsigned long figDuration = 10000;
        if (state.stage == 0 && state.stageStart == 0) {
            startFigStage(0);
            return false;
        }

        if (now - state.stageStart >= figDuration) {
            if (state.stage == 0) {
                state.stage = 1;
                blinkRgbTimes(5, 200, 200);
                setRgb(true);
                state.stageStart = millis();
                Serial.println("---------- 8 CIZ (10SN) #2");
                return false;
            } else {
                Serial.println("[BITTI] 2x 8 cizme tamamlandi");
                blinkRgbTimes(5, 80, 80);
                setRgb(false);
                state.scenario = Scenario::Done;
                setLed(false);
            }
        }
    }

    if (now - state.lastPrint >= 1000) {
        printImu(imu);
        state.lastPrint = now;
    }

    delay(10);
    return false;
}
