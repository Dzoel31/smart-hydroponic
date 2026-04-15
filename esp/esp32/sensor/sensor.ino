#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>

// ============================ PIN DEFINITIONS ============================
// Moisture sensors (ADC1-only pins, aman bareng WiFi)
#define MOISTURE_PIN1 32 // BIRU 1
#define MOISTURE_PIN2 33 // HIJAU 2
#define MOISTURE_PIN3 34 // input-only // OREN 3
#define MOISTURE_PIN4 35 // input-only // BIRU 4
#define MOISTURE_PIN5 36 // HIJAU bawah 5
#define MOISTURE_PIN6 39 // input-only (SM) // COKELAT 6

// Water flow and ultrasonic sensor
#define WATERFLOW_PIN 16 // OREN
#define TRIGGER_PIN   18 // BIRU
#define ECHO_PIN      19

// ============================ CONSTANTS ============================
#define SOUND_SPEED 0.034f              // cm/us
#define JARAK_SENSOR_KE_DASAR 43.0f     // cm
#define FLOW_CALIBRATION_FACTOR 4.5f    // pulses per L/min (contoh, sesuaikan sensormu)
#define DAY_IN_MS 86400000UL

// ============================ NETWORK ============================
const char *WIFI_SSID      = "Podcast Area"; // <-- isi
const char *WIFI_PASSWORD  = "iriunwebcam"; // <-- isi
const char *WS_SERVER_URL  = "ws://103.147.92.179"; // contoh: "ws://192.168.1.10:3000/ws"
const char *DEVICE_ID      = "esp32-plant-device";

// ============================ INTERVALS ============================
const unsigned long FLOW_INTERVAL        = 1000;   // 1s
const unsigned long ULTRASONIC_INTERVAL  = 500;    // 0.5s
const unsigned long SEND_INTERVAL        = 5000;   // 5s
const unsigned long WIFI_TIMEOUT         = 10000;  // 10s (hanya dipakai di setup awal)

// Retry cooldowns (non-blocking)
const unsigned long WIFI_RETRY_INTERVAL  = 5000;   // 5s
const unsigned long WS_RETRY_INTERVAL    = 5000;   // 5s

// ============================ GLOBALS ============================
// Moisture sensor readings
int moisture[6] = {0, 0, 0, 0, 0, 0};
int moistureAnalog[6] = {0, 0, 0, 0, 0, 0};

// Water measurements
float flowRate = 0.0f;      // L/min
float totalLitres = 0.0f;   // L (akumulasi)
volatile int pulseCount = 0;
float waterLevel = 0.0f;    // cm (dari dasar naik)

// Timing
unsigned long lastFlowCheck = 0;
unsigned long lastUltrasonicCheck = 0;
unsigned long lastSendTime = 0;
unsigned long lastDailyReset = 0;

unsigned long lastWifiAttempt = 0;
unsigned long lastWsAttempt   = 0;

// WebSocket
using namespace websockets;
WebsocketsClient client;
bool isWsConnected = false;

// ============================ ISR ============================
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

// ============================ FORWARD DECLS ============================
void connectWifiInitial();
void readMoistureSensors();
void readWaterLevel();
void readWaterFlow();
void sendSensorData();
void resetDailyCounters();
void checkConnections();

// ============================ SETUP ============================
void setup() {
  Serial.begin(115200);
  delay(200);

  // ADC setup (12-bit default; pins already ADC1 range)
  analogReadResolution(12); // 0..4095
  // (Optional) set attenuation for moisture pins to improve linearity
  analogSetPinAttenuation(MOISTURE_PIN1, ADC_11db);
  analogSetPinAttenuation(MOISTURE_PIN2, ADC_11db);
  analogSetPinAttenuation(MOISTURE_PIN3, ADC_11db);
  analogSetPinAttenuation(MOISTURE_PIN4, ADC_11db);
  analogSetPinAttenuation(MOISTURE_PIN5, ADC_11db);
  analogSetPinAttenuation(MOISTURE_PIN6, ADC_11db);

  // Pins
  pinMode(WATERFLOW_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Flow interrupt
  attachInterrupt(digitalPinToInterrupt(WATERFLOW_PIN), pulseCounter, FALLING);

  // WS events
  client.onEvent([](WebsocketsEvent e, String data) {
    if (e == WebsocketsEvent::ConnectionOpened) {
      Serial.println("[WS] Connected");
      isWsConnected = true;

      // Register device
      StaticJsonDocument<256> reg;
      reg["device_id"] = DEVICE_ID;
      reg["type"] = "join";
      reg["room"] = "plant";
      String s; serializeJson(reg, s);
      client.send(s);
    } else if (e == WebsocketsEvent::ConnectionClosed) {
      Serial.println("[WS] Disconnected");
      isWsConnected = false;
      lastWsAttempt = millis(); // debounce
    } else if (e == WebsocketsEvent::GotPing) {
      Serial.println("[WS] Ping");
    } else if (e == WebsocketsEvent::GotPong) {
      Serial.println("[WS] Pong");
    }
  });

  // Initial WiFi connect (boleh blocking sebentar saat boot)
  connectWifiInitial();

  // Initialize schedulers to "now" biar interval langsung konsisten
  unsigned long now = millis();
  lastFlowCheck = now;
  lastUltrasonicCheck = now;
  lastSendTime = now;
  lastDailyReset = now;
}

// ============================ LOOP ============================
void loop() {
  unsigned long now = millis();

  // Keep WS alive
  client.poll();

  // Non-blocking connection manager
  checkConnections();

  // Water flow interval
  if (now - lastFlowCheck >= FLOW_INTERVAL) {
    readWaterFlow();
    lastFlowCheck = now;
  }

  // Ultrasonic + moisture interval
  if (now - lastUltrasonicCheck >= ULTRASONIC_INTERVAL) {
    readWaterLevel();
    readMoistureSensors();
    lastUltrasonicCheck = now;
  }

  // Send interval (jalan terus, tak tergantung available())
  if (now - lastSendTime >= SEND_INTERVAL) {
    if (isWsConnected) {
      sendSensorData();
    } else {
      Serial.println("[WS] Not connected, skip send");
    }
    lastSendTime = now; // selalu update agar anti-burst
  }

  // Daily reset
  if (now - lastDailyReset >= DAY_IN_MS) {
    resetDailyCounters();
    lastDailyReset = now;
  }
}

// ============================ NET HELPERS ============================
void connectWifiInitial() {
  Serial.printf("Connecting WiFi SSID: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT) {
    delay(300);
    Serial.print('.');
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi OK: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWiFi init failed, will retry non-blocking in loop");
  }
}

void checkConnections() {
  unsigned long now = millis();

  // WiFi non-blocking retry
  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWifiAttempt >= WIFI_RETRY_INTERVAL) {
      lastWifiAttempt = now;
      Serial.println("[WiFi] Retry...");
      WiFi.reconnect(); // non-blocking
    }
    return; // jangan coba WS kalau WiFi belum connect
  }

  // WebSocket non-blocking retry
  if (!isWsConnected) {
    if (now - lastWsAttempt >= WS_RETRY_INTERVAL) {
      lastWsAttempt = now;
      Serial.println("[WS] Retry connect...");
      bool ok = client.connect(WS_SERVER_URL); // sync, tapi cepat jika server respon
      if (!ok) {
        Serial.println("[WS] Connect failed");
      }
    }
  }
}

// ============================ SENSORS ============================
void readMoistureSensors() {
  const int pins[6] = {
    MOISTURE_PIN1, MOISTURE_PIN2, MOISTURE_PIN3,
    MOISTURE_PIN4, MOISTURE_PIN5, MOISTURE_PIN6
  };

  for (int i = 0; i < 6; i++) {
    // Baca analog 12-bit -> 0..4095
    moistureAnalog[i] = analogRead(pins[i]);
    // Konversi ke % "basah" sederhana: 0 = basah? tergantung sensor.
    // Formula kasar (sesuaikan kalibrasi sensor kamu):
    moisture[i] = 100 - int((moistureAnalog[i] / 4095.0f) * 100.0f);
    if (moisture[i] < 0) moisture[i] = 0;
    if (moisture[i] > 100) moisture[i] = 100;
  }
}

void readWaterLevel() {
  // Trigger ultrasonik
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  // Timeout 30ms untuk hindari blocking panjang
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL); // microseconds
  if (duration == 0) {
    // no echo; keep previous waterLevel (atau set -1 sebagai invalid)
    // waterLevel = waterLevel;
    return;
  }

  // Jarak = (durasi * kecepatan suara) / 2
  float distance = (duration * SOUND_SPEED) / 2.0f; // cm
  // Level air dari dasar (semakin besar berarti air semakin tinggi)
  waterLevel = JARAK_SENSOR_KE_DASAR - distance;
  if (waterLevel < 0) waterLevel = 0; // clamp
}

void readWaterFlow() {
  // Baca pulse per interval
  noInterrupts();
  int pulses = pulseCount;
  pulseCount = 0;
  interrupts();

  // Konversi pulses -> L/min
  // (Contoh: banyak flow sensors gunakan konstanta p/L, sesuaikan FLOW_CALIBRATION_FACTOR)
  flowRate = (pulses / FLOW_CALIBRATION_FACTOR); // L/min

  // L/s
  float litersPerSecond = flowRate / 60.0f;
  totalLitres += litersPerSecond; // akumulasi per detik (FLOW_INTERVAL=1000ms)
}

// ============================ SENDING ============================
void sendSensorData() {
  StaticJsonDocument<512> json;
  json["deviceId"]  = DEVICE_ID;
  json["type"]      = "update_data";
  json["room"]      = "plant";
  json["broadcast"] = "command";

  JsonObject d = json.createNestedObject("data");
  d["moisture1"]   = moisture[0];
  d["moisture2"]   = moisture[1];
  d["moisture3"]   = moisture[2];
  d["moisture4"]   = moisture[3];
  d["moisture5"]   = moisture[4];
  d["moisture6"]   = moisture[5];
  d["moistureAvg"] = (moisture[0] + moisture[1] + moisture[2] + moisture[3] + moisture[4] + moisture[5]) / 6.0f;
  d["flowrate"]    = flowRate;     // L/min
  d["total_litres"]= totalLitres;  // L
  d["distance_cm"] = waterLevel;   // cm (level dari dasar)

  String s;
  serializeJson(json, s);
  client.send(s);
  Serial.println(String("[SEND] ") + s);
}

// ============================ DAILY RESET ============================
void resetDailyCounters() {
  totalLitres = 0.0f;

  // Jangan reset ke 0; set ke "now" agar interval tetap rapi (anti-burst)
  unsigned long now = millis();
  lastFlowCheck = now;
  lastUltrasonicCheck = now;
  lastSendTime = now;
}
