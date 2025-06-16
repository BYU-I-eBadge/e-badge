#include "st7735s.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void st7735s_send_cmd(spi_device_handle_t spi, uint8_t cmd) {
    gpio_set_level(TFT_DC, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    spi_device_transmit(spi, &t);
}

void st7735s_send_data(spi_device_handle_t spi, const uint8_t *data, int len) {
    gpio_set_level(TFT_DC, 1);
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data
    };
    spi_device_transmit(spi, &t);
}

void st7735s_init(spi_device_handle_t spi) {
    gpio_set_direction(TFT_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(TFT_RST, GPIO_MODE_OUTPUT);

    gpio_set_level(TFT_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(TFT_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    st7735s_send_cmd(spi, 0x01); // Software reset
    vTaskDelay(pdMS_TO_TICKS(150));
    st7735s_send_cmd(spi, 0x11); // Sleep out
    vTaskDelay(pdMS_TO_TICKS(120));
    st7735s_send_cmd(spi, 0x29); // Display ON
}

static void set_window(spi_device_handle_t spi, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    uint8_t data[4];

    st7735s_send_cmd(spi, 0x2A);  // CASET (Column Address Set)
    data[0] = 0x00; data[1] = x0;
    data[2] = 0x00; data[3] = x1;
    st7735s_send_data(spi, data, 4);

    st7735s_send_cmd(spi, 0x2B);  // RASET (Row Address Set)
    data[0] = 0x00; data[1] = y0;
    data[2] = 0x00; data[3] = y1;
    st7735s_send_data(spi, data, 4);

    st7735s_send_cmd(spi, 0x2C);  // RAMWR
}

void st7735s_fill_color(spi_device_handle_t spi, uint16_t color) {
    set_window(spi, 0, 0, 127, 159);
    gpio_set_level(TFT_DC, 1); // Data mode

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    uint8_t buf[128 * 2]; // One row (128 pixels)

    for (int i = 0; i < 128; ++i) {
        buf[2 * i] = hi;
        buf[2 * i + 1] = lo;
    }

    for (int y = 0; y < 160; ++y) {
        spi_transaction_t t = {
            .length = 128 * 2 * 8,
            .tx_buffer = buf
        };
        spi_device_transmit(spi, &t);
    }
}
