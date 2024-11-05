#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Ultrasonic Pin Section
#define TRIG_PIN 14 // Sensor Ultrasonik
#define ECHO_PIN 13 // Sensor Ultrasonik

#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

long duration;
float distanceCm;
float distanceInch;

void setup() {
  Serial.begin(9600);
  
  // Initialize the OLED object
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the display buffer
  display.clearDisplay();

  // Set ultrasonic sensor pin modes
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // Read distance from ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Measure the duration of the echo pulse
  duration = pulseIn(ECHO_PIN, HIGH);
  distanceCm = duration * SOUND_VELOCITY / 2;
  distanceInch = distanceCm * CM_TO_INCH;

  // Display the distance on the OLED screen
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("Jarak (cm): ");
  display.println(distanceCm);
  display.display();

  // Print the distance to Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  // Delay before the next reading
  delay(1000);
}