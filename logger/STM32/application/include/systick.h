#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>
#include "stm32l4xx.h"

void systick_delay_ms(uint32_t ms);
void systick_irq(void);

#endif // SYSTICK_H