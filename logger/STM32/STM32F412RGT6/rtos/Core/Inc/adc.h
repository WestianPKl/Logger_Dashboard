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

/*
    * @brief  Initialize the ADC1 peripheral with the desired settings.
    *         This function configures ADC1 for continuous conversion mode with DMA support, sets up the channel configurations for the specified ADC channels,
    *         and starts the ADC conversion process. The function also disables DMA interrupts for transfer complete, half transfer, and transfer error to allow for polling-based handling.
    * @retval None
*/
void MX_ADC1_Init(void);

#endif // ADC_H