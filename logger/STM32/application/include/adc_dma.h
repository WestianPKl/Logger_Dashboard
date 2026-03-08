#ifndef ADC_DMA_H
#define ADC_DMA_H

#include <stdint.h>
#include "stm32l4xx.h"

void adc_init(uint8_t continuous_mode);
void adc_dma_init(uint8_t continuous_mode, uint16_t *dst, uint16_t len);

#endif // ADC_DMA_H