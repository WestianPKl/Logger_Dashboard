#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "main.h"
#include "stm32f412rx.h"
#include "stm32f4xx_hal_spi.h"
#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t spiOwnerTask;
extern volatile uint8_t spiError;

int8_t wait_spi_done(TickType_t timeoutTicks);
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);

void spi1_cs1_high(void);
void spi1_cs1_low(void);
void spi1_cs2_high(void);
void spi1_cs2_low(void);

#endif // SPI_H