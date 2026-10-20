BYU-I eBadge v4.1.1 AUG26 - Standalone Hardware Diagnostic

Target:
  ESP32-S3-MINI-1-N4R2
  Arduino-ESP32 core 3.3.12

Navigation:
  UP / DOWN : select menu item
  A         : run selected test
  B         : exit active test and return to menu

Libraries:
  Adafruit GFX Library
  Adafruit ST7735 and ST7789 Library
  Adafruit NeoPixel

Install libraries:
  arduino-cli lib install "Adafruit GFX Library"
  arduino-cli lib install "Adafruit ST7735 and ST7789 Library"
  arduino-cli lib install "Adafruit NeoPixel"

Compile:
  arduino-cli compile \
    --fqbn 'esp32:esp32:esp32s3:UploadSpeed=460800,PSRAM=enabled' \
    ~/ebadge_v411_student_diagnostic

Upload through USB-UART:
  arduino-cli upload \
    --port /dev/ttyUSB0 \
    --fqbn 'esp32:esp32:esp32s3:UploadSpeed=460800,PSRAM=enabled' \
    ~/ebadge_v411_student_diagnostic

Optional serial monitor:
  picocom --imap lfcrlf -b 115200 /dev/ttyUSB0

WiFi test:
  SSID: Ebadge_wifi_test
  Password: password

BLE test:
  Advertising name: Ebadge_BLE_Test

Version 1.0.1 fix:
- Moved AppState, PixelPhase, and DisplayPattern enum declarations above the
  sketch's first function definition. Arduino's sketch preprocessor can insert
  auto-generated function prototypes before later enum declarations, which
  caused the original PixelPhase/AppState compile errors.
- fixed funky invertion
