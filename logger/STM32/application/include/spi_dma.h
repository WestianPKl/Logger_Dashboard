#ifndef SPI_DMA_H
#define SPI_DMA_H

#include <stdint.h>
#include "stm32l4xx.h"

void spi1_init(void);
void spi1_cs_low(void);
void spi1_cs_high(void);

#endif // SPI_DMA_H