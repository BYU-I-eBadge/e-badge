#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// ============================================================
// BYU-I eBadge v4.1.1 AUG26
// Diagnostic #3 - Board Peripheral Test
// ============================================================

// -------------------- Pin assignments ------------------------

constexpr uint8_t STATUS_LED = 6;

constexpr uint8_t RGB_R = 2;
constexpr uint8_t RGB_G = 4;
constexpr uint8_t RGB_B = 5;

constexpr uint8_t PIXEL_PIN   = 7;
constexpr uint16_t PIXEL_COUNT = 24;

constexpr uint8_t BUZZER_PIN = 48;

constexpr uint8_t BTN_LEFT  = 21;
constexpr uint8_t BTN_RIGHT = 10;
constexpr uint8_t BTN_DOWN  = 47;
constexpr uint8_t BTN_UP    = 11;
constexpr uint8_t BTN_B     = 33;
constexpr uint8_t BTN_A     = 34;

constexpr uint8_t JOY_X = 8;
constexpr uint8_t JOY_Y = 9;

constexpr uint8_t I2C_SDA = 41;
constexpr uint8_t I2C_SCL = 42;

constexpr uint8_t MMA8452_ADDR   = 0x1C;
constexpr uint8_t MMA8452_WHOAMI = 0x0D;

// -------------------------------------------------------------

Adafruit_NeoPixel pixels(
    PIXEL_COUNT,
    PIXEL_PIN,
    NEO_GRB + NEO_KHZ800
);

// Button description
struct ButtonInfo
{
    const char* name;
    uint8_t pin;
    int idleState;
    int lastState;
};

ButtonInfo buttons[] =
{
    {"LEFT ", BTN_LEFT,  0, 0},
    {"RIGHT", BTN_RIGHT, 0, 0},
    {"DOWN ", BTN_DOWN,  0, 0},
    {"UP   ", BTN_UP,    0, 0},
    {"B    ", BTN_B,     0, 0},
    {"A    ", BTN_A,     0, 0}
};

constexpr size_t BUTTON_COUNT =
sizeof(buttons) / sizeof(buttons[0]);

unsigned long lastJoystickReport = 0;

// ============================================================
// Utility
// ============================================================

void separator()
{
    Serial.println(
        "------------------------------------------------------------"
    );
}

void rgbOff()
{
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
}

void pixelsOff()
{
    pixels.clear();
    pixels.show();
}

// ============================================================
// Status LED
// ============================================================

void testStatusLed()
{
    Serial.println();
    separator();
    Serial.println("STATUS LED TEST - GPIO6");
    separator();

    for (int i = 0; i < 4; i++)
    {
        digitalWrite(STATUS_LED, HIGH);
        delay(250);

        digitalWrite(STATUS_LED, LOW);
        delay(250);
    }

    Serial.println("GPIO6 toggle test complete.");
}

// ============================================================
// Discrete RGB LED
// ============================================================

void testRgb()
{
    Serial.println();
    separator();
    Serial.println("DISCRETE RGB TEST");
    separator();

    rgbOff();

    Serial.println("RED");
    digitalWrite(RGB_R, HIGH);
    delay(800);
    rgbOff();

    Serial.println("GREEN");
    digitalWrite(RGB_G, HIGH);
    delay(800);
    rgbOff();

    Serial.println("BLUE");
    digitalWrite(RGB_B, HIGH);
    delay(800);
    rgbOff();

    Serial.println("WHITE / all channels");
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, HIGH);
    delay(800);

    rgbOff();

    Serial.println("RGB test complete.");
}

// ============================================================
// SK6805 addressable LEDs
// ============================================================

void testPixels()
{
    Serial.println();
    separator();
    Serial.println("24x SK6805 ADDRESSABLE LED TEST - GPIO7");
    separator();

    pixelsOff();

    Serial.println("Walking RED pixel...");

    for (uint16_t i = 0; i < PIXEL_COUNT; i++)
    {
        pixels.clear();
        pixels.setPixelColor(i, pixels.Color(40, 0, 0));
        pixels.show();
        delay(100);
    }

    Serial.println("All RED");
    pixels.fill(pixels.Color(30, 0, 0));
    pixels.show();
    delay(700);

    Serial.println("All GREEN");
    pixels.fill(pixels.Color(0, 30, 0));
    pixels.show();
    delay(700);

    Serial.println("All BLUE");
    pixels.fill(pixels.Color(0, 0, 30));
    pixels.show();
    delay(700);

    Serial.println("All WHITE");
    pixels.fill(pixels.Color(20, 20, 20));
    pixels.show();
    delay(700);

    pixelsOff();

    Serial.println("SK6805 test complete.");
}

// ============================================================
// Buzzer
// ============================================================

void testBuzzer()
{
    Serial.println();
    separator();
    Serial.println("BUZZER TEST - GPIO48");
    separator();

    Serial.println("3500 Hz");
    tone(BUZZER_PIN, 3500);
    delay(500);
    noTone(BUZZER_PIN);

    delay(200);

    Serial.println("4000 Hz - nominal resonance");
    tone(BUZZER_PIN, 4000);
    delay(1000);
    noTone(BUZZER_PIN);

    delay(200);

    Serial.println("4500 Hz");
    tone(BUZZER_PIN, 4500);
    delay(500);
    noTone(BUZZER_PIN);

    Serial.println("Buzzer test complete.");
}

// ============================================================
// Buttons
// ============================================================

void captureButtonIdleStates()
{
    Serial.println();
    separator();
    Serial.println("BUTTON BASELINE");
    separator();

    Serial.println(
        "Do not press any application buttons while baseline is captured."
    );

    delay(500);

    for (size_t i = 0; i < BUTTON_COUNT; i++)
    {
        buttons[i].idleState = digitalRead(buttons[i].pin);
        buttons[i].lastState = buttons[i].idleState;

        Serial.printf(
            "%s GPIO%-2u idle=%d\n",
            buttons[i].name,
            buttons[i].pin,
            buttons[i].idleState
        );
    }

    Serial.println();
    Serial.println(
        "Diagnostic detects a press as a CHANGE from the measured idle state."
    );
}

void reportButtons()
{
    for (size_t i = 0; i < BUTTON_COUNT; i++)
    {
        int state = digitalRead(buttons[i].pin);

        if (state != buttons[i].lastState)
        {
            delay(15); // simple debounce
            state = digitalRead(buttons[i].pin);

            if (state != buttons[i].lastState)
            {
                bool pressed =
                state != buttons[i].idleState;

                Serial.printf(
                    "BUTTON %-5s GPIO%-2u : %s (raw=%d)\n",
                              buttons[i].name,
                              buttons[i].pin,
                              pressed ? "PRESSED" : "RELEASED",
                              state
                );

                buttons[i].lastState = state;
            }
        }
    }
}

// ============================================================
// Joystick
// ============================================================

void reportJoystick()
{
    if (millis() - lastJoystickReport < 500)
        return;

    lastJoystickReport = millis();

    int x = analogRead(JOY_X);
    int y = analogRead(JOY_Y);

    Serial.printf(
        "JOYSTICK X=%4d  Y=%4d\n",
        x,
        y
    );
}

// ============================================================
// I2C
// ============================================================

void scanI2C()
{
    Serial.println();
    separator();
    Serial.println("I2C BUS SCAN");
    separator();

    int devices = 0;

    for (uint8_t addr = 1; addr < 127; addr++)
    {
        Wire.beginTransmission(addr);
        uint8_t result = Wire.endTransmission();

        if (result == 0)
        {
            Serial.printf(
                "Found device at 0x%02X",
                addr
            );

            if (addr == MMA8452_ADDR)
                Serial.print("  <-- MMA8452 expected address");

            Serial.println();

            devices++;
        }
    }

    if (devices == 0)
        Serial.println("No I2C devices detected.");
    else
        Serial.printf("%d I2C device(s) detected.\n", devices);
}

// ============================================================
// MMA8452
// ============================================================

bool readRegister(
    uint8_t address,
    uint8_t reg,
    uint8_t &value
)
{
    Wire.beginTransmission(address);
    Wire.write(reg);

    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(address, (uint8_t)1) != 1)
        return false;

    value = Wire.read();

    return true;
}

void testAccelerometer()
{
    Serial.println();
    separator();
    Serial.println("MMA8452 ACCELEROMETER TEST");
    separator();

    uint8_t whoAmI = 0;

    if (!readRegister(
        MMA8452_ADDR,
        MMA8452_WHOAMI,
        whoAmI))
    {
        Serial.println(
            "FAIL: Could not read MMA8452 WHO_AM_I register."
        );

        return;
    }

    Serial.printf(
        "WHO_AM_I = 0x%02X\n",
        whoAmI
    );

    if (whoAmI == 0x2A)
    {
        Serial.println(
            "MMA8452 identification: PASS"
        );
    }
    else
    {
        Serial.println(
            "MMA8452 identification: UNEXPECTED VALUE"
        );
    }
}

// ============================================================
// Information / menu
// ============================================================

void printHelp()
{
    Serial.println();
    separator();
    Serial.println("BYU-I eBadge v4.1.1 Diagnostic #3");
    Serial.println("Interactive Board Peripheral Test");
    separator();

    Serial.println("Commands:");
    Serial.println("  h  - show this help");
    Serial.println("  s  - GPIO6 status LED test");
    Serial.println("  r  - discrete RGB LED test");
    Serial.println("  n  - 24x SK6805 LED test");
    Serial.println("  z  - buzzer test");
    Serial.println("  i  - scan I2C bus");
    Serial.println("  m  - test MMA8452 WHO_AM_I");
    Serial.println("  b  - print current button states");
    Serial.println("  j  - print joystick once");
    Serial.println("  a  - run output/peripheral tests");
    Serial.println();
    Serial.println(
        "Buttons are monitored continuously."
    );

    Serial.println(
        "Joystick values are printed every 500 ms."
    );

    separator();
}

void printButtonStates()
{
    Serial.println();

    for (size_t i = 0; i < BUTTON_COUNT; i++)
    {
        Serial.printf(
            "%s GPIO%-2u raw=%d idle=%d\n",
            buttons[i].name,
            buttons[i].pin,
            digitalRead(buttons[i].pin),
                      buttons[i].idleState
        );
    }
}

void printJoystickOnce()
{
    Serial.printf(
        "JOYSTICK X=%d Y=%d\n",
        analogRead(JOY_X),
                  analogRead(JOY_Y)
    );
}

void runPeripheralTests()
{
    testStatusLed();
    testRgb();
    testPixels();
    testBuzzer();
    scanI2C();
    testAccelerometer();

    Serial.println();
    separator();
    Serial.println("Automatic peripheral test complete.");
    separator();
}

// ============================================================
// Setup
// ============================================================

void setup()
{
    Serial.begin(115200);
    delay(1200);

    pinMode(STATUS_LED, OUTPUT);

    pinMode(RGB_R, OUTPUT);
    pinMode(RGB_G, OUTPUT);
    pinMode(RGB_B, OUTPUT);

    pinMode(BUZZER_PIN, OUTPUT);

    // v4.1.1 has board-side button biasing.
    // Do not override it with internal pull-ups.
    pinMode(BTN_LEFT, INPUT);
    pinMode(BTN_RIGHT, INPUT);
    pinMode(BTN_DOWN, INPUT);
    pinMode(BTN_UP, INPUT);
    pinMode(BTN_B, INPUT);
    pinMode(BTN_A, INPUT);

    pinMode(JOY_X, INPUT);
    pinMode(JOY_Y, INPUT);

    digitalWrite(STATUS_LED, LOW);
    rgbOff();

    pixels.begin();
    pixels.setBrightness(32);
    pixelsOff();

    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);

    Serial.println();
    Serial.println(
        "============================================================"
    );

    Serial.println(
        " BYU-I eBadge v4.1.1 AUG26 - Diagnostic #3"
    );

    Serial.println(
        " Board Peripheral Diagnostic"
    );

    Serial.println(
        "============================================================"
    );

    Serial.printf(
        "Chip:  %s\n",
        ESP.getChipModel()
    );

    Serial.printf(
        "Flash: %u bytes\n",
        ESP.getFlashChipSize()
    );

    Serial.printf(
        "PSRAM: %u bytes\n",
        ESP.getPsramSize()
    );

    captureButtonIdleStates();

    scanI2C();
    testAccelerometer();

    printHelp();
}

// ============================================================
// Main loop
// ============================================================

void loop()
{
    reportButtons();
    reportJoystick();

    if (Serial.available())
    {
        char command = Serial.read();

        switch (command)
        {
            case 'h':
            case 'H':
                printHelp();
                break;

            case 's':
            case 'S':
                testStatusLed();
                break;

            case 'r':
            case 'R':
                testRgb();
                break;

            case 'n':
            case 'N':
                testPixels();
                break;

            case 'z':
            case 'Z':
                testBuzzer();
                break;

            case 'i':
            case 'I':
                scanI2C();
                break;

            case 'm':
            case 'M':
                testAccelerometer();
                break;

            case 'b':
            case 'B':
                printButtonStates();
                break;

            case 'j':
            case 'J':
                printJoystickOnce();
                break;

            case 'a':
            case 'A':
                runPeripheralTests();
                break;
        }
    }

    delay(5);
}
