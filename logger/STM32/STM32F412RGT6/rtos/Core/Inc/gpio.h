#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t  Button1TaskHandle, Button2TaskHandle;

/*
    * @brief  HAL EXTI callback dispatching button and MFP interrupts to the appropriate FreeRTOS tasks.
    * @param  GPIO_Pin: The pin that triggered the interrupt.
    * @retval None
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

/*
    * @brief  Enable EXTI interrupts for BTN1, BTN2, and the MCP7940N MFP pin.
    * @retval None
*/
void MX_GPIO_EXTI_Enable(void);

/*
    * @brief  Initialize all GPIO pins: outputs, inputs, and EXTI lines.
    * @retval None
*/
void MX_GPIO_Init(void);

/*
    * @brief  FreeRTOS task handling button 1 press events.
    *         Toggles LED1 and debounces the button input.
    * @param  argument: Unused task parameter.
    * @retval None
*/
void Button1Task(void *argument);

/*
    * @brief  FreeRTOS task handling button 2 press events.
    *         Activates the LCD backlight and starts the backlight timer.
    * @param  argument: Unused task parameter.
    * @retval None
*/
void Button2Task(void *argument);

#endif // GPIO_H