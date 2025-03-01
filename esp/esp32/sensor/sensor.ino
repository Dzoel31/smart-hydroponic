#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>

#define moisturePin1 32 // BIRU 1
#define moisturePin2 33 // HIJAU 2
#define moisturePin3 34 // input only // OREN 3
#define moisturePin4 35 // input only // BIRU 4
#define moisturePin5 36 // HIJAU bawah 5
#define moisturePin6 39 // input only (SM) // COKELAT 6

#define waterflowPin 16 // OREN
#define triggerPin 18	// BIRU
#define echoPin 19

// define sound speed in cm/uS
#define SOUND_SPEED 0.034

#define JARAK_SENSOR_KE_DASAR 40

int moisture1, moistureAnalog1;
int moisture2, moistureAnalog2;
int moisture3, moistureAnalog3;
int moisture4, moistureAnalog4;
int moisture5, moistureAnalog5;
int moisture6, moistureAnalog6;

float moistureAvg;

float flowRate;
float Litres;
float totalLitres = 0;
int pump_status = 0;

int interval = 1000;
const float calibrationFactor = 4.5;
volatile int pulseCount = 0;

unsigned long lastFlowCheck = 0;
unsigned long lastUltrasonicCheck = 0;
unsigned long lastSendTime = 0;

const unsigned long flowInterval = 1000;
const unsigned long ultrasonicInterval = 500;
const unsigned long sendInterval = 10000;

long duration;
float distanceCm;
float distanceInch;

const char *ssid = "FIK-Hotspot";
const char *password = "T4nahairku";
const char *websocket_server = "ws://172.23.0.188:10000/plantdata";

using namespace websockets;
WebsocketsClient client;

void IRAM_ATTR pulseCounter()
{
	pulseCount++;
}

void setup(void)
{
	Serial.begin(115200);

	WiFi.begin(ssid, password);
	Serial.print("Attempting to connect to ");
	Serial.print(ssid);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(1000);
		Serial.println("Connecting to WiFi...");
	}
	Serial.println("\nConnected to WiFi, IP address:");
	Serial.println(WiFi.localIP());

	bool connected = client.connect(websocket_server);
	if (connected)
	{
		Serial.println("Connected to WebSocket Server");
	}
	else
	{
		Serial.println("Failed to connect");
	}

	pinMode(waterflowPin, INPUT_PULLUP);
	pinMode(triggerPin, OUTPUT);
	pinMode(echoPin, INPUT);

	attachInterrupt(digitalPinToInterrupt(waterflowPin), pulseCounter, FALLING);
}

float getWaterLevel() {
	digitalWrite(triggerPin, LOW);
	delayMicroseconds(2);

	digitalWrite(triggerPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(triggerPin, LOW);

	duration = pulseIn(echoPin, HIGH);

	distanceCm = JARAK_SENSOR_KE_DASAR - (duration * SOUND_SPEED / 2);
	return distanceCm;
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
			delay(500); // Tunggu 500ms antara percobaan
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

void loop()
{
	if (client.available()) {
		unsigned long currentMillis = millis();
		if (currentMillis - lastFlowCheck >= flowInterval)
		{
			detachInterrupt(waterflowPin);
	
			// flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
			flowRate = (pulseCount / calibrationFactor);
			float litersPerSecond = flowRate / 60;
			totalLitres += litersPerSecond;
			pulseCount = 0;
			
			lastFlowCheck = currentMillis;

			attachInterrupt(digitalPinToInterrupt(waterflowPin), pulseCounter, FALLING);
		}

		if (currentMillis - lastUltrasonicCheck >= ultrasonicInterval)
		{
			distanceCm = getWaterLevel();
			lastUltrasonicCheck = currentMillis;
			
			moistureAnalog1 = analogRead(moisturePin1);
			moisture1 = (100 - ((moistureAnalog1 / 4095.0) * 100));
	
			moistureAnalog2 = analogRead(moisturePin2);
			moisture2 = (100 - ((moistureAnalog2 / 4095.0) * 100));
	
			moistureAnalog3 = analogRead(moisturePin3);
			moisture3 = (100 - ((moistureAnalog3 / 4095.0) * 100));
	
			moistureAnalog4 = analogRead(moisturePin4);
			moisture4 = (100 - ((moistureAnalog4 / 4095.0) * 100));
	
			moistureAnalog5 = analogRead(moisturePin5);
			moisture5 = (100 - ((moistureAnalog5 / 4095.0) * 100));
	
			moistureAnalog6 = analogRead(moisturePin6);
			moisture6 = (100 - ((moistureAnalog6 / 4095.0) * 100));

			moistureAvg = (moisture1 + moisture2 + moisture3 + moisture4 + moisture5 + moisture6) / 6;
		}

		if (currentMillis - lastSendTime < sendInterval)
		{
			// Create JSON payload
			StaticJsonDocument<256> jsonDoc;
			jsonDoc["moisture1"] = moisture1;
			jsonDoc["moisture2"] = moisture2;
			jsonDoc["moisture3"] = moisture3;
			jsonDoc["moisture4"] = moisture4;
			jsonDoc["moisture5"] = moisture5;
			jsonDoc["moisture6"] = moisture6;
			jsonDoc["moistureAvg"] = moistureAvg;
			jsonDoc["flowRate"] = flowRate;
			jsonDoc["totalLitres"] = totalLitres;
			jsonDoc["distanceCm"] = distanceCm;

			String data;
			serializeJson(jsonDoc, data);

			// Send data to WebSocket server
			client.send(data);
			Serial.println("Sent: " + data);
		}

		if (currentMillis >= 86400000) {
			totalLitres = 0;
			lastFlowCheck = 0;
			lastUltrasonicCheck = 0;
			lastSendTime = 0;
		}
		
		client.poll(); // Check for incoming messages
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
}