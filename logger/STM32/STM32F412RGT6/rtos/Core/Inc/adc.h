#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "main.h"

#define ADC_CHANNEL_COUNT 2
#define ADC_SAMPLES_PER_CHANNEL 1
#define ADC_BUFFER_SIZE (ADC_CHANNEL_COUNT * ADC_SAMPLES_PER_CHANNEL)

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern volatile uint16_t adc_data_buffer[ADC_BUFFER_SIZE];

void MX_ADC1_Init(void);

#endif // ADC_H