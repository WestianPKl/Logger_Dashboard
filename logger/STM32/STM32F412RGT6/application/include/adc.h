#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "stm32f4xx.h"

void adc1_init(uint8_t continuous_mode, uint16_t *dst, uint16_t len);
void adc1_start_conversion(void);
void adc1_stop_conversion(void);
uint8_t adc1_is_conversion_complete(void);
void adc1_clear_complete_flag(void);

#endif // ADC_H