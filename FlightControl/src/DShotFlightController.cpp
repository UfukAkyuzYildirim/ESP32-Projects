#include "DShotFlightController.h"

// --- PID AYARLARI (GÜÇLÜ + HAFIZALI + DRIFT YOK) ---

// PITCH & ROLL
#define PR_P  1.7   
#define PR_I  0.01  
#define PR_D  6.0   

// YAW 
#define YAW_P 4.0   
#define YAW_I 0.40  // Sola kaymayı bitiren ayar
#define YAW_D 0.0   

#define MAX_I 50.0       
#define PID_MAX_OUT 300.0

float MANUAL_PITCH_TRIM = 0.0; 
float MANUAL_ROLL_TRIM  = 0.0;

DShotFlightController::DShotFlightController(DShotMotorSystem &motorsRef, RadioSystem &radioRef, ImuSystem &imuRef)
    : motors(motorsRef), radio(radioRef), imu(imuRef),
      pidPitch(PR_P, PR_I, PR_D, MAX_I),
      pidRoll(PR_P, PR_I, PR_D, MAX_I),
      pidYaw(YAW_P, YAW_I, YAW_D, MAX_I), 
      currentThrottle(1000), lastTime(0), lastLogTime(0),
      pitchOffset(0), rollOffset(0) {} 

bool DShotFlightController::begin() {
    if (!radio.begin()) return false;
    if (!motors.begin()) return false;
    motors.writeAllUs(0); 
    delay(500);
    calibrateIMU(); 
    return true;
}

void DShotFlightController::calibrateIMU() {
    Serial.println("--- SENSOR HAZIRLANIYOR ---");
    delay(1000);
    pitchOffset = MANUAL_PITCH_TRIM;
    rollOffset  = MANUAL_ROLL_TRIM;
    Serial.printf("Hazir. Trimler -> P: %.2f | R: %.2f\n", pitchOffset, rollOffset);
}

void DShotFlightController::loopStep() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    if (dt > 0.1) dt = 0.01;
    lastTime = now;

    // --- 1. GÜVENLİK ---
    if (!radio.isConnectionAlive() || !radio.isSwitchOn()) {
        motors.disarm(); 
        currentThrottle = 1000;
        pidPitch.reset(); pidRoll.reset(); pidYaw.reset();
        return;
    }

    // --- 2. KUMANDA ---
    int rawThrottle = radio.getLY(); 
    int rawYaw      = radio.getLX(); 
    int rawPitch    = radio.getRY(); 
    int rawRoll     = radio.getRX(); 

    if (abs(rawThrottle) > 50) { // Deadzone

        float throttleSpeed = 3.5f; 
        float degisim = rawThrottle * throttleSpeed * dt; 
        currentThrottle += degisim;
    }
    
    // Limitler
    if (currentThrottle < 1050) currentThrottle = 1050; 
    if (currentThrottle > 1900) currentThrottle = 1900; 
    float targetPitch = map(rawPitch, -500, 500, 30, -30);
    float targetRoll  = map(rawRoll,  -500, 500, -30, 30);
    
    // --- 5. SENSÖR ---
    DroneAngles angles = imu.getAngles();
    float actualPitch = angles.pitch - pitchOffset;
    float actualRoll  = angles.roll - rollOffset;

    float targetYawRate = map(rawYaw, -500, 500, 150, -150);
    DroneAngles rates = imu.getRate(); 
    float actualYawRate = rates.yaw; 

    float yawPid = pidYaw.compute(targetYawRate, actualYawRate, dt);
    yawPid = constrain(yawPid, -PID_MAX_OUT, PID_MAX_OUT);

    // --- PID ---
    float pitchPid = pidPitch.compute(targetPitch, actualPitch, dt);
    float rollPid  = pidRoll.compute(targetRoll,  actualRoll,  dt);
    
    pitchPid = constrain(pitchPid, -PID_MAX_OUT, PID_MAX_OUT);
    rollPid  = constrain(rollPid,  -PID_MAX_OUT, PID_MAX_OUT);

    // --- KARIŞTIRMA ---
    // Gaz rölantideyse (1100 altı) PID karışmasın, yerde sekmesin.
    if (currentThrottle < 1100) {
        pitchPid = 0; rollPid = 0; yawPid = 0;
    }

    mixMotors(currentThrottle, pitchPid, rollPid, yawPid);

    if (now - lastLogTime > 200) {
        Serial.printf("THR:%.0f | P:%.1f | R:%.1f | YawRate:%.1f\n", 
            currentThrottle, actualPitch, actualRoll, actualYawRate);
        lastLogTime = now;
    }
}

void DShotFlightController::mixMotors(float throttle, float pitchPid, float rollPid, float yawPid) {
    float fl = throttle - pitchPid - rollPid + yawPid; 
    float fr = throttle - pitchPid + rollPid - yawPid;
    float rl = throttle + pitchPid - rollPid - yawPid;
    float rr = throttle + pitchPid + rollPid + yawPid;

    motors.writeMotor(0, (int)fl);
    motors.writeMotor(1, (int)fr);
    motors.writeMotor(2, (int)rl);
    motors.writeMotor(3, (int)rr);
}