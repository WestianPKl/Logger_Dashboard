#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "stm32l4xx.h"

void tim1_init(uint32_t prescaler, uint32_t period);
void tim2_pa0_pa1_pwm_init(uint32_t prescaler, uint32_t period);
void tim2_pa0_pa1_pwm_set_duty(uint8_t channel, int32_t duty);

#endif // TIMER_H