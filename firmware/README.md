# Alpha-board Test Code (ESP32-C6 + PIO)

This folder contains test code for the **Alpha-board** using the ESP32-C6-WROOM-1 development module (not mini) and the Espressif framework in C, configured via **PlatformIO** (PIO).

---

## Project: `SPI_test_PIO`

### Description:
This project demonstrates basic SPI communication using an external **1.8" RGB TFT display** with a **ST7735S** controller.

### Hardware:
- **ESP32-C6-WROOM-1** (standard Dev Module)
- **Display:**
  - 1.8" TFT
  - Resolution: 128x160 pixels
  - Controller IC: ST7735S
  - Interface: SPI

### Notes:
- The project is built using the Espressif IDF framework.
- SPI interface pins are defined in `main.c`.
- ST7735S driver already included in the project.
- Ensure the display operates at 3.3V logic level.

---

## PlatformIO Configuration

Ensure your `platformio.ini` file contains:

```ini
[env:esp32-c6]
platform = espressif32
board = esp32-c6-devkitc-1
framework = espidf
monitor_speed = 115200
