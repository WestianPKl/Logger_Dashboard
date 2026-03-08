#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BTN_GPIO 4

static const char *TAG = "GPIO_IRQ";
static volatile bool fired = false;

static void IRAM_ATTR btn_isr(void *arg) {
    (void)arg;
    fired = true; // ISR krótki: tylko flaga
}

void app_main(void) {
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BTN_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,                 // przycisk do GND
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BTN_GPIO, btn_isr, NULL);

    while (1) {
        if (fired) {
            fired = false;
            ESP_LOGI(TAG, "Interrupt!");
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}