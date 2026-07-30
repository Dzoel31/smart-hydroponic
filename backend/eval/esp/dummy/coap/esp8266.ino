#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

// Pin Definitions
#define RELAY_PUMP_1 4
#define RELAY_PUMP_2 5
#define RELAY_LIGHT_1 12
#define RELAY_LIGHT_2 14

// Configuration
const char *WIFI_SSID = "YOUR SSID";
const char *WIFI_PASSWORD = "YOUR WIFI PASSWORD";
const char *DEVICE_ID = "esp8266-actuator-device";
const unsigned long DATA_SEND_INTERVAL = 5000;
const unsigned long WIFI_RECONNECT_TIMEOUT = 10000;
const float MOISTURE_THRESHOLD = 60;
const float TEMPERATURE_THRESHOLD = 30.0;

// CoAP Configuration
IPAddress coapServerIp(192, 168, 1, 10);
const uint16_t coapServerPort = 5683;
const char *coapPathStatus = "actuator/status";
const char *coapPathControl = "actuator/control";

// State variables
struct ActuatorState
{
    int pump_status = 0;
    int light_status = 0;
    int automation_status = 0;
    float moisture_avg = 0;
    float temperature_avg = 0;
} state;

WiFiUDP udp;
Coap coap(udp, 512);

// Function prototypes
void connectToWifi();
void handleCoapMessage(const char *payload);
void updateRelays();
void sendStatusUpdateCoap();
void checkConnections();
void coapCallback(CoapPacket &packet, IPAddress ip, int port);
void generateDummyInputs();

void setup()
{
    Serial.begin(115200);
    randomSeed(micros());

    // Initialize relay pins
    for (int pin : {RELAY_PUMP_1, RELAY_PUMP_2, RELAY_LIGHT_1, RELAY_LIGHT_2})
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH); // OFF initially
    }

    connectToWifi();
    coap.server(coapCallback, coapPathControl);
    coap.start(coapServerPort);
}

void loop()
{
    checkConnections();
    coap.loop();

    static unsigned long lastSendTime = 0;
    if (millis() - lastSendTime >= DATA_SEND_INTERVAL)
    {
        generateDummyInputs();
        sendStatusUpdateCoap();
        lastSendTime = millis();
    }
}

void generateDummyInputs()
{
    state.moisture_avg += ((float)random(-20, 21)) * 0.25f;
    state.temperature_avg += ((float)random(-12, 13)) * 0.12f;

    if (state.moisture_avg < 35.0f)
        state.moisture_avg = 35.0f;
    if (state.moisture_avg > 90.0f)
        state.moisture_avg = 90.0f;

    if (state.temperature_avg < 20.0f)
        state.temperature_avg = 20.0f;
    if (state.temperature_avg > 40.0f)
        state.temperature_avg = 40.0f;

}

void connectToWifi()
{
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
    }
    else
    {
        Serial.println("\nFailed to connect");
    }
}

void coapCallback(CoapPacket &packet, IPAddress ip, int port)
{
    Serial.println("CoAP message received from " + ip.toString());

    char payload[packet.payloadlen + 1];
    memcpy(payload, packet.payload, packet.payloadlen);
    payload[packet.payloadlen] = '\0';

    handleCoapMessage(payload);

    // Send ACK
    coap.sendResponse(ip, port, packet.messageid);
}

void handleCoapMessage(const char *payload)
{
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Serial.println("JSON parse error");
        return;
    }

    // Update automation status
    if (doc.containsKey("automation_status"))
    {
        state.automation_status = doc["automation_status"];
    }

    // Update sensor values
    if (doc.containsKey("moisture_avg"))
    {
        state.moisture_avg = doc["moisture_avg"];
    }
    if (doc.containsKey("avg_temperature"))
    {
        state.temperature_avg = doc["avg_temperature"];
    }

    handleManualMode(doc);

    updateRelays();
}

void handleAutomaticMode()
{
    if (!isnan(state.moisture_avg))
    {
        state.pump_status = (state.moisture_avg < MOISTURE_THRESHOLD) ? 1 : 0;
    }
    if (!isnan(state.temperature_avg))
    {
        state.light_status = (state.temperature_avg < TEMPERATURE_THRESHOLD) ? 1 : 0;
    }
}

void handleManualMode(JsonVariant doc)
{
    if (doc.containsKey("pump_status"))
    {
        state.pump_status = doc["pump_status"];
    }
    if (doc.containsKey("light_status"))
    {
        state.light_status = doc["light_status"];
    }
}

void updateRelays()
{
    digitalWrite(RELAY_PUMP_1, state.pump_status ? LOW : HIGH);
    digitalWrite(RELAY_PUMP_2, state.pump_status ? LOW : HIGH);
    digitalWrite(RELAY_LIGHT_1, state.light_status ? LOW : HIGH);
    digitalWrite(RELAY_LIGHT_2, state.light_status ? LOW : HIGH);
}

void sendStatusUpdateCoap()
{
    StaticJsonDocument<256> doc;
    doc["pump_status"] = state.pump_status;
    doc["light_status"] = state.light_status;
    doc["automation_status"] = state.automation_status;
    doc["moisture_avg"] = state.moisture_avg;
    doc["avg_temperature"] = state.temperature_avg;
    doc["device_id"] = DEVICE_ID;

    String payload;
    serializeJson(doc, payload);

    coap.post(coapServerIp, coapServerPort, coapPathStatus, (char *)payload.c_str());
}

void checkConnections()
{
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck >= WIFI_RECONNECT_TIMEOUT)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Reconnecting WiFi...");
            connectToWifi();
        }
        lastCheck = millis();
    }
}
