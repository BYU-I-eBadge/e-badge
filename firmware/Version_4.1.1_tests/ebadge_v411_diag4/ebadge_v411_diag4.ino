#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ============================================================
// BYU-I eBadge v4.1.1 AUG26
// Diagnostic #4 - ST7789 Display Test
// ============================================================

// v4.1.1 display connections
constexpr int TFT_CS   = 0;
constexpr int TFT_RST  = 1;
constexpr int TFT_DC   = 45;
constexpr int TFT_MOSI = 3;
constexpr int TFT_SCLK = 46;

// Use software SPI explicitly so this test touches exactly
// the display pins and does not interfere with the SD SPI bus.
Adafruit_ST7789 tft =
Adafruit_ST7789(
    TFT_CS,
    TFT_DC,
    TFT_MOSI,
    TFT_SCLK,
    TFT_RST
);

uint8_t rotation = 1;
bool inverted = false;

// ============================================================
// Helpers
// ============================================================

void header(const char* title)
{
    tft.fillScreen(ST77XX_BLACK);

    tft.setTextWrap(false);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(8, 8);
    tft.println(title);

    tft.drawFastHLine(
        0,
        31,
        tft.width(),
                      ST77XX_WHITE
    );
}

void serialSeparator()
{
    Serial.println(
        "------------------------------------------------------------"
    );
}

// ============================================================
// Solid-color test
// ============================================================

void solidColors()
{
    serialSeparator();
    Serial.println("SOLID COLOR TEST");
    serialSeparator();

    struct ColorTest
    {
        const char* name;
        uint16_t color;
    };

    ColorTest colors[] =
    {
        {"BLACK",   ST77XX_BLACK},
        {"RED",     ST77XX_RED},
        {"GREEN",   ST77XX_GREEN},
        {"BLUE",    ST77XX_BLUE},
        {"CYAN",    ST77XX_CYAN},
        {"MAGENTA", ST77XX_MAGENTA},
        {"YELLOW",  ST77XX_YELLOW},
        {"WHITE",   ST77XX_WHITE}
    };

    for (auto &c : colors)
    {
        Serial.printf("Display: %s\n", c.name);

        tft.fillScreen(c.color);

        delay(700);
    }

    tft.fillScreen(ST77XX_BLACK);
}

// ============================================================
// Color bars
// ============================================================

void colorBars()
{
    serialSeparator();
    Serial.println("COLOR BAR TEST");
    serialSeparator();

    int w = tft.width();
    int h = tft.height();

    int barWidth = w / 8;

    uint16_t colors[8] =
    {
        ST77XX_WHITE,
        ST77XX_YELLOW,
        ST77XX_CYAN,
        ST77XX_GREEN,
        ST77XX_MAGENTA,
        ST77XX_RED,
        ST77XX_BLUE,
        ST77XX_BLACK
    };

    for (int i = 0; i < 8; i++)
    {
        int x = i * barWidth;

        int width =
        (i == 7)
        ? w - x
        : barWidth;

        tft.fillRect(
            x,
            0,
            width,
            h,
            colors[i]
        );
    }

    Serial.println("Color bars displayed.");
}

// ============================================================
// Geometry test
// ============================================================

void geometryTest()
{
    serialSeparator();
    Serial.println("GEOMETRY TEST");
    serialSeparator();

    tft.fillScreen(ST77XX_BLACK);

    int w = tft.width();
    int h = tft.height();

    // Outer border
    tft.drawRect(
        0,
        0,
        w,
        h,
        ST77XX_WHITE
    );

    // Nested rectangles
    for (int i = 10; i < 70; i += 10)
    {
        tft.drawRect(
            i,
            i,
            w - i * 2,
            h - i * 2,
            ST77XX_GREEN
        );
    }

    // Diagonals
    tft.drawLine(
        0,
        0,
        w - 1,
        h - 1,
        ST77XX_RED
    );

    tft.drawLine(
        w - 1,
        0,
        0,
        h - 1,
        ST77XX_BLUE
    );

    // Center cross
    tft.drawFastHLine(
        0,
        h / 2,
        w,
        ST77XX_YELLOW
    );

    tft.drawFastVLine(
        w / 2,
        0,
        h,
        ST77XX_YELLOW
    );

    // Circles
    for (int r = 10; r <= 50; r += 10)
    {
        tft.drawCircle(
            w / 2,
            h / 2,
            r,
            ST77XX_CYAN
        );
    }

    Serial.println("Geometry test displayed.");
}

// ============================================================
// Text test
// ============================================================

void textTest()
{
    serialSeparator();
    Serial.println("TEXT TEST");
    serialSeparator();

    header("eBadge v4.1.1");

    tft.setCursor(10, 45);

    tft.setTextSize(2);
    tft.setTextColor(ST77XX_GREEN);
    tft.println("DISPLAY PASS?");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);

    tft.println();
    tft.println("ST7789 diagnostic #4");
    tft.println();
    tft.println("Resolution test:");
    tft.printf("%d x %d\n",
               tft.width(),
               tft.height());

    tft.println();
    tft.setTextColor(ST77XX_RED);
    tft.println("RED");

    tft.setTextColor(ST77XX_GREEN);
    tft.println("GREEN");

    tft.setTextColor(ST77XX_BLUE);
    tft.println("BLUE");

    tft.setTextColor(ST77XX_YELLOW);
    tft.println("YELLOW");

    tft.setTextColor(ST77XX_CYAN);
    tft.println("CYAN");

    tft.setTextColor(ST77XX_MAGENTA);
    tft.println("MAGENTA");

    tft.setTextColor(ST77XX_WHITE);

    tft.println();
    tft.println("0123456789");
    tft.println("ABCDEFGHIJKLM");
    tft.println("NOPQRSTUVWXYZ");

    Serial.println("Text test displayed.");
}

// ============================================================
// Pixel-grid test
// ============================================================

void pixelGrid()
{
    serialSeparator();
    Serial.println("PIXEL / ADDRESSING TEST");
    serialSeparator();

    tft.fillScreen(ST77XX_BLACK);

    int w = tft.width();
    int h = tft.height();

    for (int x = 0; x < w; x += 10)
    {
        tft.drawFastVLine(
            x,
            0,
            h,
            ST77XX_BLUE
        );
    }

    for (int y = 0; y < h; y += 10)
    {
        tft.drawFastHLine(
            0,
            y,
            w,
            ST77XX_GREEN
        );
    }

    // Mark corners distinctly
    tft.fillRect(0, 0, 20, 20, ST77XX_RED);

    tft.fillRect(
        w - 20,
        0,
        20,
        20,
        ST77XX_GREEN
    );

    tft.fillRect(
        0,
        h - 20,
        20,
        20,
        ST77XX_BLUE
    );

    tft.fillRect(
        w - 20,
        h - 20,
        20,
        20,
        ST77XX_WHITE
    );

    Serial.println("Pixel grid displayed.");
}

// ============================================================
// Rotation
// ============================================================

void nextRotation()
{
    rotation++;

    if (rotation > 3)
        rotation = 0;

    tft.setRotation(rotation);

    Serial.printf(
        "Rotation = %u, size = %d x %d\n",
        rotation,
        tft.width(),
                  tft.height()
    );

    textTest();
}

// ============================================================
// Inversion
// ============================================================

void toggleInvert()
{
    inverted = !inverted;

    tft.invertDisplay(inverted);

    Serial.printf(
        "Display inversion: %s\n",
        inverted ? "ON" : "OFF"
    );
}

// ============================================================
// Full automatic sequence
// ============================================================

void fullTest()
{
    Serial.println();
    Serial.println(
        "============================================================"
    );

    Serial.println(
        "STARTING FULL DISPLAY TEST"
    );

    Serial.println(
        "============================================================"
    );

    solidColors();

    colorBars();
    delay(1500);

    geometryTest();
    delay(1500);

    pixelGrid();
    delay(1500);

    textTest();

    Serial.println();
    Serial.println("Full display test complete.");
}

// ============================================================
// Menu
// ============================================================

void printMenu()
{
    Serial.println();
    serialSeparator();
    Serial.println(
        "BYU-I eBadge v4.1.1 Diagnostic #4"
    );

    Serial.println("ST7789 Display Test");
    serialSeparator();

    Serial.println("Commands:");
    Serial.println("  a - run full automatic test");
    Serial.println("  c - solid-color test");
    Serial.println("  b - color bars");
    Serial.println("  g - geometry test");
    Serial.println("  p - pixel/address grid");
    Serial.println("  t - text test");
    Serial.println("  r - rotate display");
    Serial.println("  i - toggle inversion");
    Serial.println("  k - black screen");
    Serial.println("  w - white screen");
    Serial.println("  h - help");

    serialSeparator();
}

// ============================================================
// Setup
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1200);

    Serial.println();
    Serial.println(
        "============================================================"
    );

    Serial.println(
        " BYU-I eBadge v4.1.1 AUG26 - Diagnostic #4"
    );

    Serial.println(
        " ST7789 Display Diagnostic"
    );

    Serial.println(
        "============================================================"
    );

    Serial.println("Display pin mapping:");

    Serial.printf(
        "CS   GPIO%d\n",
        TFT_CS
    );

    Serial.printf(
        "RST  GPIO%d\n",
        TFT_RST
    );

    Serial.printf(
        "DC   GPIO%d\n",
        TFT_DC
    );

    Serial.printf(
        "MOSI GPIO%d\n",
        TFT_MOSI
    );

    Serial.printf(
        "SCLK GPIO%d\n",
        TFT_SCLK
    );

    Serial.println();
    Serial.println(
        "Initializing ST7789 240x320..."
    );

    tft.init(240, 320);

    rotation = 1;
    tft.setRotation(rotation);

    tft.invertDisplay(false);

    Serial.printf(
        "Display initialized. Logical size: %d x %d\n",
        tft.width(),
                  tft.height()
    );

    fullTest();

    printMenu();
}

// ============================================================
// Loop
// ============================================================

void loop()
{
    if (!Serial.available())
        return;

    char command = Serial.read();

    switch (command)
    {
        case 'a':
        case 'A':
            fullTest();
            break;

        case 'c':
        case 'C':
            solidColors();
            break;

        case 'b':
        case 'B':
            colorBars();
            break;

        case 'g':
        case 'G':
            geometryTest();
            break;

        case 'p':
        case 'P':
            pixelGrid();
            break;

        case 't':
        case 'T':
            textTest();
            break;

        case 'r':
        case 'R':
            nextRotation();
            break;

        case 'i':
        case 'I':
            toggleInvert();
            break;

        case 'k':
        case 'K':
            tft.fillScreen(ST77XX_BLACK);
            Serial.println("Black screen.");
            break;

        case 'w':
        case 'W':
            tft.fillScreen(ST77XX_WHITE);
            Serial.println("White screen.");
            break;

        case 'h':
        case 'H':
            printMenu();
            break;
    }
}
