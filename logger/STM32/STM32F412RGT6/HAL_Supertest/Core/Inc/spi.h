#ifndef SPI_H
#define SPI_H

#include "main.h"

void spi1_cs1_high(void);
void spi1_cs1_low(void);
void spi1_cs2_high(void);
void spi1_cs2_low(void);

HAL_StatusTypeDef spi1_tx_blocking(const uint8_t *src, uint16_t len, uint32_t timeout);
HAL_StatusTypeDef spi1_rx_blocking(uint8_t *dst, uint16_t len, uint32_t timeout);
HAL_StatusTypeDef spi1_txrx_blocking(const uint8_t *tx, uint8_t *rx, uint16_t len, uint32_t timeout);

#endif