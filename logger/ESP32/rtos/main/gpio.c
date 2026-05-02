#include "gpio.h"
#include "driver/gpio.h"
#include "esp_err.h"

TaskHandle_t led_blink_task_handle = NULL;

void init_led(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

void led_blink_task(void *pvParameters)
{
    while (1) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(100));

        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}