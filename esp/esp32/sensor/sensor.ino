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

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;

long duration;
float distanceCm;
float distanceInch;

const char *ssid = "FIK-Dekanat";
const char *password = "F4silkom";
const char *websocket_server = "ws://192.168.1.16:10000";
const char *type_sensor = "plant_ESP32";

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

	pulseCount = 0;
	flowRate = 0.0;
	previousMillis = 0;

	attachInterrupt(digitalPinToInterrupt(waterflowPin), pulseCounter, FALLING);
}

void loop()
{
	if (client.available()) {

		currentMillis = millis();
	
		if (currentMillis - previousMillis > interval)
		{
			pulse1Sec = pulseCount;
			pulseCount = 0;
			
	
			flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
			previousMillis = millis();
	
			// Menghitung volume air dalam liter
			Litres = (flowRate / 60);
			totalLitres += Litres; // Tambahkan volume air ke total volume
	
		}
	
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
	
	
		digitalWrite(triggerPin, LOW);
		delayMicroseconds(2);
	
		digitalWrite(triggerPin, HIGH);
		delayMicroseconds(10);
		digitalWrite(triggerPin, LOW);
	
		duration = pulseIn(echoPin, HIGH);
	
		distanceCm = duration * SOUND_SPEED / 2;

		// Create JSON payload
		StaticJsonDocument<256> jsonDoc;
		jsonDoc["type"] = type_sensor;
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

	} else {
		Serial.println("Client not available");
	}
	totalLitres = 0;
	client.poll(); // Check for incoming messages
	delay(3000);   // Send data every 5 seconds
}