#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

#define LED_GPIO        GPIO_NUM_2
#define JOY_BTN_GPIO    GPIO_NUM_0
#define JOY_X_GPIO      GPIO_NUM_4
#define JOY_Y_GPIO      GPIO_NUM_5

void app_main(void) {
    // LED setup
    gpio_config_t led_conf = {
        .pin_bit_mask = 1ULL << LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&led_conf);

    // Joystick button setup
    gpio_config_t btn_conf = {
        .pin_bit_mask = 1ULL << JOY_BTN_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn_conf);

    // ADC setup (ESP32-C6 uses ADC1, width fixed to 12-bit)
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };

    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_2, &chan_cfg); // GPIO4
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &chan_cfg); // GPIO5

    int led_state = 0;

    while (1) {
        int joy_x = 0, joy_y = 0;
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_2, &joy_x);
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &joy_y);
        int btn_state = gpio_get_level(JOY_BTN_GPIO);

        if (joy_x > 3000) {
            gpio_set_level(LED_GPIO, 1);
            led_state = 1;
        } else if (joy_x < 1000) {
            gpio_set_level(LED_GPIO, 0);
            led_state = 0;
        }

        if (btn_state == 0) {
            led_state = !led_state;
            gpio_set_level(LED_GPIO, led_state);
            vTaskDelay(pdMS_TO_TICKS(300));  // debounce
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
