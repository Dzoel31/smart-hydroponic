/*
 * This ESP8266 NodeMCU code was developed by newbiely.com
 *
 * This ESP8266 NodeMCU code is made available for public use without any restriction
 *
 * For comprehensive instructions and wiring diagrams, please visit:
 * https://newbiely.com/tutorials/esp8266/esp8266-2-channel-relay-module
 */

#define PIN_RELAY_2  12 // The ESP8266 pin connected to the IN1 pin of relay module
// The ESP8266 pin connected to the IN2 pin of relay module
const int trigPin = 14;
const int echoPin = 13;

long duration;
float distanceCm;
float distanceInch;

#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

// The setup function runs once on reset or power-up
void setup() {
  Serial.begin(115200);

  // Configure the ESP8266 pin as an digital output.
  pinMode(PIN_RELAY_2, OUTPUT);
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input
}

// The loop function repeats indefinitely
void loop() {

  // Pembacaan dari sensor ultrasonik
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_VELOCITY / 2;
  distanceInch = distanceCm * CM_TO_INCH;

  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);

  if (distanceCm < 5) {
    // Menyalakan motor jika jarak kurang dari 5 cm
    Serial.println("Jarak terdeteksi di bawah 5 cm, mengaktifkan motor DC");
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);

    for (int speed = 0; speed <= 255; speed++) {
      analogWrite(PIN_ENA, speed);
      delay(10);
    }

    delay(2000);  // Motor berputar selama 2 detik
    
    // Matikan motor setelah 2 detik
    for (int speed = 255; speed >= 0; speed--) {
      analogWrite(PIN_ENA, speed);
      delay(10);
    }

    // Aktifkan relay setelah motor berhenti
    Serial.println("Mengaktifkan relay untuk menghidupkan pompa");
    digitalWrite(PIN_RELAY_2, LOW);  // Relay aktif
    delay(1000);  // Relay aktif selama 1 detik
    digitalWrite(PIN_RELAY_2, HIGH); // Matikan relay
  } else {
    Serial.println("Jarak lebih dari 5 cm, motor dan relay tidak aktif");
  }
  delay(500);
}