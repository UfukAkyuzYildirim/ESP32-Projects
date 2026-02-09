#include "DShotFlightController.h"

// --- PID AYARLARI (REVIZE 3: SIKI DURUŞ + DRIFT KATİLİ) ---

// PITCH & ROLL (Gövde sertleşsin diye P artırıyoruz)
#define PR_P  1.7   // 1.0 idi -> 1.7 yaptık (Artık eline direnç gösterecek)
#define PR_I  0.01  // 0.0 idi -> 0.01 yaptık (Çok az hafıza ekledik ki açıyı tutsun)
#define PR_D  6.0   // 3.0 idi -> 6.0 yaptık (P artınca freni de artırdık titremesin diye)

// YAW (Drifti yok etmek için I değerini yükseltiyoruz)
#define YAW_P 4.0   // Burası iyi, kalsın.
#define YAW_I 0.40  // 0.15 idi -> 0.40 yaptık (Bu drifti affetmez, kafayı kilitler)
#define YAW_D 0.0   

#define MAX_I 50.0       
#define PID_MAX_OUT 300.0

// --- MANUEL TRIM AYARLARI (MEKANİK MONTAJ HATASI İÇİN) ---
// Bu değerler ile oynayarak dronun sürekli kaymasını engelleyeceğiz.
// Dron sürekli ÖNE gidiyorsa -> PITCH_TRIM'i artır (+2.0 gibi)
// Dron sürekli ARKAYA gidiyorsa -> PITCH_TRIM'i azalt (-2.0 gibi)
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
    
    // Motorları güvenli başlat
    motors.writeAllUs(0); 
    delay(500);

    // IMU Hazırlığı (Absolute Mod)
    calibrateIMU(); 

    return true;
}

void DShotFlightController::calibrateIMU() {
    Serial.println("--- SENSOR HAZIRLANIYOR (MUTLAK MOD) ---");
    // Sensörün kendine gelmesi için bekle
    delay(1000);
    
    // ARTIK HESAPLAMA YOK! 
    // Sensör zaten yerçekimini biliyor. 
    // Sadece senin montaj hatan varsa (Trim) onu ekliyoruz.
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
        
        if (now - lastLogTime > 500) {
           Serial.println("--- BEKLEMEDE ---");
           lastLogTime = now;
        }
        return;
    }

    // --- 2. KUMANDA ---
    int rawThrottle = radio.getLY(); 
    int rawYaw      = radio.getLX(); 
    int rawPitch    = radio.getRY(); 
    int rawRoll     = radio.getRX(); 

    // --- 3. GAZ ---
    if (abs(rawThrottle) > 50) { 
        float degisim = rawThrottle * 0.8f * dt; 
        currentThrottle += degisim;
    }
    if (currentThrottle < 1050) currentThrottle = 1050; 
    if (currentThrottle > 1800) currentThrottle = 1800; 

    // --- 4. HEDEF AÇILAR (MANEVRA YÖNÜ DÜZELTME) ---
    
    // 🔥 PITCH DÜZELTME: İleri itince Geri gidiyordu.
    // Eski: map(..., -30, 30) -> YENİ: map(..., 30, -30)
    // Artık ileri itince (pozitif raw) negatif açı (ileri) isteyecek.
    float targetPitch = map(rawPitch, -500, 500, 30, -30);
    
    // ROLL: Şikayet gelmediği için dokunmadım (-30, 30 standart)
    float targetRoll  = map(rawRoll,  -500, 500, -30, 30);
    
    // --- 5. SENSÖR (MUTLAK) ---
    DroneAngles angles = imu.getAngles();
    float actualPitch = angles.pitch - pitchOffset;
    float actualRoll  = angles.roll - rollOffset;

    // --- YAW KONTROLÜ (GYRO İLE) ---
    
    // 🔥 YAW DÜZELTME: Sola itince Sağa dönüyordu.
    // Eski: map(..., -150, 150) -> YENİ: map(..., 150, -150)
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
    mixMotors(currentThrottle, pitchPid, rollPid, yawPid);

    // --- LOG ---
    if (now - lastLogTime > 200) {
        Serial.printf("THR:%.0f | P:%.1f | R:%.1f | Y_Rate:%.1f\n", 
            currentThrottle, actualPitch, actualRoll, actualYawRate);
        lastLogTime = now;
    }
}

// 🔥 KARIŞIM MANTIĞI (PITCH TERSLENMİŞ - CORRECT) 🔥
void DShotFlightController::mixMotors(float throttle, float pitchPid, float rollPid, float yawPid) {
    
    // FL (Ön Sol) 
    float fl = throttle - pitchPid - rollPid + yawPid; 
    
    // FR (Ön Sağ) 
    float fr = throttle - pitchPid + rollPid - yawPid;
    
    // RL (Arka Sol)
    float rl = throttle + pitchPid - rollPid - yawPid;
    
    // RR (Arka Sağ)
    float rr = throttle + pitchPid + rollPid + yawPid;

    motors.writeMotor(0, (int)fl);
    motors.writeMotor(1, (int)fr);
    motors.writeMotor(2, (int)rl);
    motors.writeMotor(3, (int)rr);
}