/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_adc.h"
#include "stm32f4xx_ll_crc.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_rtc.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC_CHANNEL_COUNT 2
#define ADC_SAMPLES_PER_CHANNEL 1
#define ADC_BUFFER_SIZE (ADC_CHANNEL_COUNT * ADC_SAMPLES_PER_CHANNEL)
#define FRAME_LEN_APP 32
#define FRAME_HEADER_SIZE 5
#define FRAME_PAYLOAD_SIZE (FRAME_LEN_APP - FRAME_HEADER_SIZE)
#define UART1_RX_BUFFER_SIZE 128
#define UART1_RX_FRAME_LEN FRAME_LEN_APP
#define UART2_RX_BUFFER_SIZE 128
#define UART2_RX_FRAME_LEN FRAME_LEN_APP
#define FLASH_Hold_Pin LL_GPIO_PIN_13
#define FLASH_Hold_GPIO_Port GPIOC
#define CON_1_Pin LL_GPIO_PIN_0
#define CON_1_GPIO_Port GPIOC
#define CON_2_Pin LL_GPIO_PIN_1
#define CON_2_GPIO_Port GPIOC
#define CON_3_Pin LL_GPIO_PIN_2
#define CON_3_GPIO_Port GPIOC
#define CON_4_Pin LL_GPIO_PIN_3
#define CON_4_GPIO_Port GPIOC
#define SPI1_CS1_Pin LL_GPIO_PIN_4
#define SPI1_CS1_GPIO_Port GPIOA
#define ESP32_Write_Pin LL_GPIO_PIN_4
#define ESP32_Write_GPIO_Port GPIOC
#define ESP32_Status_Pin LL_GPIO_PIN_5
#define ESP32_Status_GPIO_Port GPIOC
#define BTN1_Pin LL_GPIO_PIN_0
#define BTN1_GPIO_Port GPIOB
#define BTN2_Pin LL_GPIO_PIN_1
#define BTN2_GPIO_Port GPIOB
#define ESP_GPIO01_Pin LL_GPIO_PIN_2
#define ESP_GPIO01_GPIO_Port GPIOB
#define PWM_CON_2_Pin LL_GPIO_PIN_10
#define PWM_CON_2_GPIO_Port GPIOB
#define CON_5_Pin LL_GPIO_PIN_12
#define CON_5_GPIO_Port GPIOB
#define ESP32_Reset_Pin LL_GPIO_PIN_13
#define ESP32_Reset_GPIO_Port GPIOB
#define LED1_Pin LL_GPIO_PIN_14
#define LED1_GPIO_Port GPIOB
#define LED2_Pin LL_GPIO_PIN_15
#define LED2_GPIO_Port GPIOB
#define RGB_Blue_Pin LL_GPIO_PIN_6
#define RGB_Blue_GPIO_Port GPIOC
#define RGB_Green_Pin LL_GPIO_PIN_7
#define RGB_Green_GPIO_Port GPIOC
#define RGB_Red_Pin LL_GPIO_PIN_8
#define RGB_Red_GPIO_Port GPIOC
#define Buzzer_Pin LL_GPIO_PIN_9
#define Buzzer_GPIO_Port GPIOC
#define PWM_CON_1_Pin LL_GPIO_PIN_8
#define PWM_CON_1_GPIO_Port GPIOA
#define SPI1_CS2_Pin LL_GPIO_PIN_12
#define SPI1_CS2_GPIO_Port GPIOC
#define MFP_Pin LL_GPIO_PIN_5
#define MFP_GPIO_Port GPIOB
#define PWM_CON_3_Pin LL_GPIO_PIN_8
#define PWM_CON_3_GPIO_Port GPIOB
#define PWM_CON_4_Pin LL_GPIO_PIN_9
#define PWM_CON_4_GPIO_Port GPIOB

extern CAN_HandleTypeDef hcan1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
