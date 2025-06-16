#include "st7735s.h"
#include "driver/gpio.h"
#include "freertos/task.h"

static spi_device_handle_t spi = NULL;

static void st7735s_send_cmd(spi_device_handle_t spi, uint8_t cmd) {
    gpio_set_level(TFT_DC, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    spi_device_transmit(spi, &t);
}

static void st7735s_send_data(spi_device_handle_t spi, const uint8_t *data, int len) {
    gpio_set_level(TFT_DC, 1);
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data
    };
    spi_device_transmit(spi, &t);
}

static void st7735s_reset(void) {
    gpio_set_level(TFT_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(TFT_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void st7735s_set_window(spi_device_handle_t spi, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    uint8_t data[4];

    st7735s_send_cmd(spi, 0x2A); // CASET
    data[0] = 0x00; data[1] = x0;
    data[2] = 0x00; data[3] = x1;
    st7735s_send_data(spi, data, 4);

    st7735s_send_cmd(spi, 0x2B); // RASET
    data[0] = 0x00; data[1] = y0;
    data[2] = 0x00; data[3] = y1;
    st7735s_send_data(spi, data, 4);

    st7735s_send_cmd(spi, 0x2C); // RAMWR
}

spi_device_handle_t st7735s_spi_init(void) {
    spi_bus_config_t buscfg = {
        .mosi_io_num = TFT_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = TFT_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TFT_WIDTH * TFT_HEIGHT * 2,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = TFT_CS,
        .queue_size = 7,
    };

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TFT_DC) | (1ULL << TFT_RST),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi);

    st7735s_reset();

    return spi;
}

void st7735s_init(spi_device_handle_t spi) {
    st7735s_send_cmd(spi, 0x01); // SWRESET
    vTaskDelay(pdMS_TO_TICKS(150));

    st7735s_send_cmd(spi, 0x11); // SLPOUT
    vTaskDelay(pdMS_TO_TICKS(150));

    st7735s_send_cmd(spi, 0x3A); // COLMOD
    uint8_t color_mode = 0x05;   // 16-bit/pixel RGB565
    st7735s_send_data(spi, &color_mode, 1);

    st7735s_send_cmd(spi, 0x36); // MADCTL (rotation)
    uint8_t madctl = 0x00;       // adjust rotation here if needed
    st7735s_send_data(spi, &madctl, 1);

    st7735s_send_cmd(spi, 0x29); // DISPON
    vTaskDelay(pdMS_TO_TICKS(100));
}

void st7735s_fill_color(spi_device_handle_t spi, uint16_t color) {
    st7735s_set_window(spi, 0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    uint8_t row[TFT_WIDTH * 2];

    for (int i = 0; i < TFT_WIDTH; i++) {
        row[2 * i] = hi;
        row[2 * i + 1] = lo;
    }

    for (int y = 0; y < TFT_HEIGHT; y++) {
        spi_transaction_t t = {
            .length = TFT_WIDTH * 2 * 8,
            .tx_buffer = row
        };
        spi_device_transmit(spi, &t);
    }
}
