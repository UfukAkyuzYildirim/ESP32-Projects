# ESP8266_IRReciever

ESP8266 (NodeMCU v2 / ESP-12E) uzerinde:
- IR kod ogrenme + gonderme (RAW dahil)
- HTTP API
- WiFi provisioning (SSID/PASS hardcode yok)

## WiFi Provisioning Akisi
1) Cihaz acilista LittleFS'ten `/wifi.json` okur.
2) Kayitli WiFi yoksa veya STA baglantisi timeout olursa `Setup AP` acar.
3) Telefon/PC ile bu AP'ye baglanip WiFi bilgisini HTTP ile gonderirsiniz.
4) Cihaz kaydeder ve restart olur; sonraki acilista STA modunda baglanir.

Not: LittleFS ilk boot'ta bos/formatlanmamis ise otomatik format denenir.

## Buton / LED
- Buton pini: D5 (GPIO14)
- LED: varsayilan olarak iki LED birlikte surulur
	- Dahili: `LED_BUILTIN` (D4 / GPIO2, aktif-low)
	- Harici (opsiyonel): D6 (GPIO12)

Buton aksiyonlari (basili tutup birakinca):
- 2 saniye: IR ogrenme modu (30 sn)
- 5 saniye: WiFi Setup AP moduna gecis

LED gostergeleri:
- IR ogrenme moduna giriste: 3 kere yanip soner
- Setup AP aktifken: surekli yanip soner

LED pin ayarlari icin: [ESP8266_IRReciever/include/Config.h](ESP8266_IRReciever/include/Config.h)

## HTTP
Varsayilan port: `8088`

### mDNS (STA modunda)

Cihaz ev/ofis WiFi'ina (STA) baglandiginda mDNS yayini yapar:

- Hostname: `ir-gateway`
- Base URL: `http://ir-gateway.local:8088`

Not: Setup AP modunda base URL genellikle `http://192.168.4.1:8088` olur.

### Durum
- `GET /status`

### WiFi Ayarlama (Setup AP uzerinden)
- `POST /wifi/config`

Ornek (PowerShell):
```powershell
$body = @{ ssid = "YOUR_WIFI"; pass = "YOUR_PASS" } | ConvertTo-Json
Invoke-RestMethod -Method Post -Uri http://192.168.4.1:8088/wifi/config -ContentType 'application/json' -Body $body
```

Ardindan cihaz `ESP.restart()` yapar.

## Build / Upload (PlatformIO)
Windows'ta `pio` PATH'te degilse:
```powershell
py -m platformio run -d ESP8266_IRReciever
py -m platformio run -d ESP8266_IRReciever -t upload
py -m platformio device monitor -b 115200
```

## Ayarlar
Provisioning ile ilgili sabitler: `include/Config.h`
- Setup AP SSID prefix
- Setup AP password (bos/7 karakterden kisa ise sifresiz AP)
- STA connect timeout
