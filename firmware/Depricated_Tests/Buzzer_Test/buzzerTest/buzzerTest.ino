// Define the pin connected to the passive piezo buzzer.
// You can change this to any available GPIO pin on your ESP32-S3.
const int buzzerPin = 42; // Example: Using GPIO 2

void setup() {
  // Initialize the serial communication for debugging.
  Serial.begin(115200);
  Serial.println("ESP32-S3 Passive Piezo Buzzer Test");

  // Set the buzzer pin as an output.
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // Play a tone (e.g., Middle C - 262 Hz) for 500 milliseconds.
  Serial.println("Playing tone: 262 Hz");
  tone(buzzerPin, 262); // Pin, Frequency (Hz)
  delay(500);           // Duration in milliseconds

  // Stop the tone.
  Serial.println("Stopping tone.");
  noTone(buzzerPin);
  delay(250); // Short pause

  // Play another tone (e.g., Middle D - 294 Hz) for 500 milliseconds.
  Serial.println("Playing tone: 294 Hz");
  tone(buzzerPin, 294);
  delay(500);

  // Stop the tone.
  Serial.println("Stopping tone.");
  noTone(buzzerPin);
  delay(250); // Short pause

  // Play a higher tone (e.g., Middle E - 330 Hz) for 750 milliseconds.
  Serial.println("Playing tone: 330 Hz");
  tone(buzzerPin, 330);
  delay(750);

  // Stop the tone.
  Serial.println("Stopping tone.");
  noTone(buzzerPin);
  delay(500); // Longer pause before repeating the sequence
}
