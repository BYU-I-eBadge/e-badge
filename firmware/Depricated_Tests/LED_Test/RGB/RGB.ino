// --- Configuration ---
// Define the GPIO pins connected to the Red, Green, and Blue channels of the RGB LED.
const int RED_PIN = 38;
const int GREEN_PIN = 39;
const int BLUE_PIN = 40;

// PWM resolution. ESP32's analogWrite can be set to 10-bit resolution.
// 0 means 0% duty cycle (off), 1023 means 10% duty cycle (full on).
// For common cathode, 0 is off, 1023 is full brightness.
const int PWM_RESOLUTION_BITS = 10;
const int PWM_MAX_DUTY = 30; 

// Note: The PWM frequency is set internally by the ESP32's `ledc` peripheral
// when using `analogWrite`. You can configure it more precisely using the
// `ledcSetup`, `ledcAttachPin`, and `ledcWrite` functions if needed,
// but `analogWrite` is simpler for basic use. The default frequency is often around 1kHz.

// --- Helper function to set RGB color ---
void setRgbColor(int r, int g, int b) {
  /*
   * Sets the RGB LED color using PWM duty cycles.
   * Assumes common anode LED (0 = brightest, 1023 = off).
   * For common cathode, simply use the scaled value directly (e.g., duty = value).
   *
   * Args:
   * r (int): Red component brightness (0-255).
   * g (int): Green component brightness (0-255).
   * b (int): Blue component brightness (0-255).
   */

  // Map 0-255 brightness to 0-1023 PWM duty cycle.
  // For common anode, we invert the value:
  // 0 (max brightness) -> 1023 (0% output for common anode)
  // 255 (off) -> 0 (10% output for common anode)
  // The formula `PWM_MAX_DUTY - map(value, 0, 255, 0, PWM_MAX_DUTY)`
  // effectively inverts the 0-255 range and scales it to 0-1023.

  // If you are using a COMMON CATHODE LED, change these lines to:
  // int dutyRed = map(r, 0, 255, 0, PWM_MAX_DUTY);
  // int dutyGreen = map(g, 0, 255, 0, PWM_MAX_DUTY);
  // int dutyBlue = map(b, 0, 255, 0, PWM_MAX_DUTY);

  int dutyRed = map(r, 0, 255, 0, PWM_MAX_DUTY);
  int dutyGreen = map(g, 0, 255, 0, PWM_MAX_DUTY);
  int dutyBlue = map(b, 0, 255, 0, PWM_MAX_DUTY);

  analogWrite(RED_PIN, dutyRed);
  analogWrite(GREEN_PIN, dutyGreen);
  analogWrite(BLUE_PIN, dutyBlue);
}

// --- Setup function (runs once when the ESP32 starts) ---
void setup() {
  Serial.begin(9600); // Initialize serial communication for debugging
  Serial.println("Starting RGB LED demonstration...");

  // Set pin modes for the LED pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Set the PWM resolution for analogWrite.
  // ESP32 supports setting resolution up to 16 bits, but 10 bits is common for LEDs.
  analogWriteResolution(RED_PIN,PWM_RESOLUTION_BITS);
  analogWriteResolution(GREEN_PIN,PWM_RESOLUTION_BITS);
  analogWriteResolution(BLUE_PIN,PWM_RESOLUTION_BITS);
}

// --- Loop function (runs repeatedly after setup) ---
void loop() {
  // Red
  Serial.println("Setting color: Red");
  setRgbColor(55, 0, 0);
  delay(1000); // Wait for 1 second

  // Green
  Serial.println("Setting color: Green");
  setRgbColor(0, 55, 0);
  delay(1000);

  // Blue
  Serial.println("Setting color: Blue");
  setRgbColor(0, 0, 10);
  delay(1000);

  // Yellow (Red + Green)
  Serial.println("Setting color: Yellow");
  setRgbColor(10, 10, 0);
  delay(1000);

  // Cyan (Green + Blue)
  Serial.println("Setting color: Cyan");
  setRgbColor(0, 10, 10);
  delay(1000);

  // Magenta (Red + Blue)
  Serial.println("Setting color: Magenta");
  setRgbColor(10, 0, 10);
  delay(1000);

  // White (All colors)
  Serial.println("Setting color: White");
  setRgbColor(10, 10, 10);
  delay(1000);

  // Off
  Serial.println("Setting color: Off");
  setRgbColor(0, 0, 0);
  delay(1000);

  // Smooth fade through colors (example)
  Serial.println("Fading through colors...");
  for (int j = 0; j <=10; j++){
  for (int i = 0; i <= 255; i++) {
       setRgbColor(i, 255 - i, 0);
    delay(10); // Small delay for smooth transition
  }
  for (int i = 0; i <= 255; i++) {
    setRgbColor(255 - i, 0, i);
    delay(10);
  }
  for (int i = 0; i <= 255; i++) {
    setRgbColor(0, i, 255 - i);
    delay(10);
  }
  }
}
