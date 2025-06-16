#ifndef ST7735S_H
#define ST7735S_H

#include "driver/spi_master.h"
#include "driver/gpio.h"

#define TFT_CS     8
#define TFT_DC     7
#define TFT_RST    6
#define TFT_CLK    9
#define TFT_MOSI   10 // SDA

void st7735s_init(spi_device_handle_t spi);
void st7735s_send_cmd(spi_device_handle_t spi, uint8_t cmd);
void st7735s_send_data(spi_device_handle_t spi, const uint8_t *data, int len);
void st7735s_fill_color(spi_device_handle_t spi, uint16_t color);


#endif
