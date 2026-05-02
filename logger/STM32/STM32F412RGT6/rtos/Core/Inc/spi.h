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

/*
    * @brief  Initialize SPI1 peripheral in master mode with DMA support.
    *         Configures 8-bit data, CPOL=0 CPHA=0, software NSS, prescaler /16.
    * @retval None
*/
void MX_SPI1_Init(void);

/*
    * @brief  Block the calling FreeRTOS task until the current SPI transfer completes or a timeout occurs.
    * @param  timeoutTicks: Maximum number of RTOS ticks to wait.
    * @retval 1 on success, -1 on timeout or SPI error.
*/
int8_t wait_spi_done(TickType_t timeoutTicks);

/*
    * @brief  HAL callback invoked when a full-duplex SPI DMA transfer completes.
    * @param  hspi: Pointer to the SPI handle.
    * @retval None
*/
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);

/*
    * @brief  HAL callback invoked when a transmit-only SPI DMA transfer completes.
    * @param  hspi: Pointer to the SPI handle.
    * @retval None
*/
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);

/*
    * @brief  HAL callback invoked when a receive-only SPI DMA transfer completes.
    * @param  hspi: Pointer to the SPI handle.
    * @retval None
*/
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);

/*
    * @brief  HAL callback invoked when an SPI error occurs during DMA transfer.
    *         Sets the spiError flag and notifies the owner task.
    * @param  hspi: Pointer to the SPI handle.
    * @retval None
*/
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);

/*
    * @brief  Drive the SPI1 chip-select 1 pin high (deselect BME280).
    * @retval None
*/
void spi1_cs1_high(void);

/*
    * @brief  Drive the SPI1 chip-select 1 pin low (select BME280).
    * @retval None
*/
void spi1_cs1_low(void);

/*
    * @brief  Drive the SPI1 chip-select 2 pin high (deselect external flash).
    * @retval None
*/
void spi1_cs2_high(void);

/*
    * @brief  Drive the SPI1 chip-select 2 pin low (select external flash).
    * @retval None
*/
void spi1_cs2_low(void);

#endif // SPI_H