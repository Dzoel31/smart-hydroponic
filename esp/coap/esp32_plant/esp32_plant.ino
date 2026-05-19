#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

// ============================ PIN DEFINITIONS ============================
#define MOISTURE_PIN1 32
#define MOISTURE_PIN2 33
#define MOISTURE_PIN3 34
#define MOISTURE_PIN4 35
#define MOISTURE_PIN5 36
#define MOISTURE_PIN6 39

#define WATERFLOW_PIN 16
#define TRIGGER_PIN 18
#define ECHO_PIN 19

// ============================ CONSTANTS ============================
#define SOUND_SPEED 0.034f
#define JARAK_SENSOR_KE_DASAR 43.0f
#define FLOW_CALIBRATION_FACTOR 4.5f
#define DAY_IN_MS 86400000UL

// ============================ NETWORK ============================
const char *WIFI_SSID = "FIK-Hotspot";
const char *WIFI_PASSWORD = "T4nahairku";

const char *DEVICE_ID = "esp32-plant-device";

IPAddress coapServerIp(172, 25, 21, 236);
const uint16_t coapServerPort = 8683;
const char *coapPath = "coap/hydroponics/plant";

// ============================ INTERVALS ============================
const unsigned long FLOW_INTERVAL = 1000;
const unsigned long ULTRASONIC_INTERVAL = 1000;
const unsigned long SEND_INTERVAL = 30000;
const unsigned long WIFI_RETRY_INTERVAL = 5000;

// ============================ GLOBALS ============================
int moisture[6];
int moistureAnalog[6];

float flowRate = 0.0f;
float totalLitres = 0.0f;
float waterLevel = 0.0f;
int seq = 1;
unsigned long send_time = 0;
int last_seq_sent = 0;

volatile uint32_t pulseCount = 0;

unsigned long lastFlowCheck = 0;
unsigned long lastUltrasonicCheck = 0;
unsigned long lastSendTime = 0;
unsigned long lastDailyReset = 0;
unsigned long lastWifiAttempt = 0;

bool wifiConnected = false;
bool coapStarted = false;

// ============================ COAP ============================
WiFiUDP udp;
Coap coap(udp, 512);

// ============================ ISR ============================
void IRAM_ATTR pulseCounter()
{
    pulseCount++;
}

// ============================ FORWARD DECLS ============================
void connectWifi();
void checkWiFi();
void startCoap();
void stopCoap();

void readMoistureSensors();
void readWaterLevel();
void readWaterFlow();

void sendSensorData();
void resetDailyCounters();

void callback_response(CoapPacket &packet, IPAddress ip, int port);

// ============================ SETUP ============================
void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("=== ESP32 PLANT SENSOR STARTING ===");

    // ADC
    analogReadResolution(12);

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

    digitalWrite(TRIGGER_PIN, LOW);

    // Interrupt
    attachInterrupt(
        digitalPinToInterrupt(WATERFLOW_PIN),
        pulseCounter,
        FALLING);

    // CoAP callback
    coap.response(callback_response);

    // WiFi connect
    connectWifi();

    // Start CoAP ONLY after WiFi connected
    if (WiFi.status() == WL_CONNECTED)
    {
        startCoap();
    }

    unsigned long now = millis();

    lastFlowCheck = now;
    lastUltrasonicCheck = now;
    lastSendTime = now;
    lastDailyReset = now;

    Serial.println("Setup complete");
}

// ============================ LOOP ============================
void loop()
{
    unsigned long now = millis();

    yield();

    // WiFi management
    checkWiFi();

    // CoAP loop ONLY when connected
    if (wifiConnected && coapStarted)
    {
        coap.loop();
    }

    // Water flow
    if (now - lastFlowCheck >= FLOW_INTERVAL)
    {
        readWaterFlow();

        lastFlowCheck = now;
    }

    // Ultrasonic + moisture
    if (now - lastUltrasonicCheck >= ULTRASONIC_INTERVAL)
    {
        readWaterLevel();

        readMoistureSensors();

        lastUltrasonicCheck = now;
    }

    // Send sensor data
    if (now - lastSendTime >= SEND_INTERVAL)
    {
        sendSensorData();

        lastSendTime = now;
    }

    // Daily reset
    if (now - lastDailyReset >= DAY_IN_MS)
    {
        resetDailyCounters();

        lastDailyReset = now;
    }

    delay(10);
}

// ============================ WIFI ============================
void connectWifi()
{
    Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);

    WiFi.mode(WIFI_STA);

    WiFi.setAutoReconnect(true);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startAttempt = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startAttempt < 10000)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiConnected = true;

        Serial.println();
        Serial.println("[WiFi] Connected!");

        Serial.print("[WiFi] IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        wifiConnected = false;

        Serial.println();
        Serial.println("[WiFi] Failed to connect");
    }
}

void checkWiFi()
{
    unsigned long now = millis();

    if (WiFi.status() != WL_CONNECTED)
    {
        if (wifiConnected)
        {
            Serial.println("[WiFi] Lost connection");

            wifiConnected = false;

            stopCoap();
        }

        if (now - lastWifiAttempt >= WIFI_RETRY_INTERVAL)
        {
            lastWifiAttempt = now;

            Serial.println("[WiFi] Reconnecting...");

            WiFi.disconnect();

            delay(100);

            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
    }
    else
    {
        if (!wifiConnected)
        {
            wifiConnected = true;

            Serial.println("[WiFi] Reconnected!");

            startCoap();
        }
    }
}

// ============================ COAP ============================
void startCoap()
{
    if (coapStarted)
        return;

    udp.begin(5683);

    coap.start();

    coapStarted = true;

    Serial.println("[CoAP] Started");
}

void stopCoap()
{
    if (!coapStarted)
        return;

    udp.stop();

    coapStarted = false;

    Serial.println("[CoAP] Stopped");
}

// ============================ SENSORS ============================
void readMoistureSensors()
{
    const int pins[6] = {
        MOISTURE_PIN1,
        MOISTURE_PIN2,
        MOISTURE_PIN3,
        MOISTURE_PIN4,
        MOISTURE_PIN5,
        MOISTURE_PIN6};

    for (int i = 0; i < 6; i++)
    {
        moistureAnalog[i] = analogRead(pins[i]);

        moisture[i] = 100 -
                      int((moistureAnalog[i] / 4095.0f) * 100.0f);

        moisture[i] = constrain(moisture[i], 0, 100);

        delayMicroseconds(10);
    }
}

void readWaterLevel()
{
    digitalWrite(TRIGGER_PIN, LOW);

    delayMicroseconds(2);

    digitalWrite(TRIGGER_PIN, HIGH);

    delayMicroseconds(10);

    digitalWrite(TRIGGER_PIN, LOW);

    unsigned long duration =
        pulseInLong(ECHO_PIN, HIGH, 30000UL);

    if (duration == 0)
    {
        Serial.println("[Ultrasonic] Timeout");

        return;
    }

    float distance =
        (duration * SOUND_SPEED) / 2.0f;

    waterLevel =
        JARAK_SENSOR_KE_DASAR - distance;

    waterLevel =
        constrain(
            waterLevel,
            0,
            JARAK_SENSOR_KE_DASAR);
}

void readWaterFlow()
{
    noInterrupts();

    uint32_t pulses = pulseCount;

    pulseCount = 0;

    interrupts();

    flowRate =
        pulses / FLOW_CALIBRATION_FACTOR;

    float litersPerSecond =
        flowRate / 60.0f;

    totalLitres += litersPerSecond;
}

// ============================ SEND ============================
void sendSensorData()
{
    if (!wifiConnected || !coapStarted)
    {
        Serial.println("[CoAP] Skip send");

        return;
    }

    StaticJsonDocument<512> json;

    json["seq"] = seq;
    json["device_id"] = DEVICE_ID;

    json["moisture1"] = moisture[0];
    json["moisture2"] = moisture[1];
    json["moisture3"] = moisture[2];
    json["moisture4"] = moisture[3];
    json["moisture5"] = moisture[4];
    json["moisture6"] = moisture[5];

    json["flowrate"] = flowRate;
    json["total_litres"] = totalLitres;

    json["water_level"] = waterLevel;

    json["timestamp"] = millis();

    String payload;

    serializeJson(json, payload);

    last_seq_sent = seq;
    send_time = millis();

    Serial.println("[CoAP] Sending...");
    Serial.println(payload);

    uint16_t messageId =
        coap.put(
            coapServerIp,
            coapServerPort,
            coapPath,
            payload.c_str(),
            payload.length());

    if (messageId > 0)
    {
        Serial.printf(
            "[CoAP] Sent OK (MID=%u)\n",
            messageId);
        seq++;
    }
    else
    {
        Serial.println("[CoAP] Send failed");
    }
}

// ============================ COAP CALLBACK ============================
void callback_response(
    CoapPacket &packet,
    IPAddress ip,
    int port)
{
    Serial.println("[CoAP] Response received");

    if (last_seq_sent > 0)
    {
        unsigned long latency = millis() - send_time;
        Serial.printf("[METRIC] Seq: %d | Latency: %lu ms\n", last_seq_sent, latency);
    }

    if (packet.payloadlen > 0)
    {
        char payload[packet.payloadlen + 1];

        memcpy(
            payload,
            packet.payload,
            packet.payloadlen);

        payload[packet.payloadlen] = '\0';

        Serial.print("[CoAP] Payload: ");

        Serial.println(payload);
    }
}

// ============================ RESET ============================
void resetDailyCounters()
{
    totalLitres = 0.0f;

    Serial.println("[RESET] Daily counters reset");
}