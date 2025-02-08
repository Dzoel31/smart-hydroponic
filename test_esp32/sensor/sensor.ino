#define BLYNK_TEMPLATE_ID "TMPL6bjOLtqP8"
#define BLYNK_TEMPLATE_NAME "Test Blynk"
#define BLYNK_AUTH_TOKEN "iaRSzo-qYbVGMy8nxWS0KDJriA6Rqt5x"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <BlynkSimpleEsp32.h>

#define moisturePin1 32 // BIRU 1
#define moisturePin2 33 // HIJAU 2
#define moisturePin3 34 // input only // OREN 3
#define moisturePin4 35 // input only // BIRU 4
#define moisturePin5 36 // HIJAU bawah 5
#define moisturePin6 39 // input only (SM) // COKELAT 6

#define waterflowPin 16 // OREN
#define triggerPin 18 // BIRU
#define echoPin 19 

//define sound speed in cm/uS
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

const char* ssid = "FIK-Hotspot";
const char* password = "T4nahairku";
String api_url = "http://172.23.13.248:8000/api/store_data";

BlynkTimer timer;

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup(void) {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Attempting to connect to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("\nConnected to WiFi, IP address:");
  Serial.println(WiFi.localIP());

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  pinMode(waterflowPin, INPUT_PULLUP);
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pulseCount = 0;
  flowRate = 0.0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(waterflowPin), pulseCounter, FALLING);

  timer.setInterval(5000L, sendToServerAndBlynk);
}

void loop(void) {

  currentMillis = millis();

  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    
    // Menghitung volume air dalam liter
    Litres = (flowRate / 60);  
    totalLitres += Litres;  // Tambahkan volume air ke total volume

    // Cetak hasil ke Serial Monitor
    Serial.print("Flow rate: ");
    Serial.print(flowRate);  // Laju aliran dalam liter per menit (L/min)
    Serial.print(" L/min");
    Serial.print("\t");      // Cetak tabulasi
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalLitres);  // Total volume air dalam liter
    Serial.println(" L");
  }
  
  moistureAnalog1 = analogRead(moisturePin1);
  moisture1 = (100 - ((moistureAnalog1 / 4095.0) * 100));
  Serial.print("Moisture1 = ");
  Serial.print(moisture1);
  Serial.println("%");

  moistureAnalog2 = analogRead(moisturePin2);
  moisture2 = (100 - ((moistureAnalog2 / 4095.0) * 100));
  Serial.print("Moisture2 = ");
  Serial.print(moisture2);
  Serial.println("%");
  
  moistureAnalog3 = analogRead(moisturePin3);
  moisture3 = (100 - ((moistureAnalog3 / 4095.0) * 100));
  Serial.print("Moisture3 = ");
  Serial.print(moisture3);
  Serial.println("%");
  
  moistureAnalog4 = analogRead(moisturePin4);
  moisture4 = (100 - ((moistureAnalog4 / 4095.0) * 100));
  Serial.print("Moisture4 = ");
  Serial.print(moisture4);
  Serial.println("%");
  
  moistureAnalog5 = analogRead(moisturePin5);
  moisture5 = (100 - ((moistureAnalog5 / 4095.0) * 100));
  Serial.print("Moisture5 = ");
  Serial.print(moisture5);
  Serial.println("%");
  
  moistureAnalog6 = analogRead(moisturePin6);
  moisture6 = (100 - ((moistureAnalog6 / 4095.0) * 100));
  Serial.print("Moisture6 = ");
  Serial.print(moisture6);
  Serial.println("%");

  moistureAvg = (moisture1 + moisture2 + moisture3 + moisture4 + moisture5 + moisture6) / 6;

  Serial.print("Connecting to: ");
  Serial.println(api_url);

  if (moistureAvg < 60) {
    pump_status = 1;
  } else {
    pump_status = 0;
  }

  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);

  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  distanceCm = duration * SOUND_SPEED/2;

  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);


  HTTPClient http;

  String url = String(api_url);

  if (http.begin(url)) {
    http.addHeader("Content-Type", "application/json");
    StaticJsonDocument<200> doc;

    doc["moisture"] = moistureAvg;
    doc["pump_status"] = pump_status;
    doc["flow_rate"] = flowRate;
    doc["total_litres"] = totalLitres;
    doc["distance"] = distanceCm;
    doc["ph"] = ph;

    String payload;
    serializeJson(doc, payload);

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println(httpResponseCode);
          Serial.println(response);
        } else {
          Serial.print("Error on sending POST: ");
          Serial.println(httpResponseCode);
        }

    http.end();
  } else {
    Serial.println("Error on HTTP request");
  }
  delay(5000);
}