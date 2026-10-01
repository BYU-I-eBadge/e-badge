// This sketch demonstrates how to blink an LED connected to an ESP32-S3 board.

// Define the GPIO pin where the LED is connected.
// For many ESP32-S3 development boards, GPIO 2 is often connected to an onboard LED.
// If you are using an external LED, connect its anode (long leg) to this pin
// through a current-limiting resistor (e.g., 220-1000 Ohm) and its cathode (short leg) to GND.
const int rled = 38;
const int gled = 39;
const int bled = 40;
const int ledPin = 41; // Change this to your desired GPIO pin if different

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
  pinMode(15, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  digitalWrite(ledPin,LOW);
  digitalWrite(rled,LOW);
  digitalWrite(gled,LOW);
  digitalWrite(bled,LOW);

}

// The loop function runs repeatedly after the setup function completes.
void loop() {
  // Turn the LED on (HIGH voltage).

  // Wait for another 1000 milliseconds (1 second).
  int readingDown = digitalRead(15);
  int readingUp = digitalRead(16);
  Serial.println(readingDown);
  Serial.println(readingUp);
  if (readingDown != 0){
    digitalWrite(bled,HIGH);
  }
  else {
    digitalWrite(bled,LOW);
  }
  if (readingUp != 0){
    digitalWrite(gled,HIGH);
  }
  else{
    digitalWrite(gled,LOW);
  }
  delay(500);
}

