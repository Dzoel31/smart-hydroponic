#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

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
#define TRIGGER_PIN 18   // BIRU
#define ECHO_PIN 19

// ============================ CONSTANTS ============================
#define SOUND_SPEED 0.034f           // cm/us
#define JARAK_SENSOR_KE_DASAR 43.0f  // cm
#define FLOW_CALIBRATION_FACTOR 4.5f // pulses per L/min (contoh, sesuaikan sensormu)
#define DAY_IN_MS 86400000UL

// ============================ NETWORK ============================
const char *WIFI_SSID = "YOUR SSID";            // <-- isi
const char *WIFI_PASSWORD = "YOUR WIFI PASSWORD";         // <-- isi
const char *DEVICE_ID = "esp32-plant-device";

// ============================ INTERVALS ============================
const unsigned long FLOW_INTERVAL = 1000;      // 1s
const unsigned long ULTRASONIC_INTERVAL = 500; // 0.5s
const unsigned long SEND_INTERVAL = 5000;      // 5s
const unsigned long WIFI_TIMEOUT = 10000;      // 10s (hanya dipakai di setup awal)

// Retry cooldowns (non-blocking)
const unsigned long WIFI_RETRY_INTERVAL = 5000; // 5s
const unsigned long WS_RETRY_INTERVAL = 5000;   // 5s

// ============================ GLOBALS ============================
// Moisture sensor readings
int moisture[6] = {0, 0, 0, 0, 0, 0};
int moistureAnalog[6] = {0, 0, 0, 0, 0, 0};

// Water measurements
float flowRate = 0.0f;    // L/min
float totalLitres = 0.0f; // L (akumulasi)
volatile int pulseCount = 0;
float waterLevel = 0.0f; // cm (dari dasar naik)
float dummyFlowBias = 1.2f;

// Timing
unsigned long lastFlowCheck = 0;
unsigned long lastUltrasonicCheck = 0;
unsigned long lastSendTime = 0;
unsigned long lastDailyReset = 0;

unsigned long lastWifiAttempt = 0;
unsigned long lastWsAttempt = 0;

// CoAP
WiFiUDP udp;
Coap coap(udp, 512);
IPAddress coapServerIp(192, 168, 1, 10); // TODO: ganti sesuai IP server CoAP-mu
const uint16_t coapServerPort = 5683;
const char *coapPath = "plant";

// ============================ ISR ============================
void IRAM_ATTR pulseCounter()
{
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
void setup()
{
    Serial.begin(115200);
    delay(200);
    randomSeed((uint32_t)esp_random());

    // ADC setup (12-bit default; pins already ADC1 range)
    analogReadResolution(12); // 0..4095
    // (Optional) set attenuation for moisture pins to improve linearity
    analogSetPinAttenuation(MOISTURE_PIN1, ADC_11db);
    analogSetPinAttenuation(MOISTURE_PIN2, ADC_11db);
    analogSetPinAttenuation(MOISTURE_PIN3, ADC_11db);
    analogSetPinAttenuation(MOISTURE_PIN4, ADC_11db);
    analogSetPinAttenuation(MOISTURE_PIN5, ADC_11db);
    analogSetPinAttenuation(MOISTURE_PIN6, ADC_11db);

    // Dummy mode: no physical sensor/interrupt setup required
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
void loop()
{
    unsigned long now = millis();

    // Non-blocking connection manager
    checkConnections();

    // Water flow interval
    if (now - lastFlowCheck >= FLOW_INTERVAL)
    {
        readWaterFlow();
        lastFlowCheck = now;
    }

    // Ultrasonic + moisture interval
    if (now - lastUltrasonicCheck >= ULTRASONIC_INTERVAL)
    {
        readWaterLevel();
        readMoistureSensors();
        lastUltrasonicCheck = now;
    }

    // Send interval (jalan terus, tak tergantung available())
    if (now - lastSendTime >= SEND_INTERVAL)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            sendSensorData();
        }
        else
        {
            Serial.println("[CoAP] WiFi not connected, skip send");
        }
        lastSendTime = now; // selalu update agar anti-burst
    }

    // Daily reset
    if (now - lastDailyReset >= DAY_IN_MS)
    {
        resetDailyCounters();
        lastDailyReset = now;
    }
}

// ============================ NET HELPERS ============================
void connectWifiInitial()
{
    Serial.printf("Connecting WiFi SSID: %s\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT)
    {
        delay(300);
        Serial.print('.');
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.printf("\nWiFi OK: %s\n", WiFi.localIP().toString().c_str());
    }
    else
    {
        Serial.println("\nWiFi init failed, will retry non-blocking in loop");
    }
}

void checkConnections()
{
    unsigned long now = millis();

    // WiFi non-blocking retry
    if (WiFi.status() != WL_CONNECTED)
    {
        if (now - lastWifiAttempt >= WIFI_RETRY_INTERVAL)
        {
            lastWifiAttempt = now;
            Serial.println("[WiFi] Retry...");
            WiFi.reconnect(); // non-blocking
        }
        return; // jangan coba WS kalau WiFi belum connect
    }
}

// ============================ SENSORS ============================
void readMoistureSensors()
{
    for (int i = 0; i < 6; i++)
    {
        // Dummy random-walk for stable but changing values
        int delta = random(-3, 4);
        int next = moisture[i] == 0 ? random(45, 76) : moisture[i] + delta;
        if (next < 35)
            next = 35;
        if (next > 90)
            next = 90;

        moisture[i] = next;
        moistureAnalog[i] = (100 - moisture[i]) * 4095 / 100;
        if (moisture[i] < 0)
            moisture[i] = 0;
        if (moisture[i] > 100)
            moisture[i] = 100;
    }
}

void readWaterLevel()
{
    float start = waterLevel <= 0.01f ? 23.0f : waterLevel;
    float delta = ((float)random(-10, 11)) * 0.08f;
    waterLevel = start + delta;
    if (waterLevel < 0)
        waterLevel = 0; // clamp
    if (waterLevel > JARAK_SENSOR_KE_DASAR)
        waterLevel = JARAK_SENSOR_KE_DASAR;
}

void readWaterFlow()
{
    float jitter = ((float)random(-25, 26)) * 0.02f;
    dummyFlowBias += jitter;
    if (dummyFlowBias < 0.2f)
        dummyFlowBias = 0.2f;
    if (dummyFlowBias > 3.5f)
        dummyFlowBias = 3.5f;

    flowRate = dummyFlowBias; // L/min
    totalLitres += flowRate * (FLOW_INTERVAL / 60000.0f);
}

// ============================ SENDING ============================
void sendSensorData()
{
    StaticJsonDocument<512> json;
    json["moisture1"] = moisture[0];
    json["moisture2"] = moisture[1];
    json["moisture3"] = moisture[2];
    json["moisture4"] = moisture[3];
    json["moisture5"] = moisture[4];
    json["moisture6"] = moisture[5];
    json["flowrate"] = flowRate;        // L/min
    json["total_litres"] = totalLitres; // L
    json["distance_cm"] = waterLevel;   // cm (level dari dasar)

    String s;
    serializeJson(json, s);
    yield();

    // Kirim CoAP
    coap.post(coapServerIp, coapServerPort, coapPath, s.c_str());
    Serial.println(String("[SEND] ") + s);
}

// ============================ DAILY RESET ============================
void resetDailyCounters()
{
    totalLitres = 0.0f;

    // Jangan reset ke 0; set ke "now" agar interval tetap rapi (anti-burst)
    unsigned long now = millis();
    lastFlowCheck = now;
    lastUltrasonicCheck = now;
    lastSendTime = now;
}