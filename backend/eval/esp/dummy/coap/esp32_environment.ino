#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

const char *WIFI_SSID = "YOUR SSID";
const char *WIFI_PASSWORD = "YOUR WIFI PASSWORD";
const char *DEVICE_ID = "esp32-environment-device";

const unsigned long SEND_INTERVAL = 5000;
const unsigned long WIFI_TIMEOUT = 15000;
const unsigned long WIFI_RETRY_INTERVAL = 5000;

float temperature_atas = 27.0f;
float humidity_atas = 62.0f;
float temperature_bawah = 26.0f;
float humidity_bawah = 66.0f;
float tdsValue = 720.0f;
float phValue = 6.3f;

unsigned long lastSendTime = 0;
unsigned long lastWifiAttempt = 0;

WiFiUDP udp;
Coap coap(udp, 512);
IPAddress coapServerIp(192, 168, 1, 10);
const uint16_t coapServerPort = 5683;
const char *coapPath = "environment";

void connectWifiInitial();
void checkConnections();
void generateDummyEnvironment();
void sendSensorData();

void setup()
{
    Serial.begin(115200);
    delay(300);
    randomSeed((uint32_t)esp_random());
    connectWifiInitial();
    lastSendTime = millis();
}

void loop()
{
    unsigned long now = millis();
    checkConnections();

    if (now - lastSendTime >= SEND_INTERVAL)
    {
        generateDummyEnvironment();

        if (WiFi.status() == WL_CONNECTED)
        {
            sendSensorData();
        }
        else
        {
            Serial.println("[CoAP] WiFi not connected, skip send");
        }

        lastSendTime = now;
    }
}

void connectWifiInitial()
{
    Serial.printf("Connecting WiFi SSID: %s\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT)
    {
        delay(250);
        Serial.print('.');
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.printf("\nWiFi OK: %s\n", WiFi.localIP().toString().c_str());
    }
    else
    {
        Serial.println("\nWiFi init failed, will retry in loop");
    }
}

void checkConnections()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    unsigned long now = millis();
    if (now - lastWifiAttempt >= WIFI_RETRY_INTERVAL)
    {
        lastWifiAttempt = now;
        Serial.println("[WiFi] Retry...");
        WiFi.reconnect();
    }
}

void generateDummyEnvironment()
{
    temperature_atas += ((float)random(-12, 13)) * 0.08f;
    temperature_bawah += ((float)random(-12, 13)) * 0.08f;
    humidity_atas += ((float)random(-20, 21)) * 0.25f;
    humidity_bawah += ((float)random(-20, 21)) * 0.25f;
    tdsValue += ((float)random(-35, 36)) * 1.0f;
    phValue += ((float)random(-10, 11)) * 0.02f;

    if (temperature_atas < 20.0f)
        temperature_atas = 20.0f;
    if (temperature_atas > 35.0f)
        temperature_atas = 35.0f;

    if (temperature_bawah < 20.0f)
        temperature_bawah = 20.0f;
    if (temperature_bawah > 35.0f)
        temperature_bawah = 35.0f;

    if (humidity_atas < 35.0f)
        humidity_atas = 35.0f;
    if (humidity_atas > 90.0f)
        humidity_atas = 90.0f;

    if (humidity_bawah < 35.0f)
        humidity_bawah = 35.0f;
    if (humidity_bawah > 90.0f)
        humidity_bawah = 90.0f;

    if (tdsValue < 250.0f)
        tdsValue = 250.0f;
    if (tdsValue > 1300.0f)
        tdsValue = 1300.0f;

    if (phValue < 5.0f)
        phValue = 5.0f;
    if (phValue > 8.2f)
        phValue = 8.2f;
}

void sendSensorData()
{
    StaticJsonDocument<384> json;
    json["device_id"] = DEVICE_ID;
    json["temperature_atas"] = temperature_atas;
    json["humidity_atas"] = humidity_atas;
    json["temperature_bawah"] = temperature_bawah;
    json["humidity_bawah"] = humidity_bawah;
    json["tds"] = tdsValue;
    json["ph"] = phValue;

    String payload;
    serializeJson(json, payload);

    coap.post(coapServerIp, coapServerPort, coapPath, payload.c_str());
    Serial.println(String("[SEND] ") + payload);
}
