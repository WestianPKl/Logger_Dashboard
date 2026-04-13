#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "sht40.h"
#include "bme280.h"
#include "rtc.h"

void Error_Handler(void);
void measure_trigger_update(void);

#define FLASH_Hold_Pin GPIO_PIN_13
#define FLASH_Hold_GPIO_Port GPIOC
#define CON_1_Pin GPIO_PIN_0
#define CON_1_GPIO_Port GPIOC
#define CON_2_Pin GPIO_PIN_1
#define CON_2_GPIO_Port GPIOC
#define CON_3_Pin GPIO_PIN_2
#define CON_3_GPIO_Port GPIOC
#define CON_4_Pin GPIO_PIN_3
#define CON_4_GPIO_Port GPIOC
#define SPI1_CS1_Pin GPIO_PIN_4
#define SPI1_CS1_GPIO_Port GPIOA
#define ESP32_Write_Pin GPIO_PIN_4
#define ESP32_Write_GPIO_Port GPIOC
#define ESP32_Status_Pin GPIO_PIN_5
#define ESP32_Status_GPIO_Port GPIOC
#define BTN1_Pin GPIO_PIN_0
#define BTN1_GPIO_Port GPIOB
#define BTN2_Pin GPIO_PIN_1
#define BTN2_GPIO_Port GPIOB
#define ESP_GPIO01_Pin GPIO_PIN_2
#define ESP_GPIO01_GPIO_Port GPIOB
#define PWM_CON_2_Pin GPIO_PIN_10
#define PWM_CON_2_GPIO_Port GPIOB
#define CON_5_Pin GPIO_PIN_12
#define CON_5_GPIO_Port GPIOB
#define ESP32_Reset_Pin GPIO_PIN_13
#define ESP32_Reset_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_14
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_15
#define LED2_GPIO_Port GPIOB
#define RGB_Blue_Pin GPIO_PIN_6
#define RGB_Blue_GPIO_Port GPIOC
#define RGB_Green_Pin GPIO_PIN_7
#define RGB_Green_GPIO_Port GPIOC
#define RGB_Red_Pin GPIO_PIN_8
#define RGB_Red_GPIO_Port GPIOC
#define Buzzer_Pin GPIO_PIN_9
#define Buzzer_GPIO_Port GPIOC
#define PWM_CON_1_Pin GPIO_PIN_8
#define PWM_CON_1_GPIO_Port GPIOA
#define SPI1_CS2_Pin GPIO_PIN_12
#define SPI1_CS2_GPIO_Port GPIOC
#define MFP_Pin GPIO_PIN_5
#define MFP_GPIO_Port GPIOB
#define PWM_CON_3_Pin GPIO_PIN_8
#define PWM_CON_3_GPIO_Port GPIOB
#define PWM_CON_4_Pin GPIO_PIN_9
#define PWM_CON_4_GPIO_Port GPIOB


extern sht40_data_t sht40_data;
extern bme280_data_t bme280_data;
extern rtc_date_time_t rtc_date_time;

extern QueueHandle_t framCmdQueue;
extern SemaphoreHandle_t sensorDataMutex;

extern TaskHandle_t SensorTaskHandle, MeasureTaskHandle, FRAMTaskHandle;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
