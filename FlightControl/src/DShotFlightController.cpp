#include "DShotFlightController.h"

// --- PID SETTINGS ---
#define K_P  1.2   
#define K_I  0.0   
#define K_D  1.0   

#define MAX_I 50.0 
#define PID_MAX_OUT 250.0 

DShotFlightController::DShotFlightController(DShotMotorSystem &motorsRef, RadioSystem &radioRef, ImuSystem &imuRef)
    : motors(motorsRef), radio(radioRef), imu(imuRef),
      pidPitch(K_P, K_I, K_D, MAX_I),
      pidRoll(K_P, K_I, K_D, MAX_I),
      pidYaw(2.0, 0.0, 0.0, MAX_I), 
      currentThrottle(1000), lastTime(0), lastLogTime(0),
      pitchOffset(0), rollOffset(0) {} 

bool DShotFlightController::begin() {
    if (!radio.begin()) return false;
    if (!motors.begin()) return false;
    
    motors.writeAllUs(1000);
    delay(500);

    // Calibrate IMU on startup
    calibrateIMU(); 

    return true;
}

// IMU Calibration function
void DShotFlightController::calibrateIMU() {
    Serial.println("--- CALIBRATING IMU (DO NOT MOVE) ---");
    float pSum = 0;
    float rSum = 0;
    
    // Average 100 readings
    for(int i=0; i<100; i++) {
        DroneAngles ang = imu.getAngles();
        pSum += ang.pitch;
        rSum += ang.roll;
        delay(5);
    }
    
    pitchOffset = pSum / 100.0;
    rollOffset = rSum / 100.0;
    
    Serial.printf("Scale Ended -> P_Offset: %.2f | R_Offset: %.2f\n", pitchOffset, rollOffset);
}

void DShotFlightController::loopStep() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    if (dt > 0.1) dt = 0.01;
    lastTime = now;

    // 1. SAFETY CHECKS
    if (!radio.isConnectionAlive() || !radio.isSwitchOn()) {
        motors.disarm();
        pidPitch.reset();
        pidRoll.reset();
        pidYaw.reset();
        currentThrottle = 1000;
        return;
    }

    // 2. READ CONTROLLER
    int joyY = radio.getY();
    int joyX = radio.getX();

    // 3. THROTTLE CONTROL (INCREMENTAL)
    if (abs(joyY) > 50) {
        float degisim = joyY * 0.8f * dt; 
        currentThrottle += degisim;
    }
    // Limits
    if (currentThrottle < 1000) currentThrottle = 1000;
    if (currentThrottle > 1900) currentThrottle = 1900; 

    // 4. TARGET ANGLES & SENSOR READ
    float targetRoll = map(joyX, -500, 500, -25, 25); 
    float targetPitch = 0; 

    DroneAngles angles = imu.getAngles();
    
    // Apply calibration offsets
    float actualPitch = angles.pitch - pitchOffset;
    float actualRoll  = angles.roll - rollOffset;

    // 5. PID CALCULATION
    if (currentThrottle > 1050) {
        float pitchPid = pidPitch.compute(targetPitch, actualPitch, dt);
        float rollPid  = pidRoll.compute(targetRoll, actualRoll, dt);
        
        pitchPid = constrain(pitchPid, -PID_MAX_OUT, PID_MAX_OUT);
        rollPid  = constrain(rollPid, -PID_MAX_OUT, PID_MAX_OUT);

        mixMotors(currentThrottle, pitchPid, rollPid, 0);
    } 
    else {
        motors.writeMotor(0, 0);
        motors.writeMotor(1, 0);
        motors.writeMotor(2, 0);
        motors.writeMotor(3, 0);
        pidPitch.reset();
        pidRoll.reset();
    }

    // LOG
    if (now - lastLogTime > 200) {
        Serial.printf("Throttle:%.0f | P_Angle:%.1f | R_Angle:%.1f\n", 
            currentThrottle, actualPitch, actualRoll);
        lastLogTime = now;
    }
}

void DShotFlightController::mixMotors(float throttle, float pitchPid, float rollPid, float yawPid) {
    // Quad-X Standard Mixing
    float fl = throttle - pitchPid + rollPid + yawPid;
    float fr = throttle - pitchPid - rollPid - yawPid;
    float rl = throttle + pitchPid + rollPid - yawPid;
    float rr = throttle + pitchPid - rollPid + yawPid;

    motors.writeMotor(0, (int)fl);
    motors.writeMotor(1, (int)fr);
    motors.writeMotor(2, (int)rl);
    motors.writeMotor(3, (int)rr);
}