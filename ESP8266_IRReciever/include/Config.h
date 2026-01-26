#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pin Tanimlamalari (NodeMCU V2)
#define PIN_IR_RECV 4   // D2 - IR Alici
#define PIN_IR_SEND 5   // D1 - IR Verici
#define PIN_BUTTON  14  // D5 - Kontrol Butonu
// LED Ayarlari
// NodeMCU uzerindeki dahili LED genelde D4 (GPIO2) ve aktif-low.
// Ayrica harici bir LED'i D6 (GPIO12) uzerinden de yakabiliriz.
#define PIN_LED_BUILTIN        LED_BUILTIN
#define LED_BUILTIN_ACTIVE_LOW 1

#define PIN_LED_EXTERNAL        12  // D6
#define LED_EXTERNAL_ACTIVE_LOW 0

// Zamanlamalar (Milisaniye)
#define LEARN_MODE_TIMEOUT    30000 // 30 saniye
#define LEARN_PRESS_DURATION  2000  // 2 saniye basili tutup birakinca: ogrenme modu
#define AP_PRESS_DURATION     5000  // 5 saniye basili tutup birakinca: WiFi setup AP modu
#define TRIPLE_CLICK_INTERVAL 1000  // Tiklamalar arasi maksimum sure
#define TRIPLE_CLICK_COUNT    3     // Kac kere basilinca iptal olsun

// Wifi Ayarlari
#define HTTP_PORT 8088

// mDNS (STA modunda cihaz adıyla erişim)
// Cihaz WiFi'a (STA) bağlandığında `http://<hostname>.local:8088` şeklinde erişebilmek için.
// Not: Aynı ağda birden fazla cihaz varsa çakışma olmaması için ileride benzersiz hostname'e geçebiliriz.
#define MDNS_HOSTNAME "ir-gateway"

// WiFi Provisioning (AP Setup) Ayarlari
// Cihaz bir ağa bağlanamazsa kendi AP'ini açar ve bu AP üzerinden WiFi bilgileri set edilir.
#define SETUP_AP_SSID_PREFIX "IR-Gateway-"
// Boş bırakılırsa açık (şifresiz) AP olur. (En az 8 karakter olmalı)
#define SETUP_AP_PASSWORD ""

// WiFi bağlantı deneme süresi
#define WIFI_CONNECT_TIMEOUT_MS 10000

#endif
