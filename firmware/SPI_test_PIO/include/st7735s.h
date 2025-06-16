#pragma once
#include "driver/spi_master.h"

#define TFT_WIDTH  128
#define TFT_HEIGHT 160

// Pins
#define TFT_MOSI 10 //SDA
#define TFT_SCLK 9
#define TFT_CS   8
#define TFT_DC   7
#define TFT_RST  6

void st7735s_init(spi_device_handle_t spi);
void st7735s_fill_color(spi_device_handle_t spi, uint16_t color);
spi_device_handle_t st7735s_spi_init(void);
