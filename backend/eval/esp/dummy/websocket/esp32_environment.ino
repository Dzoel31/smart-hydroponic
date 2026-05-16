#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>

using namespace websockets;

const char *WIFI_SSID = "YOUR SSID";
const char *WIFI_PASSWORD = "YOUR WIFI PASSWORD";
const char *WS_SERVER_URL = "ws://192.168.1.10:5000/ws/environment"; // contoh: "ws://
const char *DEVICE_ID = "esp32-environment-device";

const unsigned long SEND_INTERVAL = 5000;
const unsigned long WIFI_TIMEOUT = 15000;
const unsigned long WIFI_RETRY_INTERVAL = 5000;
const unsigned long WS_RETRY_INTERVAL = 5000;

float temperature_atas = 27.0f;
float humidity_atas = 62.0f;
float temperature_bawah = 26.0f;
float humidity_bawah = 66.0f;
float tdsValue = 720.0f;
float phValue = 6.3f;

unsigned long lastSendTime = 0;
unsigned long lastWifiAttempt = 0;
unsigned long lastWsAttempt = 0;

WebsocketsClient client;
bool isWebsocketConnected = false;

void connectWifiInitial();
void reconnectServicesNonBlocking();
void onWebsocketEvent(WebsocketsEvent event, String data);
void generateDummyEnvironment();
void sendSensorData();

void setup()
{
    Serial.begin(115200);
    delay(300);
    randomSeed((uint32_t)esp_random());

    client.onEvent(onWebsocketEvent);
    connectWifiInitial();
    lastSendTime = millis();
}

void loop()
{
    unsigned long now = millis();

    client.poll();
    reconnectServicesNonBlocking();

    if (now - lastSendTime >= SEND_INTERVAL)
    {
        generateDummyEnvironment();

        if (isWebsocketConnected)
        {
            sendSensorData();
        }
        else
        {
            Serial.println("[WS] Not connected, skip send");
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

void onWebsocketEvent(WebsocketsEvent event, String)
{
    if (event == WebsocketsEvent::ConnectionOpened)
    {
        isWebsocketConnected = true;
        Serial.println("[WS] Connected");

        StaticJsonDocument<256> reg;
        reg["deviceId"] = DEVICE_ID;
        reg["type"] = "join";
        reg["room"] = "environment";

        String payload;
        serializeJson(reg, payload);
        client.send(payload);
    }
    else if (event == WebsocketsEvent::ConnectionClosed)
    {
        isWebsocketConnected = false;
        lastWsAttempt = millis();
        Serial.println("[WS] Disconnected");
    }
}

void reconnectServicesNonBlocking()
{
    unsigned long now = millis();

    if (WiFi.status() != WL_CONNECTED)
    {
        if (now - lastWifiAttempt >= WIFI_RETRY_INTERVAL)
        {
            lastWifiAttempt = now;
            Serial.println("[WiFi] Retry...");
            WiFi.reconnect();
        }
        return;
    }

    if (!isWebsocketConnected && now - lastWsAttempt >= WS_RETRY_INTERVAL)
    {
        lastWsAttempt = now;
        Serial.println("[WS] Retry connect...");
        bool ok = client.connect(WS_SERVER_URL);
        if (!ok)
        {
            Serial.println("[WS] Connect failed");
        }
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
    StaticJsonDocument<512> json;
    json["deviceId"] = DEVICE_ID;
    json["type"] = "update_data";
    json["room"] = "environment";
    json["broadcast"] = "command";

    JsonObject data = json.createNestedObject("data");
    data["temperatureAtas"] = temperature_atas;
    data["humidityAtas"] = humidity_atas;
    data["temperatureBawah"] = temperature_bawah;
    data["humidityBawah"] = humidity_bawah;
    data["tds"] = tdsValue;
    data["ph"] = phValue;

    String payload;
    serializeJson(json, payload);
    client.send(payload);
    Serial.println(String("[SEND] ") + payload);
}
