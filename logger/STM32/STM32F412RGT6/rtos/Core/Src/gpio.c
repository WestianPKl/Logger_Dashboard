#include "gpio.h"
#include "app_flags.h"
#include "lcd.h"

TaskHandle_t Button1TaskHandle, Button2TaskHandle;

void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOC, FLASH_Hold_Pin|CON_1_Pin|CON_2_Pin|CON_3_Pin
                          |CON_4_Pin|ESP32_Write_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, ESP_GPIO01_Pin|CON_5_Pin|ESP32_Reset_Pin|LED1_Pin
                          |LED2_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(SPI1_CS2_GPIO_Port, SPI1_CS2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SPI1_CS1_GPIO_Port, SPI1_CS1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, FLASH_Hold_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = FLASH_Hold_Pin|CON_1_Pin|CON_2_Pin|CON_3_Pin
                          |CON_4_Pin|ESP32_Write_Pin|SPI1_CS2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SPI1_CS1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_CS1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = ESP32_Status_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ESP32_Status_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BTN1_Pin|BTN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = ESP_GPIO01_Pin|CON_5_Pin|ESP32_Reset_Pin|LED1_Pin
                          |LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = MFP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MFP_GPIO_Port, &GPIO_InitStruct);
}

void MX_GPIO_EXTI_Enable(void)
{
  __HAL_GPIO_EXTI_CLEAR_IT(BTN1_Pin);
  __HAL_GPIO_EXTI_CLEAR_IT(BTN2_Pin);
  __HAL_GPIO_EXTI_CLEAR_IT(MFP_Pin);

  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (GPIO_Pin == BTN1_Pin) {
        if (Button1TaskHandle != NULL) {
            xTaskNotifyFromISR(Button1TaskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
        }
    }
    else if (GPIO_Pin == BTN2_Pin) {
        if (Button2TaskHandle != NULL) {
            xTaskNotifyFromISR(Button2TaskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
        }
    }
    else if (GPIO_Pin == MFP_Pin) {
        if (MeasureTaskHandle != NULL) {
            vTaskNotifyGiveFromISR(MeasureTaskHandle, &xHigherPriorityTaskWoken);
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void Button1Task(void *argument)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        app_io_state.btn1_pressed = 0U;

        if (HAL_GPIO_ReadPin(GPIOB, BTN1_Pin) == GPIO_PIN_RESET)
        {
            app_io_state.btn1_pressed = 1U;
            HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
            app_io_state.led1_state =
                (HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin) == GPIO_PIN_SET) ? 1U : 0U;

            vTaskDelay(pdMS_TO_TICKS(10));

            while (HAL_GPIO_ReadPin(GPIOB, BTN1_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            app_io_state.btn1_pressed = 0U;
        }
    }
}

void Button2Task(void *argument)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        app_io_state.btn2_pressed = 0U;

        if (HAL_GPIO_ReadPin(GPIOB, BTN2_Pin) == GPIO_PIN_RESET)
        {
            app_io_state.btn2_pressed = 1U;

            if (xEventGroupGetBits(appEvents) & EVT_LCD_PRESENT)
            {
                if (lcdCmdQueue != NULL)
                {
                  lcd_msg_t msg = { .cmd = LCD_BACKLIGHT, .flag = 1U };
                  (void)xQueueSend(lcdCmdQueue, &msg, 0);

                  if (LCDTaskHandle != NULL) {
                      xTaskNotify(LCDTaskHandle, LCD_NOTIFY_COMMAND, eSetBits);
                  }
                }
                (void)xTimerReset(backlightTimerHandle, 0);
            }

            vTaskDelay(pdMS_TO_TICKS(10));

            while (HAL_GPIO_ReadPin(GPIOB, BTN2_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            app_io_state.btn2_pressed = 0U;
        }
    }
}