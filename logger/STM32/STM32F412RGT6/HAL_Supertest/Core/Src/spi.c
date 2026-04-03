#include "spi.h"
#include "main.h"
#include "stm32f4xx_hal.h"

void spi1_cs1_high(void)
{
    HAL_GPIO_WritePin(SPI1_CS1_GPIO_Port, SPI1_CS1_Pin, GPIO_PIN_SET);
}

void spi1_cs1_low(void)
{
    HAL_GPIO_WritePin(SPI1_CS1_GPIO_Port, SPI1_CS1_Pin, GPIO_PIN_RESET);
}

void spi1_cs2_high(void)
{
    HAL_GPIO_WritePin(SPI1_CS2_GPIO_Port, SPI1_CS2_Pin, GPIO_PIN_SET);
}

void spi1_cs2_low(void)
{
    HAL_GPIO_WritePin(SPI1_CS2_GPIO_Port, SPI1_CS2_Pin, GPIO_PIN_RESET);
}

HAL_StatusTypeDef spi1_tx_blocking(const uint8_t *src, uint16_t len, uint32_t timeout)
{
    if ((src == NULL) || (len == 0U)) {
        return HAL_ERROR;
    }

    return HAL_SPI_Transmit(&hspi1, (uint8_t *)src, len, timeout);
}

HAL_StatusTypeDef spi1_rx_blocking(uint8_t *dst, uint16_t len, uint32_t timeout)
{
    if ((dst == NULL) || (len == 0U)) {
        return HAL_ERROR;
    }

    return HAL_SPI_Receive(&hspi1, dst, len, timeout);
}

HAL_StatusTypeDef spi1_txrx_blocking(const uint8_t *tx, uint8_t *rx, uint16_t len, uint32_t timeout)
{
    if ((tx == NULL) || (rx == NULL) || (len == 0U)) {
        return HAL_ERROR;
    }

    return HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)tx, rx, len, timeout);
}