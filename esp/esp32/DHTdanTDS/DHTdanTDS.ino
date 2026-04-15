#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <DHT.h>

// =================================================================
//                      --- KONFIGURASI ---
// =================================================================

// --- Pin dan Sensor ---
#define TDS_SENSOR_PIN    34  // ADC1
#define PH_SENSOR_PIN     35  // ADC1
#define DHT11_PIN_ATAS    33
#define DHT11_PIN_BAWAH   32
#define DHT11_TYPE        DHT11

// --- Jaringan ---
const char *WIFI_SSID     = "Podcast Area";
const char *WIFI_PASSWORD = "iriunwebcam";
// NOTE: kalau server pakai port/path custom, pakai "ws://103.147.92.179:3000/ws"
const char *server_url    = "ws://103.147.92.179";
const char *device_id     = "esp32-environment-device";

// --- Umum ---
#define SCOUNT            30
const unsigned long SEND_INTERVAL       = 5000;  // 5s
const unsigned long WIFI_TIMEOUT        = 15000; // buat attempt awal di setup
const unsigned long TDS_SAMPLE_INTERVAL = 40;    // 40 ms

// --- pH calibration (isi sesuai hasil real) ---
const float pH_low  = 4.01;
const float pH_high = 9.18;
const float V_low   = 3.15;  // Volt saat pH 4.01
const float V_high  = 2.05;  // Volt saat pH 9.18

// --- TDS calibration ---
float TDS_FACTOR = 0.5f;  // 0.5 (air minum). Coba 0.65 kalau mau mendekati handheld meter komersial
float TDS_SCALE  = 0.0f;  // scaling hasil two-point calibration
float TDS_OFFSET = 0.0f;   // offset ppm

// =================================================================
//                    --- VARIABEL GLOBAL ---
// =================================================================
int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
unsigned long lastAnalogSampleTime = 0;

float slope = 0.0f, intercept = 0.0f;
unsigned long lastSendTime = 0;

float tdsValue = 0, phValue = 0;
float temperature_atas = 0, humidity_atas = 0, temperature_bawah = 0, humidity_bawah = 0;

DHT dht11Atas(DHT11_PIN_ATAS, DHT11_TYPE);
DHT dht11Bawah(DHT11_PIN_BAWAH, DHT11_TYPE);

using namespace websockets;
WebsocketsClient client;
bool isWebsocketConnected = false;

// Retry cooldown (non-blocking reconnect)
unsigned long lastWifiAttempt = 0;
unsigned long lastWsAttempt   = 0;
const unsigned long WIFI_RETRY_INTERVAL = 5000; // 5s
const unsigned long WS_RETRY_INTERVAL   = 5000; // 5s

// =================================================================
//                  --- UTIL & EVENT HANDLERS ---
// =================================================================
int compareInt(const void* a, const void* b) {
  return (*(int*)a - *(int*)b);
}

// Baca tegangan rata-rata (Volt) pakai ADC kalibrasi mV
float readVoltage_V(int pin, int samples = 10) {
  long mv = 0;
  for (int i = 0; i < samples; i++) {
    mv += analogReadMilliVolts(pin); // sudah kalibrasi via eFuse
    delay(1); // beri waktu scheduler
  }
  return (mv / (float)samples) / 1000.0f; // mV -> V
}

void onWebsocketEvent(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("[WS] Koneksi Terbuka");
    isWebsocketConnected = true;

    // Kirim registrasi saat connect
    StaticJsonDocument<256> reg;
    reg["deviceId"] = device_id;
    reg["type"]     = "join";
    reg["room"]     = "environment";
    String s;
    serializeJson(reg, s);
    client.send(s);
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("[WS] Koneksi Tertutup");
    isWebsocketConnected = false;
    lastWsAttempt = millis(); // debounce reconnect
  } else if (event == WebsocketsEvent::GotPing) {
    Serial.println("[WS] Ping diterima");
  } else if (event == WebsocketsEvent::GotPong) {
    Serial.println("[WS] Pong diterima");
  }
}

// Reconnect non-blocking (cooldown)
void reconnectServicesNonBlocking() {
  unsigned long now = millis();

  // WiFi
  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWifiAttempt >= WIFI_RETRY_INTERVAL) {
      lastWifiAttempt = now;
      Serial.println("[reconnect] WiFi retry...");
      WiFi.reconnect(); // non-blocking
      yield();
    }
    return; // jangan lanjut WS kalau WiFi belum nyambung
  }

  // WebSocket
  if (!isWebsocketConnected) {
    if (now - lastWsAttempt >= WS_RETRY_INTERVAL) {
      lastWsAttempt = now;
      Serial.println("[reconnect] WS retry...");
      bool ok = client.connect(server_url); // sync, tapi cepat kalau server responsif
      yield();
      if (!ok) {
        Serial.println("[reconnect] WS connect failed");
      }
    }
  }
}

// =================================================================
//                         --- SETUP ---
// =================================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  // ADC setup (ESP32)
  analogReadResolution(12); // 0..4095
  analogSetPinAttenuation(TDS_SENSOR_PIN, ADC_11db); // ~0..3.3V
  analogSetPinAttenuation(PH_SENSOR_PIN,  ADC_11db);

  pinMode(TDS_SENSOR_PIN, INPUT);
  pinMode(PH_SENSOR_PIN, INPUT);

  dht11Atas.begin();
  dht11Bawah.begin();

  // pH calibration line (Volt -> pH)
  slope     = (pH_high - pH_low) / (V_high - V_low);
  intercept = pH_high - slope * V_high;
  Serial.println("[pH] Kalibrasi:");
  Serial.print("  slope = "); Serial.println(slope, 6);
  Serial.print("  intercept = "); Serial.println(intercept, 6);

  // WebSocket event
  client.onEvent(onWebsocketEvent);

  // WiFi awal (boleh blocking sebentar saat boot)
  Serial.print("Menghubungkan ke WiFi ..");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {
    delay(250);
    Serial.print('.');
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nTerhubung ke WiFi!");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nGagal WiFi awal. Lanjut retry non-blocking di loop.");
  }
}

// =================================================================
//                         --- LOOP ---
// =================================================================
void loop() {
  unsigned long now = millis();

  // Selalu panggil poll
  client.poll();
  reconnectServicesNonBlocking();
  yield();

  // --- Sampling TDS (non-blocking) ---
  if (now - lastAnalogSampleTime > TDS_SAMPLE_INTERVAL) {
    lastAnalogSampleTime = now;
    analogBuffer[analogBufferIndex] = analogRead(TDS_SENSOR_PIN);
    analogBufferIndex = (analogBufferIndex + 1) % SCOUNT;
  }

  // --- Kirim data berkala ---
  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;

    // 1) DHT
    temperature_atas  = dht11Atas.readTemperature();
    humidity_atas     = dht11Atas.readHumidity();
    temperature_bawah = dht11Bawah.readTemperature();
    humidity_bawah    = dht11Bawah.readHumidity();
    if (isnan(temperature_atas))  temperature_atas = 0;
    if (isnan(humidity_atas))      humidity_atas = 0;
    if (isnan(temperature_bawah))  temperature_bawah = 0; // default netral utk kompensasi
    if (isnan(humidity_bawah))     humidity_bawah = 0;
    yield();

    // 2) pH (pakai ADC mV -> Volt)
    float volt_pH = readVoltage_V(PH_SENSOR_PIN, 10);
    phValue = slope * volt_pH + intercept;

    // 3) TDS (median filter)
    for (int i = 0; i < SCOUNT; i++) analogBufferTemp[i] = analogBuffer[i];
    qsort(analogBufferTemp, SCOUNT, sizeof(int), compareInt);
    float medianRaw = (SCOUNT & 1)
        ? analogBufferTemp[(SCOUNT - 1) / 2]
        : (analogBufferTemp[SCOUNT / 2] + analogBufferTemp[SCOUNT / 2 - 1]) / 2.0f;

    // Konversi RAW -> Volt (gunakan rentang ADC 12-bit)
    // NOTE: karena kita sampling raw di buffer, konversi manual:
    // Lebih akurat pakai analogReadMilliVolts saat sampling, tapi ini tetap OK.
    const float VREF_ADC = 3.3f; // kira-kira, ADC_11db up to ~3.3V
    float averageVoltage = medianRaw * (VREF_ADC / 4095.0f);

    // Kompensasi suhu
    float compensationCoefficient = 1.0f + 0.02f * (temperature_bawah - 25.0f);
    float compensationVoltage = averageVoltage / compensationCoefficient;

    // EC dari polinomial Gravity
    float ecValue = (133.42f * powf(compensationVoltage, 3)
                   - 255.86f * powf(compensationVoltage, 2)
                   + 857.39f * compensationVoltage);
    if (ecValue < 0) ecValue = 0;

    // EC -> TDS (ppm) + kalibrasi
    // tdsValue = ecValue;
    tdsValue = ecValue * TDS_FACTOR;
    // tdsValue = tdsValue * TDS_SCALE + TDS_OFFSET;
    if (tdsValue < 0) tdsValue = 0;

    // --- Debug serial ---
    Serial.println("---------------------------------");
    Serial.printf("pH: %.2f (V=%.3f)\n", phValue, volt_pH);
    Serial.printf("TDS: %.1f ppm | EC: %.1f uS/cm | V=%.3f (comp=%.3f) | T=%.1f°C\n",
                  tdsValue, ecValue, averageVoltage, compensationVoltage, temperature_bawah);

    // 4) Kirim WS
    StaticJsonDocument<512> jsonDoc;
    jsonDoc["deviceId"] = device_id;
    jsonDoc["type"]     = "update_data";
    jsonDoc["room"]     = "environment";
    jsonDoc["broadcast"]= "command";
    JsonObject data     = jsonDoc.createNestedObject("data");
    data["temperatureAtas"]  = temperature_atas;
    data["humidityAtas"]     = humidity_atas;
    data["temperatureBawah"] = temperature_bawah;
    data["humidityBawah"]    = humidity_bawah;
    data["tds"]              = tdsValue;
    data["ph"]               = phValue;

    String payload;
    serializeJson(jsonDoc, payload);
    yield();

    if (isWebsocketConnected) {
      client.send(payload);
      Serial.println("Terkirim: " + payload);
    } else {
      Serial.println("WS belum konek: payload diskip.");
    }
  }
}
