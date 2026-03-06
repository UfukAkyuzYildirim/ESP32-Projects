#include "DShotFlightController.h"

// --- PID Configuration (Optimized for Hold & Drift) ---

// PITCH & ROLL
#define PR_P  1.7   
#define PR_I  0.01  
#define PR_D  6.0   

// YAW 
#define YAW_P 4.0   
#define YAW_I 0.40  // I-term for drift correction
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
      pitchOffset(0), rollOffset(0), yawOffset(0), targetHeading(0) {} 

bool DShotFlightController::begin() {
    if (!radio.begin()) return false;
    if (!motors.begin()) return false;
    motors.writeAllUs(0); 
    delay(500);
    calibrateIMU(); 
    return true;
}

void DShotFlightController::calibrateIMU() {
    Serial.println("--- CALIBRATING SENSOR ---");
    delay(1000);
    pitchOffset = MANUAL_PITCH_TRIM;
    rollOffset  = MANUAL_ROLL_TRIM;
    
    // Set initial Yaw reference (Zero Point)
    DroneAngles angles = imu.getAngles();
    yawOffset = angles.yaw;

    Serial.printf("Ready. Trims -> P: %.2f | R: %.2f | Y_Off: %.2f\n", pitchOffset, rollOffset, yawOffset);
}

// Calculate relative Yaw angle based on offset (-180..+180)
float DShotFlightController::getRelativeYaw() {
    float currentYaw = imu.getAngles().yaw;
    float relative = currentYaw - yawOffset;

    // Handle 360-degree wrap-around
    if (relative > 180) relative -= 360;
    if (relative < -180) relative += 360;
    
    return relative;
}

float DShotFlightController::getErrorShortestPath(float target, float current) {
    float error = target - current;
    if (error > 180) error -= 360;
    if (error < -180) error += 360;
    return error;
}

void DShotFlightController::loopStep() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    if (dt > 0.1) dt = 0.01;
    lastTime = now;

    // --- ARMING & OFFSET RESET LOGIC ---
    static bool wasArmed = false;
    bool isArmed = radio.isSwitchOn();

    // Check for Arm Switch transition (OFF -> ON)
    if (isArmed && !wasArmed) {
        DroneAngles angles = imu.getAngles();
        yawOffset = angles.yaw; // Set current heading as zero reference
        targetHeading = 0;      // Reset target heading
    }
    wasArmed = isArmed;
    // -----------------------------------

    // --- 1. SAFETY CHECK ---
    if (!radio.isConnectionAlive() || !isArmed) {
        motors.disarm(); 
        currentThrottle = 1000;
        pidPitch.reset(); pidRoll.reset(); pidYaw.reset();
        return;
    }

    // --- 2. RC INPUT ---
    // Throttle calculation
    int rawThrottle = radio.getLY(); 
    if (abs(rawThrottle) > 50) { // Deadzone
        float throttleSpeed = 3.5f; 
        float degisim = rawThrottle * throttleSpeed * dt; 
        currentThrottle += degisim;
    }
    // Limit Throttle Range
    if (currentThrottle < 1050) currentThrottle = 1050; 
    if (currentThrottle > 1900) currentThrottle = 1900; 

    // Pitch & Roll Targets
    int rawPitch    = radio.getRY(); 
    int rawRoll     = radio.getRX(); 
    float targetPitch = map(rawPitch, -500, 500, 30, -30);
    float targetRoll  = map(rawRoll,  -500, 500, -30, 30);
    
    // --- 3. YAW CONTROL (HEADING HOLD) ---
    int rawYaw = radio.getLX(); 
    float yawPid = 0;
    float currentYaw = getRelativeYaw();
    bool isYawMoving = (abs(rawYaw) > 15); // Deadzone
    
    float actualYawRate = imu.getRate().yaw; 

    if (isYawMoving) {
        // --- MODE A: RATE CONTROL (MANUAL) ---
        // Reverse RC input mapping for correct yaw direction
        float targetYawRate = map(rawYaw, -500, 500, -150, 150);
        
        yawPid = pidYaw.compute(targetYawRate, actualYawRate, dt);
        
        // Update target heading to current while moving
        targetHeading = currentYaw; 
    } else {
        // --- MODE B: HEADING HOLD (AUTO-STABILIZATION) ---
        
        // Active only when airborne (Throttle > 1150)
        if (currentThrottle > 1150) {
            // Calculate shortest path error to target
            float error = getErrorShortestPath(targetHeading, currentYaw);
            
            // Convert angle error to desired rotation rate (P-Controller behavior)
            // Multiplier 3.0 provides 30 deg/s correction for 10 deg error
            float desiredRate = error * 3.0; 
            
            // Compute PID to achieve desired rate
            yawPid = pidYaw.compute(desiredRate, actualYawRate, dt);

        } else {
            // Disable Heading Hold on ground, sync target
            yawPid = 0;
            targetHeading = currentYaw;
        }
    }
    yawPid = constrain(yawPid, -PID_MAX_OUT, PID_MAX_OUT);

    // --- 4. PITCH & ROLL PID ---
    DroneAngles angles = imu.getAngles();
    float actualPitch = angles.pitch - pitchOffset;
    float actualRoll  = angles.roll - rollOffset;

    float pitchPid = pidPitch.compute(targetPitch, actualPitch, dt);
    float rollPid  = pidRoll.compute(targetRoll,  actualRoll,  dt);
    
    pitchPid = constrain(pitchPid, -PID_MAX_OUT, PID_MAX_OUT);
    rollPid  = constrain(rollPid,  -PID_MAX_OUT, PID_MAX_OUT);

    // --- 5. MOTOR MIXING ---
    // Disable PID mixing at idle throttle
    if (currentThrottle < 1060) {
        pitchPid = 0; rollPid = 0; yawPid = 0;
    }

    mixMotors(currentThrottle, pitchPid, rollPid, yawPid);

    if (now - lastLogTime > 200) {
        // WebSerial Logging (Includes Yaw/Heading)
        Serial.printf("THR:%.0f | P:%.1f | R:%.1f | Y:%.1f | T_Y:%.1f\n", 
            currentThrottle, actualPitch, actualRoll, currentYaw, targetHeading);
        lastLogTime = now;
    }
}

void DShotFlightController::mixMotors(float throttle, float pitchPid, float rollPid, float yawPid) {
    // Reconfigured Yaw Mixing for Heading Hold (Positive PID = CW Rotation)
    // Corrects previous direction inversion. CW motors decrease, CCW motors increase on positive PID.
    
    // YAW EXPLANATION:
    // If drone rotates Left (CCW), Error is Positive -> PID is Positive.
    // Positive PID -> Must produce Right (CW) torque.
    // Therefore, CCW motors (FR, RL) must increase.
    // CW motors (FL, RR) must decrease.
    
    float fl = throttle - pitchPid + rollPid - yawPid; 
    float fr = throttle - pitchPid - rollPid + yawPid; 
    float rl = throttle + pitchPid + rollPid + yawPid; 
    float rr = throttle + pitchPid - rollPid - yawPid; 

    // Store for logging
    motorFL = fl; motorFR = fr; motorRL = rl; motorRR = rr;

    motors.writeMotor(0, (int)fl);
    motors.writeMotor(1, (int)fr);
    motors.writeMotor(2, (int)rl);
    motors.writeMotor(3, (int)rr);
}