
#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GPIO               22

extern TaskHandle_t led_blink_task_handle;

/*
    * @brief  Configure the LED GPIO pin as a push-pull output.
*/
void init_led(void);

/*
    * @brief  FreeRTOS task that blinks the status LED (100 ms on, 500 ms off).
    * @param  pvParameters: Unused.
*/
void led_blink_task(void *pvParameters);

#endif // GPIO_H