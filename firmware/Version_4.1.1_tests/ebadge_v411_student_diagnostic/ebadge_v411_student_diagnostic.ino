/*
  BYU-I eBadge v4.1.1 AUG26
  Standalone Interactive Hardware Diagnostic - Responsive UI v2

  Navigation:
    UP / DOWN : select menu item
    A         : run selected test
    B         : exit current test and return to menu

  Hidden easter egg:
    UP, UP, DOWN, DOWN, LEFT, RIGHT, LEFT, RIGHT, B, A

  Target:
    ESP32-S3-MINI-1-N4R2
    Arduino-ESP32 3.3.12
*/

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_NeoPixel.h>

// ============================================================
// eBadge v4.1.1 pin map
// ============================================================

// Display - ST7789, 240x320
constexpr uint8_t TFT_CS   = 0;
constexpr uint8_t TFT_RST  = 1;
constexpr uint8_t TFT_DC   = 45;
constexpr uint8_t TFT_MOSI = 3;
constexpr uint8_t TFT_SCLK = 46;

// LEDs / buzzer
constexpr uint8_t STATUS_LED = 6;
constexpr uint8_t PIXEL_PIN = 7;
constexpr uint16_t PIXEL_COUNT = 24;

constexpr uint8_t RGB_R = 2;
constexpr uint8_t RGB_G = 4;
constexpr uint8_t RGB_B = 5;

constexpr uint8_t BUZZER_PIN = 48;

// Buttons
constexpr uint8_t BTN_LEFT  = 21;
constexpr uint8_t BTN_RIGHT = 10;
constexpr uint8_t BTN_DOWN  = 47;
constexpr uint8_t BTN_UP    = 11;
constexpr uint8_t BTN_B     = 33;
constexpr uint8_t BTN_A     = 34;

// v4.1.1 button circuits use board-side pull-downs.
// A pressed button therefore reads HIGH.
constexpr uint8_t BUTTON_ACTIVE_LEVEL = HIGH;

// Joystick
constexpr uint8_t JOY_X = 8;
constexpr uint8_t JOY_Y = 9;

// I2C
constexpr uint8_t I2C_SDA = 41;
constexpr uint8_t I2C_SCL = 42;
constexpr uint8_t MMA8452_ADDR = 0x1C;

// SD card / SPI3
constexpr uint8_t SD_MISO = 37;
constexpr uint8_t SD_MOSI = 39;
constexpr uint8_t SD_SCLK = 38;
constexpr uint8_t SD_CS   = 40;

// ============================================================
// Display / LEDs
// ============================================================

// Use the ESP32-S3's two general-purpose hardware SPI controllers.
// FSPI drives the display on the v4.1.1 display pins.
// HSPI is reserved for the SD-card/SPI3 test.
// This keeps the two buses electrically separate without slow software SPI.
SPIClass displaySPI(FSPI);
SPIClass sdSPI(HSPI);

Adafruit_ST7789 tft(
  &displaySPI,
  TFT_CS,
  TFT_DC,
  TFT_RST
);

Adafruit_NeoPixel pixels(
  PIXEL_COUNT,
  PIXEL_PIN,
  NEO_GRB + NEO_KHZ800
);

// ============================================================
// Button handling
// ============================================================

enum ButtonId {
  ID_UP = 0,
  ID_DOWN,
  ID_LEFT,
  ID_RIGHT,
  ID_A,
  ID_B,
  BUTTON_COUNT
};

// Arduino sketch preprocessing can auto-generate function prototypes before
// later type declarations. Keep all custom enum types above the first function
// definition so their generated prototypes compile correctly.

enum AppState {
  STATE_MENU,
  STATE_SYSTEM,
  STATE_BUTTONS,
  STATE_JOYSTICK,
  STATE_LED,
  STATE_PIXELS,
  STATE_BUZZER,
  STATE_DISPLAY,
  STATE_I2C,
  STATE_SD,
  STATE_WIFI,
  STATE_BLE,
  STATE_WOLF
};

enum PixelPhase {
  PIXEL_WALK,
  PIXEL_RED,
  PIXEL_GREEN,
  PIXEL_BLUE,
  PIXEL_WHITE
};

enum DisplayPattern {
  DISPLAY_COLOR_BARS,
  DISPLAY_GEOMETRY,
  DISPLAY_GRID,
  DISPLAY_TEXT,
  DISPLAY_SOLID_COLORS,
  DISPLAY_PATTERN_COUNT
};

struct ButtonState {
  const char* name;
  uint8_t pin;
  bool rawPressed;
  bool stablePressed;
  bool pressedEvent;
  bool releasedEvent;
  unsigned long rawChangedAt;
};

ButtonState buttons[BUTTON_COUNT] = {
  {"UP",    BTN_UP,    false, false, false, false, 0},
  {"DOWN",  BTN_DOWN,  false, false, false, false, 0},
  {"LEFT",  BTN_LEFT,  false, false, false, false, 0},
  {"RIGHT", BTN_RIGHT, false, false, false, false, 0},
  {"A",     BTN_A,     false, false, false, false, 0},
  {"B",     BTN_B,     false, false, false, false, 0}
};

constexpr unsigned long BUTTON_DEBOUNCE_MS = 22;

bool rawButtonPressed(ButtonId id) {
  return digitalRead(buttons[id].pin) == BUTTON_ACTIVE_LEVEL;
}

void updateButtons() {
  unsigned long now = millis();

  for (int i = 0; i < BUTTON_COUNT; i++) {
    ButtonState& b = buttons[i];

    b.pressedEvent = false;
    b.releasedEvent = false;

    bool raw = rawButtonPressed((ButtonId)i);

    if (raw != b.rawPressed) {
      b.rawPressed = raw;
      b.rawChangedAt = now;
    }

    if ((now - b.rawChangedAt) >= BUTTON_DEBOUNCE_MS &&
        b.stablePressed != b.rawPressed) {

      b.stablePressed = b.rawPressed;

      if (b.stablePressed) {
        b.pressedEvent = true;
        Serial.printf("[BUTTON] %s pressed\n", b.name);
      } else {
        b.releasedEvent = true;
        Serial.printf("[BUTTON] %s released\n", b.name);
      }
    }
  }
}

bool pressed(ButtonId id) {
  return buttons[id].stablePressed;
}

bool pressedEvent(ButtonId id) {
  return buttons[id].pressedEvent;
}

// ============================================================
// App states / menu
// ============================================================


struct MenuItem {
  const char* label;
  AppState state;
};

MenuItem menuItems[] = {
  {"System / Memory",       STATE_SYSTEM},
  {"Button Test",           STATE_BUTTONS},
  {"Joystick Graph",        STATE_JOYSTICK},
  {"Status + RGB LEDs",     STATE_LED},
  {"24x Addressable LEDs",  STATE_PIXELS},
  {"Buzzer / Audio",        STATE_BUZZER},
  {"Display Test",          STATE_DISPLAY},
  {"I2C / Accelerometer",   STATE_I2C},
  {"SPI / SD Card",         STATE_SD},
  {"WiFi AP Test",          STATE_WIFI},
  {"Bluetooth LE Test",     STATE_BLE}
};

constexpr int MENU_COUNT = sizeof(menuItems) / sizeof(menuItems[0]);
constexpr int MENU_VISIBLE_ROWS = 7;

AppState appState = STATE_MENU;
int menuIndex = 0;

// ============================================================
// Shared drawing helpers
// ============================================================

constexpr uint16_t COLOR_BG = ST77XX_BLACK;
constexpr uint16_t COLOR_TITLE = ST77XX_CYAN;
constexpr uint16_t COLOR_TEXT = ST77XX_WHITE;
constexpr uint16_t COLOR_PASS = ST77XX_GREEN;
constexpr uint16_t COLOR_FAIL = ST77XX_RED;
constexpr uint16_t COLOR_WARN = ST77XX_YELLOW;
constexpr uint16_t COLOR_GREY = 0x7BEF;
constexpr uint16_t COLOR_ORANGE = 0xFD20;

void clearScreen(uint16_t color = COLOR_BG) {
  tft.fillScreen(color);
}

void footer(const char* text = "B: Back") {
  tft.fillRect(0, 218, 320, 22, ST77XX_BLACK);
  tft.drawFastHLine(0, 217, 320, COLOR_GREY);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(6, 224);
  tft.print(text);
}

void testHeader(const char* title) {
  clearScreen();

  tft.setTextWrap(false);
  tft.setTextSize(2);
  tft.setTextColor(COLOR_TITLE);
  tft.setCursor(7, 7);
  tft.print(title);

  tft.drawFastHLine(0, 30, 320, ST77XX_WHITE);
  footer();
}

void printAt(
  int16_t x,
  int16_t y,
  const String& text,
  uint16_t color = ST77XX_WHITE,
  uint8_t size = 1
) {
  tft.setTextSize(size);
  tft.setTextColor(color);
  tft.setCursor(x, y);
  tft.print(text);
}

void clearLine(int16_t y, int16_t height = 14) {
  tft.fillRect(0, y, 320, height, ST77XX_BLACK);
}

void outputsOff() {
  digitalWrite(STATUS_LED, LOW);
  digitalWrite(RGB_R, LOW);
  digitalWrite(RGB_G, LOW);
  digitalWrite(RGB_B, LOW);

  pixels.clear();
  pixels.show();

  noTone(BUZZER_PIN);
}

// ============================================================
// Main menu
// ============================================================

void drawMenu() {
  clearScreen();

  tft.setTextWrap(false);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(8, 6);
  tft.println("eBadge v4.1.1 Diagnostic");

  tft.drawFastHLine(0, 29, 320, ST77XX_WHITE);

  int first = menuIndex - (MENU_VISIBLE_ROWS / 2);

  if (first < 0) first = 0;
  if (first > MENU_COUNT - MENU_VISIBLE_ROWS) {
    first = max(0, MENU_COUNT - MENU_VISIBLE_ROWS);
  }

  for (int row = 0; row < MENU_VISIBLE_ROWS; row++) {
    int index = first + row;
    if (index >= MENU_COUNT) break;

    int y = 38 + row * 24;

    if (index == menuIndex) {
      tft.fillRoundRect(5, y - 3, 310, 21, 4, ST77XX_CYAN);
      tft.setTextColor(ST77XX_BLACK);
    } else {
      tft.setTextColor(ST77XX_WHITE);
    }

    tft.setTextSize(1);
    tft.setCursor(12, y + 3);

    char line[42];
    snprintf(line, sizeof(line), "%2d. %s", index + 1, menuItems[index].label);
    tft.print(line);
  }

  tft.drawFastHLine(0, 211, 320, COLOR_GREY);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(7, 220);
  tft.print("UP/DOWN: Select     A: Run");

  Serial.printf("[MENU] Selected %d: %s\n",
                menuIndex + 1,
                menuItems[menuIndex].label);
}

// ============================================================
// System / memory test
// ============================================================

bool lastPsramTestPass = false;

bool runPsramTest(size_t testSize = 256 * 1024) {
  if (!psramFound()) {
    Serial.println("[SYSTEM] PSRAM not found");
    return false;
  }

  uint8_t* buffer = (uint8_t*)ps_malloc(testSize);

  if (!buffer) {
    Serial.println("[SYSTEM] PSRAM allocation failed");
    return false;
  }

  for (size_t i = 0; i < testSize; i++) {
    buffer[i] = (uint8_t)((i * 37u + 13u) & 0xFFu);
  }

  bool pass = true;

  for (size_t i = 0; i < testSize; i++) {
    uint8_t expected = (uint8_t)((i * 37u + 13u) & 0xFFu);

    if (buffer[i] != expected) {
      Serial.printf(
        "[SYSTEM] PSRAM verify failed at 0x%X expected=%02X got=%02X\n",
        (unsigned)i,
        expected,
        buffer[i]
      );
      pass = false;
      break;
    }
  }

  free(buffer);

  Serial.printf(
    "[SYSTEM] 256 KiB PSRAM write/read: %s\n",
    pass ? "PASS" : "FAIL"
  );

  return pass;
}

void drawSystemTest() {
  testHeader("System / Memory");

  printAt(10, 42, String("Chip: ") + ESP.getChipModel());
  printAt(10, 58, String("CPU: ") + ESP.getCpuFreqMHz() + " MHz");
  printAt(10, 74, String("Flash: ") + (ESP.getFlashChipSize() / 1024 / 1024) + " MiB");
  printAt(10, 90, String("PSRAM: ") + (ESP.getPsramSize() / 1024 / 1024) + " MiB");
  printAt(10, 106, String("Free heap: ") + ESP.getFreeHeap());
  printAt(10, 122, String("Free PSRAM: ") + ESP.getFreePsram());

  printAt(
    10,
    146,
    String("256 KiB PSRAM test: ") + (lastPsramTestPass ? "PASS" : "FAIL"),
    lastPsramTestPass ? COLOR_PASS : COLOR_FAIL,
    2
  );

  printAt(10, 186, "A: Run memory test again", ST77XX_YELLOW);

  footer("A: Retest                 B: Back");
}

void enterSystemTest() {
  Serial.println("\n=== SYSTEM / MEMORY TEST ===");
  lastPsramTestPass = runPsramTest();
  drawSystemTest();
}

// ============================================================
// Button test
// ============================================================

bool lastButtonDisplayState[BUTTON_COUNT] = {};

void drawButtonCell(
  int x,
  int y,
  int w,
  int h,
  const char* label,
  bool active
) {
  uint16_t color = active ? ST77XX_GREEN : ST77XX_RED;

  tft.drawRoundRect(x, y, w, h, 5, color);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(x + 8, y + 6);
  tft.print(label);

  tft.setTextSize(1);
  tft.setTextColor(color);
  tft.setCursor(x + 8, y + 28);
  tft.print(active ? "PRESSED" : "released");
}

void drawButtonTest() {
  testHeader("Button Test");

  printAt(8, 36, "Press every button. B verifies itself by exiting.", ST77XX_YELLOW);

  const int w = 96;
  const int h = 44;

  drawButtonCell(8,   58, w, h, "UP",    pressed(ID_UP));
  drawButtonCell(112, 58, w, h, "DOWN",  pressed(ID_DOWN));
  drawButtonCell(216, 58, w, h, "A",     pressed(ID_A));

  drawButtonCell(8,   112, w, h, "LEFT",  pressed(ID_LEFT));
  drawButtonCell(112, 112, w, h, "RIGHT", pressed(ID_RIGHT));
  drawButtonCell(216, 112, w, h, "B",     pressed(ID_B));

  printAt(12, 174, "Green = pressed", ST77XX_GREEN, 2);
  footer("Press B to verify B and return to menu");
}

void enterButtonTest() {
  Serial.println("\n=== BUTTON TEST ===");

  for (int i = 0; i < BUTTON_COUNT; i++) {
    lastButtonDisplayState[i] = !pressed((ButtonId)i);
  }

  drawButtonTest();
}

void updateButtonTest() {
  bool changed = false;

  for (int i = 0; i < BUTTON_COUNT; i++) {
    bool now = pressed((ButtonId)i);

    if (now != lastButtonDisplayState[i]) {
      lastButtonDisplayState[i] = now;
      changed = true;
    }
  }

  if (changed) {
    drawButtonTest();
  }
}

// ============================================================
// Joystick test
// ============================================================

int oldJoyDotX = -1;
int oldJoyDotY = -1;
unsigned long lastJoyUpdate = 0;

constexpr int JOY_GRAPH_X = 55;
constexpr int JOY_GRAPH_Y = 55;
constexpr int JOY_GRAPH_W = 210;
constexpr int JOY_GRAPH_H = 145;

void drawJoystickBackground() {
  testHeader("Joystick Graph");

  tft.drawRect(
    JOY_GRAPH_X,
    JOY_GRAPH_Y,
    JOY_GRAPH_W,
    JOY_GRAPH_H,
    ST77XX_WHITE
  );

  tft.drawFastVLine(
    JOY_GRAPH_X + JOY_GRAPH_W / 2,
    JOY_GRAPH_Y,
    JOY_GRAPH_H,
    COLOR_GREY
  );

  tft.drawFastHLine(
    JOY_GRAPH_X,
    JOY_GRAPH_Y + JOY_GRAPH_H / 2,
    JOY_GRAPH_W,
    COLOR_GREY
  );

  printAt(4, 55, "Y+", ST77XX_YELLOW);
  printAt(270, 124, "X+", ST77XX_YELLOW);
  printAt(7, 202, "0", COLOR_GREY);

  footer();
}

void enterJoystickTest() {
  Serial.println("\n=== JOYSTICK TEST ===");
  oldJoyDotX = -1;
  oldJoyDotY = -1;
  lastJoyUpdate = 0;
  drawJoystickBackground();
}

void updateJoystickTest() {
  if (millis() - lastJoyUpdate < 60) return;
  lastJoyUpdate = millis();

  int rawX = analogRead(JOY_X);
  int rawY = analogRead(JOY_Y);

  int x = map(
    constrain(rawX, 0, 4095),
    0,
    4095,
    JOY_GRAPH_X + 5,
    JOY_GRAPH_X + JOY_GRAPH_W - 6
  );

  int y = map(
    constrain(rawY, 0, 4095),
    0,
    4095,
    JOY_GRAPH_Y + JOY_GRAPH_H - 6,
    JOY_GRAPH_Y + 5
  );

  if (oldJoyDotX >= 0) {
    tft.fillCircle(oldJoyDotX, oldJoyDotY, 6, ST77XX_BLACK);

    // Restore graph lines that may have been erased.
    tft.drawRect(
      JOY_GRAPH_X,
      JOY_GRAPH_Y,
      JOY_GRAPH_W,
      JOY_GRAPH_H,
      ST77XX_WHITE
    );

    tft.drawFastVLine(
      JOY_GRAPH_X + JOY_GRAPH_W / 2,
      JOY_GRAPH_Y,
      JOY_GRAPH_H,
      COLOR_GREY
    );

    tft.drawFastHLine(
      JOY_GRAPH_X,
      JOY_GRAPH_Y + JOY_GRAPH_H / 2,
      JOY_GRAPH_W,
      COLOR_GREY
    );
  }

  tft.fillCircle(x, y, 6, ST77XX_YELLOW);

  oldJoyDotX = x;
  oldJoyDotY = y;

  tft.fillRect(70, 36, 190, 14, ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(70, 38);
  tft.printf("X=%4d   Y=%4d", rawX, rawY);

  static int lastSerialX = -10000;
  static int lastSerialY = -10000;

  if (abs(rawX - lastSerialX) > 80 ||
      abs(rawY - lastSerialY) > 80) {

    Serial.printf("[JOYSTICK] X=%d Y=%d\n", rawX, rawY);
    lastSerialX = rawX;
    lastSerialY = rawY;
  }
}

// ============================================================
// Status + discrete RGB LED test
// ============================================================

unsigned long ledTestChangedAt = 0;
int ledTestStage = 0;

const char* ledStageName(int stage) {
  switch (stage) {
    case 0: return "STATUS LED ON";
    case 1: return "STATUS LED OFF";
    case 2: return "RGB RED";
    case 3: return "RGB YELLOW";
    case 4: return "RGB GREEN";
    case 5: return "RGB CYAN";
    case 6: return "RGB BLUE";
    case 7: return "RGB MAGENTA";
    case 8: return "RGB WHITE";
    default: return "OFF";
  }
}

void applyLedStage(int stage) {
  digitalWrite(STATUS_LED, LOW);
  digitalWrite(RGB_R, LOW);
  digitalWrite(RGB_G, LOW);
  digitalWrite(RGB_B, LOW);

  switch (stage) {
    case 0:
      digitalWrite(STATUS_LED, HIGH);
      break;

    case 2:
      digitalWrite(RGB_R, HIGH);
      break;

    case 3:
      digitalWrite(RGB_R, HIGH);
      digitalWrite(RGB_G, HIGH);
      break;

    case 4:
      digitalWrite(RGB_G, HIGH);
      break;

    case 5:
      digitalWrite(RGB_G, HIGH);
      digitalWrite(RGB_B, HIGH);
      break;

    case 6:
      digitalWrite(RGB_B, HIGH);
      break;

    case 7:
      digitalWrite(RGB_R, HIGH);
      digitalWrite(RGB_B, HIGH);
      break;

    case 8:
      digitalWrite(RGB_R, HIGH);
      digitalWrite(RGB_G, HIGH);
      digitalWrite(RGB_B, HIGH);
      break;
  }

  Serial.printf("[LED] %s\n", ledStageName(stage));
}

void drawLedTestStage() {
  tft.fillRect(0, 40, 320, 165, ST77XX_BLACK);

  printAt(10, 50, "Automatic LED sequence", ST77XX_WHITE, 2);

  printAt(
    10,
    92,
    ledStageName(ledTestStage),
    ST77XX_YELLOW,
    3
  );

  printAt(10, 150, "Watch GPIO6 status LED and RGB LED.");
  printAt(10, 168, "Sequence repeats automatically.");
}

void enterLedTest() {
  Serial.println("\n=== STATUS + RGB LED TEST ===");
  testHeader("Status + RGB LEDs");

  ledTestStage = 0;
  ledTestChangedAt = millis();

  applyLedStage(ledTestStage);
  drawLedTestStage();
}

void updateLedTest() {
  if (millis() - ledTestChangedAt < 850) return;

  ledTestChangedAt = millis();
  ledTestStage = (ledTestStage + 1) % 9;

  applyLedStage(ledTestStage);
  drawLedTestStage();
}

// ============================================================
// 24x SK6805 test
// ============================================================


PixelPhase pixelPhase = PIXEL_WALK;
int pixelWalkIndex = 0;
unsigned long pixelChangedAt = 0;

const char* pixelPhaseName(PixelPhase phase) {
  switch (phase) {
    case PIXEL_WALK:  return "Walking red pixel";
    case PIXEL_RED:   return "All red";
    case PIXEL_GREEN: return "All green";
    case PIXEL_BLUE:  return "All blue";
    case PIXEL_WHITE: return "All white";
  }
  return "";
}

void drawPixelStatus() {
  tft.fillRect(0, 40, 320, 160, ST77XX_BLACK);

  printAt(10, 48, "GPIO7 / 24 SK6805 LEDs", ST77XX_WHITE, 2);
  printAt(10, 92, pixelPhaseName(pixelPhase), ST77XX_YELLOW, 2);

  if (pixelPhase == PIXEL_WALK) {
    printAt(10, 124, String("LED index: ") + pixelWalkIndex);
  }

  printAt(10, 164, "Brightness limited for safe testing.");
}

void enterPixelTest() {
  Serial.println("\n=== ADDRESSABLE LED TEST ===");
  testHeader("24x Addressable LEDs");

  pixels.clear();
  pixels.show();

  pixelPhase = PIXEL_WALK;
  pixelWalkIndex = 0;
  pixelChangedAt = 0;

  drawPixelStatus();
}

void applyPixelPhase() {
  pixels.clear();

  switch (pixelPhase) {
    case PIXEL_RED:
      pixels.fill(pixels.Color(30, 0, 0));
      break;

    case PIXEL_GREEN:
      pixels.fill(pixels.Color(0, 30, 0));
      break;

    case PIXEL_BLUE:
      pixels.fill(pixels.Color(0, 0, 30));
      break;

    case PIXEL_WHITE:
      pixels.fill(pixels.Color(18, 18, 18));
      break;

    default:
      break;
  }

  pixels.show();

  Serial.printf("[PIXELS] %s\n", pixelPhaseName(pixelPhase));
  drawPixelStatus();
}

void updatePixelTest() {
  unsigned long now = millis();

  if (pixelPhase == PIXEL_WALK) {
    if (now - pixelChangedAt < 120) return;

    pixelChangedAt = now;

    if (pixelWalkIndex >= PIXEL_COUNT) {
      pixelPhase = PIXEL_RED;
      applyPixelPhase();
      return;
    }

    pixels.clear();
    pixels.setPixelColor(
      pixelWalkIndex,
      pixels.Color(35, 0, 0)
    );
    pixels.show();

    Serial.printf("[PIXELS] walking index %d\n", pixelWalkIndex);

    drawPixelStatus();
    pixelWalkIndex++;
    return;
  }

  if (now - pixelChangedAt < 900) return;

  pixelChangedAt = now;

  if (pixelPhase == PIXEL_RED) {
    pixelPhase = PIXEL_GREEN;
    applyPixelPhase();
  } else if (pixelPhase == PIXEL_GREEN) {
    pixelPhase = PIXEL_BLUE;
    applyPixelPhase();
  } else if (pixelPhase == PIXEL_BLUE) {
    pixelPhase = PIXEL_WHITE;
    applyPixelPhase();
  } else {
    pixels.clear();
    pixels.show();

    pixelPhase = PIXEL_WALK;
    pixelWalkIndex = 0;

    Serial.println("[PIXELS] restarting walking pixel");
    drawPixelStatus();
  }
}

// ============================================================
// Buzzer test
// ============================================================

const int buzzerFrequencies[] = {3500, 4000, 4500};
int buzzerIndex = 0;
unsigned long buzzerChangedAt = 0;

void drawBuzzerStatus() {
  tft.fillRect(0, 40, 320, 160, ST77XX_BLACK);

  printAt(12, 48, "Passive magnetic buzzer", ST77XX_WHITE, 2);

  printAt(
    12,
    94,
    String(buzzerFrequencies[buzzerIndex]) + " Hz",
    ST77XX_YELLOW,
    4
  );

  printAt(12, 156, "Adjust volume potentiometer if needed.");
}

void applyBuzzerTone() {
  tone(BUZZER_PIN, buzzerFrequencies[buzzerIndex]);

  Serial.printf(
    "[BUZZER] tone %d Hz\n",
    buzzerFrequencies[buzzerIndex]
  );

  drawBuzzerStatus();
}

void enterBuzzerTest() {
  Serial.println("\n=== BUZZER TEST ===");
  testHeader("Buzzer / Audio");

  buzzerIndex = 0;
  buzzerChangedAt = millis();
  applyBuzzerTone();
}

void updateBuzzerTest() {
  if (millis() - buzzerChangedAt < 1000) return;

  buzzerChangedAt = millis();
  buzzerIndex = (buzzerIndex + 1) % 3;
  applyBuzzerTone();
}

// ============================================================
// Display test
// ============================================================


DisplayPattern displayPattern = DISPLAY_COLOR_BARS;
int solidColorIndex = 0;
unsigned long displayPatternChangedAt = 0;

const uint16_t solidColors[] = {
  ST77XX_BLACK,
  ST77XX_RED,
  ST77XX_GREEN,
  ST77XX_BLUE,
  ST77XX_CYAN,
  ST77XX_MAGENTA,
  ST77XX_YELLOW,
  ST77XX_WHITE
};

const char* solidColorNames[] = {
  "BLACK",
  "RED",
  "GREEN",
  "BLUE",
  "CYAN",
  "MAGENTA",
  "YELLOW",
  "WHITE"
};

void displayColorBars() {
  int barW = 320 / 8;

  uint16_t colors[8] = {
    ST77XX_WHITE,
    ST77XX_YELLOW,
    ST77XX_CYAN,
    ST77XX_GREEN,
    ST77XX_MAGENTA,
    ST77XX_RED,
    ST77XX_BLUE,
    ST77XX_BLACK
  };

  for (int i = 0; i < 8; i++) {
    int x = i * barW;
    int w = (i == 7) ? 320 - x : barW;
    tft.fillRect(x, 0, w, 218, colors[i]);
  }

  footer("A/UP/DOWN: Next pattern        B: Back");
}

void displayGeometry() {
  clearScreen();

  tft.drawRect(0, 0, 320, 218, ST77XX_WHITE);

  for (int i = 10; i < 80; i += 10) {
    tft.drawRect(
      i,
      i,
      320 - i * 2,
      218 - i * 2,
      ST77XX_GREEN
    );
  }

  tft.drawLine(0, 0, 319, 217, ST77XX_RED);
  tft.drawLine(319, 0, 0, 217, ST77XX_BLUE);

  tft.drawFastVLine(160, 0, 218, ST77XX_YELLOW);
  tft.drawFastHLine(0, 109, 320, ST77XX_YELLOW);

  for (int r = 10; r <= 60; r += 10) {
    tft.drawCircle(160, 109, r, ST77XX_CYAN);
  }

  footer("A/UP/DOWN: Next pattern        B: Back");
}

void displayGrid() {
  clearScreen();

  for (int x = 0; x < 320; x += 10) {
    tft.drawFastVLine(x, 0, 218, ST77XX_BLUE);
  }

  for (int y = 0; y < 218; y += 10) {
    tft.drawFastHLine(0, y, 320, ST77XX_GREEN);
  }

  tft.fillRect(0, 0, 20, 20, ST77XX_RED);
  tft.fillRect(300, 0, 20, 20, ST77XX_GREEN);
  tft.fillRect(0, 198, 20, 20, ST77XX_BLUE);
  tft.fillRect(300, 198, 20, 20, ST77XX_WHITE);

  footer("A/UP/DOWN: Next pattern        B: Back");
}

void displayTextPattern() {
  clearScreen();

  tft.setTextWrap(false);

  printAt(10, 10, "eBadge v4.1.1", ST77XX_CYAN, 3);
  printAt(10, 52, "ST7789 320 x 240", ST77XX_WHITE, 2);

  printAt(10, 88, "RED", ST77XX_RED, 2);
  printAt(70, 88, "GREEN", ST77XX_GREEN, 2);
  printAt(170, 88, "BLUE", ST77XX_BLUE, 2);

  printAt(10, 120, "CYAN", ST77XX_CYAN, 2);
  printAt(90, 120, "MAGENTA", ST77XX_MAGENTA, 2);
  printAt(220, 120, "YELLOW", ST77XX_YELLOW, 2);

  printAt(10, 160, "0123456789 ABCDEFGHIJKLMNOP", ST77XX_WHITE);
  printAt(10, 176, "QRSTUVWXYZ !@#$%^&*()", ST77XX_WHITE);
  printAt(10, 192, "Konami", ST77XX_WHITE);

  footer("A/UP/DOWN: Next pattern        B: Back");
}

void drawDisplayPattern() {
  tft.invertDisplay(true);
  delay(2);
  switch (displayPattern) {
    case DISPLAY_COLOR_BARS:
      Serial.println("[DISPLAY] color bars");
      displayColorBars();
      break;

    case DISPLAY_GEOMETRY:
      Serial.println("[DISPLAY] geometry");
      displayGeometry();
      break;

    case DISPLAY_GRID:
      Serial.println("[DISPLAY] pixel grid");
      displayGrid();
      break;

    case DISPLAY_TEXT:
      Serial.println("[DISPLAY] text");
      displayTextPattern();
      break;

    case DISPLAY_SOLID_COLORS:
      Serial.println("[DISPLAY] solid colors");
      solidColorIndex = 0;
      displayPatternChangedAt = 0;
      break;

    default:
      break;
  }
}

void enterDisplayTest() {
  Serial.println("\n=== DISPLAY TEST ===");

  // Force ST7789 back into normal color mode.
  tft.invertDisplay(true);
  delay(5);

  displayPattern = DISPLAY_COLOR_BARS;
  drawDisplayPattern();
}

void nextDisplayPattern(int direction) {
  int next = (int)displayPattern + direction;

  if (next < 0) next = DISPLAY_PATTERN_COUNT - 1;
  if (next >= DISPLAY_PATTERN_COUNT) next = 0;

  displayPattern = (DisplayPattern)next;
  drawDisplayPattern();
}

void updateDisplayTest() {
  if (pressedEvent(ID_A) || pressedEvent(ID_DOWN)) {
    nextDisplayPattern(+1);
    return;
  }

  if (pressedEvent(ID_UP)) {
    nextDisplayPattern(-1);
    return;
  }

  if (displayPattern == DISPLAY_SOLID_COLORS) {
    if (millis() - displayPatternChangedAt < 800) return;

    displayPatternChangedAt = millis();

    uint16_t c = solidColors[solidColorIndex];

    tft.fillScreen(c);

    // Make the current color name readable.
    uint16_t textColor =
      (solidColorIndex == 7 || solidColorIndex == 6)
      ? ST77XX_BLACK
      : ST77XX_WHITE;

    tft.setTextColor(textColor);
    tft.setTextSize(3);
    tft.setCursor(85, 90);
    tft.print(solidColorNames[solidColorIndex]);

    Serial.printf(
      "[DISPLAY] solid %s\n",
      solidColorNames[solidColorIndex]
    );

    solidColorIndex =
      (solidColorIndex + 1) %
      (sizeof(solidColors) / sizeof(solidColors[0]));
  }
}

// ============================================================
// I2C + MMA8452 test
// ============================================================

uint8_t foundI2C[20] = {};
int foundI2CCount = 0;
bool mmaPresent = false;
uint8_t mmaWhoAmI = 0;
bool mmaConfigured = false;
unsigned long lastAccelUpdate = 0;

bool i2cReadRegister(
  uint8_t addr,
  uint8_t reg,
  uint8_t& value
) {
  Wire.beginTransmission(addr);
  Wire.write(reg);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(addr, (uint8_t)1) != 1) {
    return false;
  }

  value = Wire.read();
  return true;
}

bool i2cWriteRegister(
  uint8_t addr,
  uint8_t reg,
  uint8_t value
) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

void scanI2C() {
  foundI2CCount = 0;

  Serial.println("[I2C] scanning bus...");

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t result = Wire.endTransmission();

    if (result == 0) {
      if (foundI2CCount < 20) {
        foundI2C[foundI2CCount] = addr;
      }

      foundI2CCount++;
      Serial.printf("[I2C] found device 0x%02X\n", addr);
    }
  }

  mmaPresent = i2cReadRegister(
    MMA8452_ADDR,
    0x0D,
    mmaWhoAmI
  );

  mmaConfigured = false;

  if (mmaPresent && mmaWhoAmI == 0x2A) {
    // Standby, +/-2g, active.
    i2cWriteRegister(MMA8452_ADDR, 0x2A, 0x00);
    i2cWriteRegister(MMA8452_ADDR, 0x0E, 0x00);
    i2cWriteRegister(MMA8452_ADDR, 0x2A, 0x01);
    mmaConfigured = true;
  }

  Serial.printf(
    "[I2C] MMA8452 WHO_AM_I=0x%02X %s\n",
    mmaWhoAmI,
    (mmaPresent && mmaWhoAmI == 0x2A) ? "PASS" : "FAIL"
  );
}

bool readMmaRaw(int16_t& x, int16_t& y, int16_t& z) {
  if (!mmaConfigured) return false;

  Wire.beginTransmission(MMA8452_ADDR);
  Wire.write(0x01);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(MMA8452_ADDR, (uint8_t)6) != 6) {
    return false;
  }

  uint8_t xh = Wire.read();
  uint8_t xl = Wire.read();
  uint8_t yh = Wire.read();
  uint8_t yl = Wire.read();
  uint8_t zh = Wire.read();
  uint8_t zl = Wire.read();

  x = (int16_t)((xh << 8) | xl);
  y = (int16_t)((yh << 8) | yl);
  z = (int16_t)((zh << 8) | zl);

  // MMA8452Q uses 12-bit left-aligned output.
  x >>= 4;
  y >>= 4;
  z >>= 4;

  return true;
}

void drawI2CTest() {
  testHeader("I2C / Accelerometer");

  printAt(
    8,
    40,
    String("Devices found: ") + foundI2CCount,
    foundI2CCount > 0 ? COLOR_PASS : COLOR_FAIL,
    2
  );

  String addresses = "Addresses: ";

  int showCount = min(foundI2CCount, 8);

  for (int i = 0; i < showCount; i++) {
    char temp[8];
    snprintf(temp, sizeof(temp), "0x%02X ", foundI2C[i]);
    addresses += temp;
  }

  printAt(8, 70, addresses);

  printAt(8, 100, "Expected MMA8452: 0x1C");
  printAt(
    8,
    118,
    String("WHO_AM_I: 0x") +
      String(mmaWhoAmI, HEX) +
      ((mmaPresent && mmaWhoAmI == 0x2A) ? "  PASS" : "  FAIL"),
    (mmaPresent && mmaWhoAmI == 0x2A) ? COLOR_PASS : COLOR_FAIL,
    2
  );

  printAt(8, 160, "Accel raw XYZ:", ST77XX_YELLOW);
  footer("A: Rescan I2C             B: Back");
}

void enterI2CTest() {
  Serial.println("\n=== I2C / ACCELEROMETER TEST ===");
  scanI2C();
  drawI2CTest();
  lastAccelUpdate = 0;
}

void updateI2CTest() {
  if (pressedEvent(ID_A)) {
    scanI2C();
    drawI2CTest();
    return;
  }

  if (millis() - lastAccelUpdate < 150) return;
  lastAccelUpdate = millis();

  int16_t x, y, z;

  if (readMmaRaw(x, y, z)) {
    tft.fillRect(8, 176, 300, 24, ST77XX_BLACK);

    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(8, 178);
    tft.printf("X:%5d Y:%5d Z:%5d", x, y, z);

    static unsigned long lastSerial = 0;

    if (millis() - lastSerial > 750) {
      lastSerial = millis();
      Serial.printf("[ACCEL] X=%d Y=%d Z=%d\n", x, y, z);
    }
  }
}

// ============================================================
// SPI / SD-card test
// ============================================================

bool sdInitPass = false;
bool sdRwPass = false;
uint8_t sdType = CARD_NONE;
uint64_t sdSizeMB = 0;

String sdTypeName(uint8_t type) {
  switch (type) {
    case CARD_MMC:  return "MMC";
    case CARD_SD:   return "SDSC";
    case CARD_SDHC: return "SDHC/SDXC";
    case CARD_NONE: return "NONE";
    default:        return "UNKNOWN";
  }
}

void cleanupSD() {
  SD.end();
  sdSPI.end();
}

void runSDTest() {
  cleanupSD();

  sdInitPass = false;
  sdRwPass = false;
  sdType = CARD_NONE;
  sdSizeMB = 0;

  Serial.println("[SPI/SD] Initializing SPI bus");
  Serial.printf(
    "[SPI/SD] SCLK=%d MISO=%d MOSI=%d CS=%d\n",
    SD_SCLK,
    SD_MISO,
    SD_MOSI,
    SD_CS
  );

  sdSPI.begin(
    SD_SCLK,
    SD_MISO,
    SD_MOSI,
    SD_CS
  );

  if (!SD.begin(SD_CS, sdSPI, 10000000)) {
    Serial.println("[SPI/SD] SD.begin() FAILED");
    return;
  }

  sdType = SD.cardType();

  if (sdType == CARD_NONE) {
    Serial.println("[SPI/SD] No SD card detected");
    return;
  }

  sdInitPass = true;
  sdSizeMB = SD.cardSize() / (1024ULL * 1024ULL);

  Serial.printf(
    "[SPI/SD] Card type=%s size=%llu MiB\n",
    sdTypeName(sdType).c_str(),
    sdSizeMB
  );

  const char* path = "/ebadge_diag_test.txt";
  const char* payload = "BYU-I eBadge v4.1.1 SD test PASS";

  SD.remove(path);

  File file = SD.open(path, FILE_WRITE);

  if (!file) {
    Serial.println("[SPI/SD] Could not create test file");
    return;
  }

  file.print(payload);
  file.close();

  file = SD.open(path, FILE_READ);

  if (!file) {
    Serial.println("[SPI/SD] Could not reopen test file");
    return;
  }

  String data = file.readString();
  file.close();

  data.trim();

  sdRwPass = (data == payload);

  Serial.printf(
    "[SPI/SD] write/read compare: %s\n",
    sdRwPass ? "PASS" : "FAIL"
  );

  SD.remove(path);
}

void drawSDTest() {
  testHeader("SPI / SD Card");

  printAt(8, 40, "SPI3 pin test via SD card", ST77XX_WHITE, 2);

  printAt(8, 72, "CLK  GPIO38");
  printAt(8, 88, "MISO GPIO37");
  printAt(8, 104, "MOSI GPIO39");
  printAt(8, 120, "CS   GPIO40");

  printAt(
    150,
    72,
    String("Card: ") + (sdInitPass ? "DETECTED" : "NOT FOUND"),
    sdInitPass ? COLOR_PASS : COLOR_FAIL,
    1
  );

  if (sdInitPass) {
    printAt(150, 92, String("Type: ") + sdTypeName(sdType));
    printAt(150, 108, String("Size: ") + String((unsigned long)sdSizeMB) + " MiB");

    printAt(
      150,
      132,
      String("R/W test: ") + (sdRwPass ? "PASS" : "FAIL"),
      sdRwPass ? COLOR_PASS : COLOR_FAIL,
      2
    );
  } else {
    printAt(150, 98, "Insert FAT-formatted SD card.", COLOR_WARN);
  }

  footer("A: Retest SD              B: Back");
}

void enterSDTest() {
  Serial.println("\n=== SPI / SD CARD TEST ===");
  runSDTest();
  drawSDTest();
}

void updateSDTest() {
  if (pressedEvent(ID_A)) {
    runSDTest();
    drawSDTest();
  }
}

// ============================================================
// Wi-Fi SoftAP test
// ============================================================

constexpr const char* WIFI_TEST_SSID = "Ebadge_wifi_test";
constexpr const char* WIFI_TEST_PASSWORD = "password";

bool wifiAPStarted = false;
unsigned long lastWifiScreenUpdate = 0;
int lastWifiClientCount = -1;

String firstWifiClientInfo() {
  wifi_sta_list_t stationList;

  if (esp_wifi_ap_get_sta_list(&stationList) != ESP_OK ||
      stationList.num <= 0) {
    return "No client connected";
  }

  wifi_sta_info_t& s = stationList.sta[0];

  char buffer[64];

  snprintf(
    buffer,
    sizeof(buffer),
    "%02X:%02X:%02X:%02X:%02X:%02X  RSSI %d",
    s.mac[0],
    s.mac[1],
    s.mac[2],
    s.mac[3],
    s.mac[4],
    s.mac[5],
    s.rssi
  );

  return String(buffer);
}

void stopWifiTest() {
  if (wifiAPStarted) {
    Serial.println("[WIFI] stopping test AP");
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiAPStarted = false;
    delay(40);
  }
}

void drawWifiTest() {
  testHeader("WiFi Access Point");

  if (!wifiAPStarted) {
    printAt(20, 80, "FAILED TO START AP", COLOR_FAIL, 2);
    printAt(20, 112, "Press A to retry.", COLOR_WARN, 2);
    footer("A: Retry                  B: Back");
    return;
  }

  printAt(8, 40, "SSID:", ST77XX_CYAN);
  printAt(52, 40, WIFI_TEST_SSID, ST77XX_WHITE, 2);

  printAt(8, 66, "Password:", ST77XX_CYAN);
  printAt(72, 66, WIFI_TEST_PASSWORD, ST77XX_WHITE, 2);

  printAt(8, 94, String("AP IP:  ") + WiFi.softAPIP().toString());
  printAt(8, 110, String("AP MAC: ") + WiFi.softAPmacAddress());

  int clients = WiFi.softAPgetStationNum();

  printAt(
    8,
    136,
    String("Connected clients: ") + clients,
    clients > 0 ? COLOR_PASS : COLOR_WARN,
    2
  );

  tft.fillRect(8, 162, 304, 32, ST77XX_BLACK);

  if (clients > 0) {
    printAt(8, 164, "First client:", ST77XX_CYAN);
    printAt(8, 180, firstWifiClientInfo(), ST77XX_WHITE);
  } else {
    printAt(8, 174, "Connect a phone/laptop to verify AP.", ST77XX_YELLOW);
  }

  footer();
}

void startWifiTest() {
  stopWifiTest();

  Serial.println("[WIFI] starting SoftAP");
  Serial.printf("[WIFI] SSID=%s password=%s\n",
                WIFI_TEST_SSID,
                WIFI_TEST_PASSWORD);

  WiFi.mode(WIFI_AP);

  wifiAPStarted = WiFi.softAP(
    WIFI_TEST_SSID,
    WIFI_TEST_PASSWORD
  );

  if (wifiAPStarted) {
    Serial.printf(
      "[WIFI] AP IP=%s MAC=%s\n",
      WiFi.softAPIP().toString().c_str(),
      WiFi.softAPmacAddress().c_str()
    );
  } else {
    Serial.println("[WIFI] SoftAP start FAILED");
  }

  lastWifiClientCount = -1;
  lastWifiScreenUpdate = 0;
}

void enterWifiTest() {
  Serial.println("\n=== WIFI AP TEST ===");
  startWifiTest();
  drawWifiTest();
}

void updateWifiTest() {
  if (pressedEvent(ID_A) && !wifiAPStarted) {
    startWifiTest();
    drawWifiTest();
    return;
  }

  if (!wifiAPStarted) return;

  if (millis() - lastWifiScreenUpdate < 600) return;
  lastWifiScreenUpdate = millis();

  int clients = WiFi.softAPgetStationNum();

  if (clients != lastWifiClientCount) {
    lastWifiClientCount = clients;

    Serial.printf(
      "[WIFI] clients=%d %s\n",
      clients,
      firstWifiClientInfo().c_str()
    );

    drawWifiTest();
  }
}

// ============================================================
// Bluetooth LE test
// ============================================================

constexpr const char* BLE_TEST_NAME = "Ebadge_BLE_Test";
constexpr const char* BLE_SERVICE_UUID =
  "7c1b0001-5a5a-4e42-4144-474554455354";
constexpr const char* BLE_CHARACTERISTIC_UUID =
  "7c1b0002-5a5a-4e42-4144-474554455354";

BLEServer* bleServer = nullptr;
BLEService* bleService = nullptr;
BLECharacteristic* bleCharacteristic = nullptr;
bool bleInitialized = false;
unsigned long lastBleUpdate = 0;
int lastBleClientCount = -1;

void stopBleTest() {
  if (!bleInitialized) return;

  Serial.println("[BLE] stopping advertising / BLE stack");

  BLEDevice::stopAdvertising();
  BLEDevice::deinit(false);

  bleServer = nullptr;
  bleService = nullptr;
  bleCharacteristic = nullptr;
  bleInitialized = false;

  delay(60);
}

bool startBleTest() {
  stopBleTest();

  Serial.println("[BLE] initializing");

  if (!BLEDevice::init(BLE_TEST_NAME)) {
    Serial.println("[BLE] initialization FAILED");
    return false;
  }

  bleServer = BLEDevice::createServer();

  if (!bleServer) {
    Serial.println("[BLE] server creation FAILED");
    BLEDevice::deinit(false);
    return false;
  }

  bleServer->advertiseOnDisconnect(true);

  bleService = bleServer->createService(BLE_SERVICE_UUID);

  if (!bleService) {
    Serial.println("[BLE] service creation FAILED");
    BLEDevice::deinit(false);
    bleServer = nullptr;
    return false;
  }

  bleCharacteristic = bleService->createCharacteristic(
    BLE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ
  );

  if (bleCharacteristic) {
    bleCharacteristic->setValue(
      "BYU-I eBadge v4.1.1 hardware diagnostic"
    );
  }

  bleService->start();

  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BLE_SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMaxPreferred(0x12);

  BLEDevice::startAdvertising();

  bleInitialized = true;
  lastBleClientCount = -1;
  lastBleUpdate = 0;

  Serial.printf(
    "[BLE] advertising name=%s address=%s\n",
    BLE_TEST_NAME,
    BLEDevice::getAddress().toString().c_str()
  );

  return true;
}

void drawBleTest() {
  testHeader("Bluetooth LE");

  if (!bleInitialized) {
    printAt(20, 82, "BLE INITIALIZATION FAILED", COLOR_FAIL, 2);
    printAt(20, 116, "Press A to retry.", COLOR_WARN, 2);
    footer("A: Retry                  B: Back");
    return;
  }

  printAt(8, 42, "Advertising name:", ST77XX_CYAN);
  printAt(8, 60, BLE_TEST_NAME, ST77XX_WHITE, 2);

  printAt(8, 90, "BLE address:", ST77XX_CYAN);
  printAt(
    8,
    106,
    BLEDevice::getAddress().toString(),
    ST77XX_WHITE,
    2
  );

  int clients =
    bleServer ? (int)bleServer->getConnectedCount() : 0;

  printAt(
    8,
    140,
    String("Connected clients: ") + clients,
    clients > 0 ? COLOR_PASS : COLOR_WARN,
    2
  );

  printAt(8, 174, "Scan for Ebadge_BLE_Test from a phone.");
  footer();
}

void enterBleTest() {
  Serial.println("\n=== BLUETOOTH LE TEST ===");
  startBleTest();
  drawBleTest();
}

void updateBleTest() {
  if (pressedEvent(ID_A) && !bleInitialized) {
    startBleTest();
    drawBleTest();
    return;
  }

  if (!bleInitialized || !bleServer) return;

  if (millis() - lastBleUpdate < 600) return;
  lastBleUpdate = millis();

  int clients = (int)bleServer->getConnectedCount();

  if (clients != lastBleClientCount) {
    lastBleClientCount = clients;

    Serial.printf(
      "[BLE] connected clients=%d\n",
      clients
    );

    drawBleTest();
  }
}

// ============================================================
// Wolf easter egg
// ============================================================

const ButtonId wolfSequence[] = {
  ID_UP,
  ID_UP,
  ID_DOWN,
  ID_DOWN,
  ID_LEFT,
  ID_RIGHT,
  ID_LEFT,
  ID_RIGHT,
  ID_B,
  ID_A
};

constexpr int WOLF_SEQUENCE_LENGTH =
  sizeof(wolfSequence) / sizeof(wolfSequence[0]);

int wolfSequencePos = 0;

bool feedWolfSequence(ButtonId id) {
  if (id == wolfSequence[wolfSequencePos]) {
    wolfSequencePos++;

    if (wolfSequencePos >= WOLF_SEQUENCE_LENGTH) {
      wolfSequencePos = 0;
      return true;
    }

    return false;
  }

  wolfSequencePos =
    (id == wolfSequence[0]) ? 1 : 0;

  return false;
}

void drawWolf() {
  clearScreen();

  tft.setTextColor(COLOR_ORANGE);
  tft.setTextSize(2);
  tft.setCursor(52, 14);
  tft.println("Created By Wolf");

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);

  tft.setCursor(100, 63);
  tft.println("/\\_/\\");
  tft.setCursor(92, 91);
  tft.println("( o.o )");
  tft.setCursor(105, 119);
  tft.println("> ^ <");

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(76, 158);
  tft.println("WolfWorks Labs");

  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(65, 190);
  tft.println("AWOOOO! Diagnostics in the wild.");

  footer("B: Return to the civilized menu");

  Serial.println("\n[WOLF] AWOOOO! Wolf mode unlocked.");
}

void enterWolf() {
  appState = STATE_WOLF;

  // Tiny "awoo" chirp around the buzzer's useful range.
  tone(BUZZER_PIN, 3600);
  delay(80);
  tone(BUZZER_PIN, 4000);
  delay(100);
  tone(BUZZER_PIN, 4400);
  delay(120);
  noTone(BUZZER_PIN);

  drawWolf();
}

// ============================================================
// State management
// ============================================================

void leaveCurrentTest() {
  outputsOff();

  if (appState == STATE_SD) {
    cleanupSD();
  }

  if (appState == STATE_WIFI) {
    stopWifiTest();
  }

  if (appState == STATE_BLE) {
    stopBleTest();
  }
}

void returnToMenu() {
  leaveCurrentTest();
  appState = STATE_MENU;
  drawMenu();
}

void enterTest(AppState state) {
  leaveCurrentTest();

  appState = state;

  switch (state) {
    case STATE_SYSTEM:
      enterSystemTest();
      break;

    case STATE_BUTTONS:
      enterButtonTest();
      break;

    case STATE_JOYSTICK:
      enterJoystickTest();
      break;

    case STATE_LED:
      enterLedTest();
      break;

    case STATE_PIXELS:
      enterPixelTest();
      break;

    case STATE_BUZZER:
      enterBuzzerTest();
      break;

    case STATE_DISPLAY:
      enterDisplayTest();
      break;

    case STATE_I2C:
      enterI2CTest();
      break;

    case STATE_SD:
      enterSDTest();
      break;

    case STATE_WIFI:
      enterWifiTest();
      break;

    case STATE_BLE:
      enterBleTest();
      break;

    default:
      returnToMenu();
      break;
  }
}

// ============================================================
// Menu input
// ============================================================

bool processWolfInputs() {
  // Feed secret-sequence buttons before normal menu handling.
  ButtonId order[] = {
    ID_UP,
    ID_DOWN,
    ID_LEFT,
    ID_RIGHT,
    ID_B,
    ID_A
  };

  for (ButtonId id : order) {
    if (pressedEvent(id)) {
      if (feedWolfSequence(id)) {
        enterWolf();
        return true;
      }
    }
  }

  return false;
}

void handleMenu() {
  if (processWolfInputs()) {
    return;
  }

  if (pressedEvent(ID_UP)) {
    menuIndex--;
    if (menuIndex < 0) menuIndex = MENU_COUNT - 1;
    drawMenu();
  }

  if (pressedEvent(ID_DOWN)) {
    menuIndex++;
    if (menuIndex >= MENU_COUNT) menuIndex = 0;
    drawMenu();
  }

  if (pressedEvent(ID_A)) {
    enterTest(menuItems[menuIndex].state);
  }
}

// ============================================================
// Setup
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(STATUS_LED, OUTPUT);

  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_LEFT, INPUT);
  pinMode(BTN_RIGHT, INPUT);
  pinMode(BTN_A, INPUT);
  pinMode(BTN_B, INPUT);

  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);

  analogReadResolution(12);

  pixels.begin();
  pixels.setBrightness(32);
  pixels.clear();
  pixels.show();

  outputsOff();

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);

  // Initialize display last among strapping-pin peripherals.
  // Route FSPI through the actual v4.1.1 display pins before the
  // Adafruit driver initializes the controller.
  displaySPI.begin(
    TFT_SCLK,
    -1,          // display has no MISO/read line
    TFT_MOSI,
    TFT_CS
  );

  tft.init(240, 320);
  tft.setRotation(1);
  tft.invertDisplay(true);

  // 40 MHz is a normal hardware-SPI rate for this ST7789 test and is
  // dramatically faster than the software-bit-banged implementation.
  tft.setSPISpeed(40000000);
  clearScreen();

  printAt(26, 34, "BYU-I eBadge", ST77XX_CYAN, 3);
  printAt(68, 78, "v4.1.1 AUG26", ST77XX_WHITE, 2);
  printAt(34, 118, "Standalone Hardware", ST77XX_YELLOW, 2);
  printAt(74, 144, "Diagnostic", ST77XX_YELLOW, 2);
  printAt(56, 184, "Release all buttons...", ST77XX_WHITE);

  Serial.println();
  Serial.println("============================================================");
  Serial.println(" BYU-I eBadge v4.1.1 Standalone Hardware Diagnostic");
  Serial.println("============================================================");

  // Give buttons time to settle before accepting navigation.
  delay(800);

  for (int i = 0; i < BUTTON_COUNT; i++) {
    buttons[i].rawPressed = rawButtonPressed((ButtonId)i);
    buttons[i].stablePressed = buttons[i].rawPressed;
    buttons[i].rawChangedAt = millis();
  }

  appState = STATE_MENU;
  drawMenu();
}

// ============================================================
// Main loop
// ============================================================

void loop() {
  updateButtons();

  // B is an emergency/back key while a test is active.  Check the raw
  // pin as well as the debounced event so a short press is not lost while
  // a display transaction is finishing.  Hardware SPI keeps those
  // transactions short, while this raw-level check makes Back immediate.
  if (appState != STATE_MENU &&
      appState != STATE_WOLF &&
      (rawButtonPressed(ID_B) || pressedEvent(ID_B))) {

    Serial.println("[UI] B pressed - returning to menu");
    returnToMenu();
    delay(2);
    return;
  }

  if (appState == STATE_WOLF) {
    if (rawButtonPressed(ID_B) || pressedEvent(ID_B)) {
      returnToMenu();
    }

    delay(2);
    return;
  }

  switch (appState) {
    case STATE_MENU:
      handleMenu();
      break;

    case STATE_SYSTEM:
      if (pressedEvent(ID_A)) {
        lastPsramTestPass = runPsramTest();
        drawSystemTest();
      }
      break;

    case STATE_BUTTONS:
      updateButtonTest();
      break;

    case STATE_JOYSTICK:
      updateJoystickTest();
      break;

    case STATE_LED:
      updateLedTest();
      break;

    case STATE_PIXELS:
      updatePixelTest();
      break;

    case STATE_BUZZER:
      updateBuzzerTest();
      break;

    case STATE_DISPLAY:
      updateDisplayTest();
      break;

    case STATE_I2C:
      updateI2CTest();
      break;

    case STATE_SD:
      updateSDTest();
      break;

    case STATE_WIFI:
      updateWifiTest();
      break;

    case STATE_BLE:
      updateBleTest();
      break;

    default:
      break;
  }

  delay(2);
}
