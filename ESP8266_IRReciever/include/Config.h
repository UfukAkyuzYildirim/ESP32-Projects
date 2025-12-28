#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pin Tanimlamalari (NodeMCU V2)
#define PIN_IR_RECV 4   // D2 - IR Alici
#define PIN_IR_SEND 5   // D1 - IR Verici
#define PIN_BUTTON  14  // D5 - Kontrol Butonu
#define PIN_LED     12  // D6 - Durum LED'i

// Zamanlamalar (Milisaniye)
#define LEARN_MODE_TIMEOUT    30000 // 30 saniye
#define LONG_PRESS_DURATION   3000  // 3 saniye basili tutma suresi
#define TRIPLE_CLICK_INTERVAL 1000  // Tiklamalar arasi maksimum sure
#define TRIPLE_CLICK_COUNT    3     // Kac kere basilinca iptal olsun

// Wifi Ayarlari
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
#define HTTP_PORT 8088


#endif
