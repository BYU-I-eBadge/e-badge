// This sketch demonstrates how to blink an LED connected to an ESP32-S3 board.

// Define the GPIO pin where the LED is connected.
// For many ESP32-S3 development boards, GPIO 2 is often connected to an onboard LED.
// If you are using an external LED, connect its anode (long leg) to this pin
// through a current-limiting resistor (e.g., 220-1000 Ohm) and its cathode (short leg) to GND.
const int rled = 38;
const int gled = 39;
const int bled = 40;
const int ledPin = 41; // Change this to your desired GPIO pin if different
const int A = 18;
const int B = 17;
const int Up = 16;
const int Down = 15;
const int Left = 14;
const int Right = 13;
// The setup function runs once when the ESP32-S3 starts up or resets.
void setup() {
  // Initialize the serial communication for debugging purposes.
  // This allows you to see messages printed to the Serial Monitor in the Arduino IDE.
  Serial.begin(115200);
  Serial.println("ESP32-S3 LED Blinker Starting...");

  // Set the LED pin as an OUTPUT.
  // This configures the pin to send voltage (HIGH) or no voltage (LOW) to the LED.
  pinMode(ledPin, OUTPUT);
  pinMode(rled, OUTPUT);
  pinMode(gled, OUTPUT);
  pinMode(bled, OUTPUT);
  pinMode(A, INPUT);
  pinMode(B, INPUT);
  pinMode(Up, INPUT);
  pinMode(Down, INPUT);
  pinMode(Left, INPUT);
  pinMode(Right, INPUT);
  digitalWrite(ledPin,LOW);
  digitalWrite(rled,LOW);
  digitalWrite(gled,LOW);
  digitalWrite(bled,LOW);

}

// The loop function runs repeatedly after the setup function completes.
void loop() {
  // Turn the LED on (HIGH voltage).

  // Wait for another 1000 milliseconds (1 second).
  int readingA = analogRead(A);
  int readingB = analogRead(B);
  int readingUp = analogRead(Up);
  int readingDown = analogRead(Down);
  int readingLeft = analogRead(Left);
  int readingRight = analogRead(Right);
  Serial.println("A");
  Serial.println(readingA);
  Serial.println("B");
  Serial.println(readingB);
  Serial.println("Up");
  Serial.println(readingUp);
  Serial.println("Down");
  Serial.println(readingDown);
  Serial.println("Left");
  Serial.println(readingLeft);
  Serial.println("Right");
  Serial.println(readingRight);
  if (readingA != 0){
    digitalWrite(rled,HIGH);
    delay(500);
  }
  else {
    digitalWrite(rled,LOW);
  }
  if (readingB != 0){
    digitalWrite(gled,HIGH);
    delay(500);
  }
  else {
    digitalWrite(gled,LOW);
  }
  if (readingUp != 0){
    digitalWrite(bled,HIGH);
    delay(500);
  }
  else {
    digitalWrite(bled,LOW);
  }
  if (readingDown != 0){
    digitalWrite(ledPin,HIGH);
    delay(500);
  }
  else {
    digitalWrite(ledPin,LOW);
  }
  if (readingLeft != 0){
    digitalWrite(bled,HIGH);
    delay(500);
  }
  else {
    digitalWrite(bled,LOW);
  }
  if (readingRight != 0){
    digitalWrite(bled,HIGH);
    delay(500);
  }
  else {
    digitalWrite(bled,LOW);
  }

  delay(1000);
}

