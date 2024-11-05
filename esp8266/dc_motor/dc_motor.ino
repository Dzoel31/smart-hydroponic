/*
 * This ESP8266 NodeMCU code was developed by newbiely.com
 *
 * This ESP8266 NodeMCU code is made available for public use without any restriction
 *
 * For comprehensive instructions and wiring diagrams, please visit:
 * https://newbiely.com/tutorials/esp8266/esp8266-dc-motor
 */

#define PIN_ENA  14 // The ESP8266 pin connected to the EN1 pin L298N
#define PIN_IN1  12 // The ESP8266 pin connected to the IN1 pin L298N
#define PIN_IN2  13 // The ESP8266 pin connected to the IN2 pin L298N

// The setup function runs once on reset or power-up
void setup() {
  // initialize digital pins as outputs.
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);
}

// The loop function repeats indefinitely
void loop() {
  digitalWrite(PIN_IN1, HIGH); // control the motor's direction in clockwise
  digitalWrite(PIN_IN2, LOW);  // control the motor's direction in clockwise

  for (int speed = 0; speed <= 255; speed++) {
    analogWrite(PIN_ENA, speed); // speed up
    delay(10);
  }

  delay(10000); // rotate at maximum speed 2 seconds in clockwise direction

  // change direction
  digitalWrite(PIN_IN1, LOW);   // control the motor's direction in anti-clockwise
  digitalWrite(PIN_IN2, HIGH);  // control the motor's direction in anti-clockwise

  delay(10000); // rotate at maximum speed for 2 seconds in anti-clockwise direction

  for (int speed = 255; speed >= 0; speed--) {
    analogWrite(PIN_ENA, speed); // speed down
    delay(10);
  }

  delay(4000); // stop motor 2 second
}
