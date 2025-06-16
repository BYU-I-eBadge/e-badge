#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "st7735s.h"

void app_main(void) {
    printf("SPI init...\n");
    spi_device_handle_t spi = st7735s_spi_init();
    printf("Display init...\n");
    st7735s_init(spi);
    printf("Entering color loop...\n");

    while (1) {
        printf("Red\n");
        st7735s_fill_color(spi, 0xF800);
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf("Green\n");
        st7735s_fill_color(spi, 0x07E0);
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf("Blue\n");
        st7735s_fill_color(spi, 0x001F);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

