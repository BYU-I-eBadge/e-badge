#include <stdio.h>
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "st7735s.h"

spi_device_handle_t spi;

void spi_init() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = TFT_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = TFT_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = TFT_CS,
        .queue_size = 7,
    };

    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
}

void app_main(void) {
    spi_init();
    st7735s_init(spi);

    while (1) {
        st7735s_fill_color(spi, 0xF800); // Red
        vTaskDelay(pdMS_TO_TICKS(1000));

        st7735s_fill_color(spi, 0x07E0); // Green
        vTaskDelay(pdMS_TO_TICKS(1000));

        st7735s_fill_color(spi, 0x001F); // Blue
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

