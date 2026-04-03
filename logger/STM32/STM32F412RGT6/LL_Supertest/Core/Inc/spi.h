#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "main.h"

void spi1_cs1_high(void);
void spi1_cs1_low(void);
void spi1_cs2_high(void);
void spi1_cs2_low(void);

void dma_spi1_rx(uint32_t dst, uint32_t len);
void dma_spi1_tx(uint32_t src, uint32_t len);

#endif // SPI_H