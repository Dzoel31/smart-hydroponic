#include <DHT.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>

// Pin Definitions
// Ultrasonic Pin Section
#define TRIG_PIN 19 // Sensor Ultrasonik
#define ECHO_PIN 18 // Sensor Ultrasonik

// Water Flow Pin Section
#define FLOW_SENSOR 34           // Pin untuk sensor aliran air (flow sensor)


// Motor DC Pin Section
#define PIN_ENA  25 // The ESP8266 pin connected to the EN1 pin L298N
#define PIN_IN1  17 // The ESP8266 pin connected to the IN1 pin L298N
#define PIN_IN2  16 // The ESP8266 pin connected to the IN2 pin L298N

#define PIN_RELAY_1 32 // 
#define PIN_RELAY_2  27 // The ESP8266 pin connected to the IN2 pin of relay module

// Konstanta
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;
float moisture_percentage;

// Water flow variables
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;

float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;

float flowRate;
float Litres;
float totalLitres = 0;  // Inisialisasi totalLitres dari 0

int statusMotor = 0;
int statusWaterPump = 0;

const char* ssid = "Digital_Library";
const char* password = "puska2023$";

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup() {
  // Inisialisasi Serial
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(TRIG_PIN, OUTPUT);  // Sets the trigPin as an Output
  pinMode(ECHO_PIN, INPUT);   // Sets the echoPin as an Input
  pinMode(FLOW_SENSOR, INPUT_PULLUP); // Pin untuk water flow sensor
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_RELAY_1, OUTPUT);
  pinMode(PIN_RELAY_2, OUTPUT);
  pulseCount = 0;
  flowRate = 0.0;
  previousMillis = 0;

  // Atur interrupt untuk sensor aliran air
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR), pulseCounter, FALLING);
}
  
void activateMotor() {
  statusMotor = 1;
  Serial.println("Jarak terdeteksi di bawah 5 cm, mengaktifkan motor DC");
  digitalWrite(PIN_RELAY_1, HIGH);
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);

  // Gradually increase motor speed
  for (int speed = 0; speed <= 255; speed++) {
    analogWrite(PIN_ENA, speed);
    delay(10);
  }

  delay(10000);  // Motor berputar selama 10 detik

  // Gradually decrease motor speed to stop
  for (int speed = 255; speed >= 0; speed--) {
    analogWrite(PIN_ENA, speed);
    delay(10);
  }

  statusMotor = 0;
  Serial.println("Motor berhenti");
}

void activatePump() {
  if (statusMotor == 0) {  // Ensure pump only activates after motor has stopped
    Serial.println("Mengaktifkan relay untuk menghidupkan pompa");
    digitalWrite(PIN_RELAY_2, LOW);  // Activate relay (pump on)
    delay(8000);                     // Pump runs for 8 seconds
    digitalWrite(PIN_RELAY_2, HIGH); // Deactivate relay (pump off)
    Serial.println("Pompa berhenti");
  } else {
    Serial.println("Menunggu motor berhenti sebelum menghidupkan pompa");
  }
}

void checkDistanceAndOperate() {
  if (distanceCm < 5) {
    activateMotor();  // Start motor if distance is below threshold
    activatePump();   // Start pump after motor stops
  } else {
    Serial.println("Jarak lebih dari 5 cm, motor dan relay tidak aktif");
    digitalWrite(PIN_RELAY_1, LOW);
  }
}

void loop() {

  // Pembacaan dari sensor ultrasonik
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);
  distanceCm = duration * SOUND_VELOCITY / 2;
  distanceInch = distanceCm * CM_TO_INCH;

  // Menampilkan jarak pada Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);

// // Clear the buffer.
//   display.clearDisplay();
//   display.setTextSize(2);
//   display.setTextColor(WHITE);
//   display.setCursor(0, 20);
//   display.print("Jarak (cm): ");
//   display.println(distanceCm);
//   display.display();

  checkDistanceAndOperate();

  // // Pembacaan sensor kelembapan tanah
  // moisture_percentage = (100.00 - ((analogRead(SENSOR_SOIL) / 1023.00) * 100.00));
  // Serial.print("Soil Moisture(in Percentage) = ");
  // Serial.print(moisture_percentage);
  // Serial.println("%");

  // Water flow sensor logic
  currentMillis = millis();
  
  // Mengecek setiap 1 detik
  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;  // Simpan jumlah pulsa per detik
    pulseCount = 0;          // Reset hitungan pulsa untuk pengukuran berikutnya
    
    // Hitung laju aliran (liter per menit)
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

    // Menyalakan LED jika aliran air terdeteksi
    // if (flowRate > 0) {
    //   digitalWrite(LED_PIN, HIGH);  // Menyalakan LED
    //   delay(500);
    //   digitalWrite(LED_PIN, LOW);   // Mematikan LED
    //   delay(500);
    // }
    
  }
  // Delay sebelum pengulangan loop
  delay(1000);
}