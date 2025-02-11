#define BLYNK_TEMPLATE_ID "TMPL68FSq4d8e"
#define BLYNK_TEMPLATE_NAME "Smart Hidroponik"
#define BLYNK_AUTH_TOKEN "5cmX2SdrFnscq7WZvCXxWuEfxTbkEoHK"

#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include "DHT.h"

#define TdsSensorPin A0
#define VREF 3.3
#define SCOUNT 30
#define DHT_PIN 14
#define DHT_TYPE DHT11

int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 0;

const char *ssid = "FIK-Dekanat";
const char *password = "F4silkom";
const char *websocket_server = "ws://192.168.1.16:10000";
const char *type_sensor = "environment_ESP32";

DHT dht(DHT_PIN, DHT_TYPE);

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

void setup()
{
    Serial.begin(115200);
    pinMode(TdsSensorPin, INPUT);
    dht.begin();
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

        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        if (isnan(temperature) || isnan(humidity))
        {
            Serial.println("Gagal membaca sensor DHT11!");
            return;
        }

        // Baca sensor TDS
        averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * VREF / 4095.0;
        float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
        float compensationVoltage = averageVoltage / compensationCoefficient;
        tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage);

        StaticJsonDocument<256> jsonDoc;
        jsonDoc["type"] = type_sensor;
        jsonDoc["temperature"] = temperature;
        jsonDoc["humidity"] = humidity;
        jsonDoc["tdsValue"] = tdsValue;

        String data;
        serializeJson(jsonDoc, data);

        client.send(data);
        Serial.println("Sent: " + data);

    } else {
        Serial.println("Client not available");
    }

    client.poll();
    delay(3000);
}
