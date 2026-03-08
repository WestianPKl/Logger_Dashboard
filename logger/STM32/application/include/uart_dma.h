#ifndef UART_DMA_H
#define UART_DMA_H

#include <stdint.h>
#include "stm32l4xx.h"

void uart1_rxtx_init(void);
void uart2_rxtx_init(void);

#endif // UART_DMA_H