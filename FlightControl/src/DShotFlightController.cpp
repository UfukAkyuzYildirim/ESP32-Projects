#include "DShotFlightController.h"

// --- PID AYARLARI ---
#define PR_P  1.0   // Pitch and Roll P multipliers ( smaller count is more smooth but less responsive)
#define PR_I  0.02   // I impact (can be used to reduce steady-state error, but may cause instability if too high)
#define PR_D  12.0   // D impact (helps reduce overshoot and improve stability, but can cause noise if too high)

#define YAW_P 2.5   
#define YAW_I 0.0
#define YAW_D 0.0

#define MAX_I 50.0       
#define PID_MAX_OUT 300.0 

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
    
    // Motorları güvenli başlat (0 gönder)
    motors.writeAllUs(0); 
    delay(500);

    // IMU Kalibrasyonu
    calibrateIMU(); 

    return true;
}

// IMU KALİBRASYON
void DShotFlightController::calibrateIMU() {
    Serial.println("--- IMU KALIBRASYON (KIPIRDATMA!) ---");
    float pSum = 0;
    float rSum = 0;
    
    for(int i=0; i<200; i++) {
        DroneAngles ang = imu.getAngles();
        pSum += ang.pitch;
        rSum += ang.roll;
        delay(3);
    }
    
    pitchOffset = pSum / 200.0;
    rollOffset = rSum / 200.0;
    
    Serial.printf("Kalibrasyon Bitti -> P_Offset: %.2f | R_Offset: %.2f\n", pitchOffset, rollOffset);
}

void DShotFlightController::loopStep() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    
    if (dt > 0.1) dt = 0.01;
    lastTime = now;

    // --- 1. GÜVENLİK KONTROLLERİ ---
    if (!radio.isConnectionAlive()) {
        motors.disarm(); 
        currentThrottle = 1000;
        pidPitch.reset(); pidRoll.reset(); pidYaw.reset();
        return;
    }

    if (!radio.isSwitchOn()) {
        motors.disarm();
        currentThrottle = 1000;
        pidPitch.reset(); pidRoll.reset(); pidYaw.reset();
        
        if (now - lastLogTime > 500) {
           Serial.println("--- BEKLEMEDE (ARM ICIN SWITCH AC) ---");
           lastLogTime = now;
        }
        return;
    }

    // --- 2. KUMANDA OKUMA ---
    int rawThrottle = radio.getLY(); 
    int rawYaw      = radio.getLX(); 
    int rawPitch    = radio.getRY(); 
    int rawRoll     = radio.getRX(); 

    // --- 3. GAZ (THROTTLE) AYARI ---
    if (abs(rawThrottle) > 50) { 
        float degisim = rawThrottle * 0.8f * dt; 
        currentThrottle += degisim;
    }

    if (currentThrottle < 1050) currentThrottle = 1050; 
    if (currentThrottle > 1800) currentThrottle = 1800; 

    // --- 4. HEDEF AÇILAR ---
    float targetPitch = map(rawPitch, -500, 500, -30, 30);
    float targetRoll  = map(rawRoll,  -500, 500, -30, 30);
    
    // --- 5. SENSÖR OKUMA ---
    DroneAngles angles = imu.getAngles();
    float actualPitch = angles.pitch - pitchOffset;
    float actualRoll  = angles.roll - rollOffset;
    
    // Yaw Kontrolü (Basit P etkisi)
    float yawPid = rawYaw * 0.3; 

    // --- 6. PID HESAPLAMA ---
    float pitchPid = pidPitch.compute(targetPitch, actualPitch, dt);
    float rollPid  = pidRoll.compute(targetRoll,  actualRoll,  dt);
    
    pitchPid = constrain(pitchPid, -PID_MAX_OUT, PID_MAX_OUT);
    rollPid  = constrain(rollPid,  -PID_MAX_OUT, PID_MAX_OUT);

    // --- 7. MOTORLARA GÖNDER ---
    mixMotors(currentThrottle, pitchPid, rollPid, yawPid);

    // --- 8. LOGLAMA ---
    if (now - lastLogTime > 200) {
        Serial.printf("THR:%.0f | P_Tgt:%.1f P_Act:%.1f | R_Tgt:%.1f R_Act:%.1f\n", 
            currentThrottle, targetPitch, actualPitch, targetRoll, actualRoll);
        lastLogTime = now;
    }
}

// 🔥 DÜZELTİLMİŞ MOTOR KARIŞIMI (VERSION 3) 🔥
// Durum:
// 1. Roll (Sağ/Sol) düzelmişti (Önceki adımda yaptık).
// 2. Pitch (İleri/Geri) ters çalışıyor dedin (Arkaya eğince arkadakiler duruyor).
// ÇÖZÜM: Pitch işaretlerini TERSİNE çeviriyoruz.

void DShotFlightController::mixMotors(float throttle, float pitchPid, float rollPid, float yawPid) {
    
    // YENİ İŞARETLER:
    // PITCH: Önceden Önler (+) Arkalar (-) idi. ŞİMDİ TAM TERSİ.
    // ROLL:  Önceki ayarda bıraktık (Çünkü o düzelmişti sanırım).

    // FL (Ön Sol) -> Ön olduğu için Pitch ÇIKARILACAK (-)
    float fl = throttle - pitchPid - rollPid + yawPid; 
    
    // FR (Ön Sağ) -> Ön olduğu için Pitch ÇIKARILACAK (-)
    float fr = throttle - pitchPid + rollPid - yawPid;
    
    // RL (Arka Sol)-> Arka olduğu için Pitch EKLENECEK (+)
    float rl = throttle + pitchPid - rollPid - yawPid;
    
    // RR (Arka Sağ)-> Arka olduğu için Pitch EKLENECEK (+)
    float rr = throttle + pitchPid + rollPid + yawPid;

    // Motorlara Yaz
    motors.writeMotor(0, (int)fl);
    motors.writeMotor(1, (int)fr);
    motors.writeMotor(2, (int)rl);
    motors.writeMotor(3, (int)rr);
}