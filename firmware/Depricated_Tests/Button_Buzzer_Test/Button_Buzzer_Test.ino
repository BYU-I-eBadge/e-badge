// Define the pin connected to the passive piezo buzzer.
// You can change this to any available GPIO pin on your ESP32-S3.
const int buzzerPin = 42; // Example: Using GPIO 42

// Define the pins for the 6 buttons.
// Connect each button between the specified GPIO pin and GND.
// Internal pull-up resistors will be enabled.
const int buttonPin1 = 13;
const int buttonPin2 = 14;
const int buttonPin3 = 15;
const int button4Pin = 16; // Using a different naming convention for clarity
const int button5Pin = 17;
const int button6Pin = 18;

// Define the notes and their frequencies (in Hz)
// These are common musical notes, you can add more or change them.
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_REST 0 // Represents a pause

unsigned long lastDebounceTime[6] = {0, 0, 0, 0, 0, 0}; // Last time the output pin was toggled
unsigned long debounceDelay = 10; // Debounce time in milliseconds

void setup() {
  // Initialize the serial communication for debugging.
  Serial.begin(115200);
  Serial.println("ESP32-S3 Passive Piezo Buzzer with Button Control");

  // Set the buzzer pin as an output.
  pinMode(buzzerPin, OUTPUT);


  // Set button pins as inputs with internal pull-up resistors.
  // This means the pin will read HIGH when the button is not pressed,
  // and LOW when the button is pressed (connected to GND).
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(button4Pin, INPUT_PULLUP);
  pinMode(button5Pin, INPUT_PULLUP);
  pinMode(button6Pin, INPUT_PULLUP);
}

void loop() {
  // Read the state of each button
  int buttonState1 = digitalRead(buttonPin1);
  int buttonState2 = digitalRead(buttonPin2);
  int buttonState3 = digitalRead(buttonPin3);
  int buttonState4 = digitalRead(button4Pin);
  int buttonState5 = digitalRead(button5Pin);
  int buttonState6 = digitalRead(button6Pin);

  // Variable to keep track if any button is currently pressed
  bool anyButtonPressed = false;

  // Check button 1
  if (buttonState1 != LOW) { // Button is pressed (pull-up means LOW when pressed)
    if (millis() - lastDebounceTime[0] > debounceDelay) {
      Serial.println("Button 1 pressed - Playing C4");
      tone(buzzerPin, NOTE_C4);
      anyButtonPressed = true;
      lastDebounceTime[0] = millis(); // Update debounce time
    }
  } else {
    lastDebounceTime[0] = 0; // Reset debounce timer when button is released
  }

  // Check button 2
  if (buttonState2 != LOW) {
    if (millis() - lastDebounceTime[1] > debounceDelay) {
      Serial.println("Button 2 pressed - Playing D4");
      tone(buzzerPin, NOTE_D4);
      anyButtonPressed = true;
      lastDebounceTime[1] = millis();
    }
  } else {
    lastDebounceTime[1] = 0;
  }

  // Check button 3
  if (buttonState3 != LOW) {
    if (millis() - lastDebounceTime[2] > debounceDelay) {
      Serial.println("Button 3 pressed - Playing E4");
      tone(buzzerPin, NOTE_E4);
      anyButtonPressed = true;
      lastDebounceTime[2] = millis();
    }
  } else {
    lastDebounceTime[2] = 0;
  }

  // Check button 4
  if (buttonState4 != LOW) {
    if (millis() - lastDebounceTime[3] > debounceDelay) {
      Serial.println("Button 4 pressed - Playing F4");
      tone(buzzerPin, NOTE_F4);
      anyButtonPressed = true;
      lastDebounceTime[3] = millis();
    }
  } else {
    lastDebounceTime[3] = 0;
  }

  // Check button 5
  if (buttonState5 != LOW) {
    if (millis() - lastDebounceTime[4] > debounceDelay) {
      Serial.println("Button 5 pressed - Playing G4");
      tone(buzzerPin, NOTE_G4);
      anyButtonPressed = true;
      lastDebounceTime[4] = millis();
    }
  } else {
    lastDebounceTime[4] = 0;
  }

  // Check button 6
  if (buttonState6 != LOW) {
    if (millis() - lastDebounceTime[5] > debounceDelay) {
      Serial.println("Button 6 pressed - Playing A4");
      tone(buzzerPin, NOTE_A4);
      anyButtonPressed = true;
      lastDebounceTime[5] = millis();
    }
  } else {
    lastDebounceTime[5] = 0;
  }

  // If no button is currently pressed, stop the tone
  if (!anyButtonPressed) {
    noTone(buzzerPin);
  }

  // A small delay to allow the ESP32 to perform other tasks and for stable readings
  delay(500);
}

