#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t  Button1TaskHandle, Button2TaskHandle;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void MX_GPIO_EXTI_Enable(void);
void MX_GPIO_Init(void);
void Button1Task(void *argument);
void Button2Task(void *argument);

#endif // GPIO_H