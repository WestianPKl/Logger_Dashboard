#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;

extern SemaphoreHandle_t spiMutex;
extern TaskHandle_t spiOwnerTask;

extern volatile uint8_t spiError;

void MX_SPI1_Init(void);
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