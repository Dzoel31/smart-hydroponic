#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include "DHT.h"


#define TdsSensorPin A0
#define VREF 3.3
#define SCOUNT 30
#define DHT11_PIN 14  
#define DHT22_PIN 26
#define DHT11_TYPE DHT11
#define DHT22_TYPE DHT22


int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 0;

const char *ssid = "Podcast Area";
const char *password = "iriunwebcam";
const char *websocket_server = "ws://172.23.0.188:10000/environmentdata";

using namespace websockets;
WebsocketsClient client;

DHT dht11(DHT11_PIN, DHT11_TYPE);
DHT dht22(DHT22_PIN, DHT22_TYPE);

int getMedianNum(int bArray[], int iFilterLen)
{
    int bTab[iFilterLen];
    for (byte i = 0; i < iFilterLen; i++)
        bTab[i] = bArray[i];
    int i, j, bTemp;
    for (j = 0; j < iFilterLen - 1; j++)
    {
        for (i = 0; i < iFilterLen - j - 1; i++)
        {
            if (bTab[i] > bTab[i + 1])
            {
                bTemp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = bTemp;
            }
        }
    }
    return (iFilterLen & 1) ? bTab[(iFilterLen - 1) / 2] : (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
}

void checkWiFiConnection()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi disconnected, attempting to reconnect...");

        WiFi.disconnect();
        WiFi.begin(ssid, password);

        unsigned long startAttemptTime = millis();
        const unsigned long wifiTimeout = 10000; // Timeout 10 detik

        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout)
        {
            Serial.println("Reconnecting...");
            delay(500);
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("Reconnected to WiFi!");
        }
        else
        {
            Serial.println("Failed to reconnect. Will retry later.");
        }
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(TdsSensorPin, INPUT);
    dht11.begin();
    dht22.begin();
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print('.');
    }
    Serial.println("\nConnected to WiFi");

    bool connected = client.connect(websocket_server);
    if (connected)
    {
        Serial.println("Connected to WebSocket Server");
    }
    else
    {
        Serial.println("Failed to connect");
    }
}

void loop()
{
    if (client.available())
    {
        static unsigned long analogSampleTimepoint = millis();
        if (millis() - analogSampleTimepoint > 40U)
        {
            analogSampleTimepoint = millis();
            analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
            analogBufferIndex = (analogBufferIndex + 1) % SCOUNT;
        }

        float temperature_atas = dht11.readTemperature();
        float humidity_atas = dht11.readHumidity();
        float temperature_bawah = dht22.readTemperature();
        float humidity_bawah = dht22.readHumidity();

        if (isnan(temperature_atas) || isnan(humidity_atas) || isnan(temperature_bawah) || isnan(humidity_bawah))
        {
            Serial.println("Gagal membaca sensor DHT11 dan DHT22!");
            return;
        }

        // Baca sensor TDS
        averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * VREF / 4095.0;
        float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
        float compensationVoltage = averageVoltage / compensationCoefficient;
        tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage);

        StaticJsonDocument<256> jsonDoc;
        jsonDoc["temperature_atas"] = temperature_atas;
        jsonDoc["humidity_atas"] = humidity_atas;
        jsonDoc["temperature_bawah"] = temperature_bawah;
        jsonDoc["humidity_bawah"] = humidity_bawah;
        jsonDoc["tdsValue"] = tdsValue;

        String data;
        serializeJson(jsonDoc, data);

        client.send(data);
        Serial.println("Sent: " + data);

    } else {
        Serial.println("Client not available");
        Serial.println("WebSocket disconnected, attempting to reconnect...");
        while (!client.connect(websocket_server))
        { // Loop hingga berhasil connect
            Serial.println("Reconnection failed, retrying...");
            delay(1000);
        }
        Serial.println("Reconnected to WebSocket Server");
    }

    checkWiFiConnection();

    client.poll();
    delay(10000);
}