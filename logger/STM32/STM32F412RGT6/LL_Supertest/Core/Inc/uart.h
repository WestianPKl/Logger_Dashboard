#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "main.h"


void dma1_uart2_rx_config(uint8_t *dst, uint16_t len);
void dma1_uart2_tx_start(uint8_t *src, uint16_t len);

void dma2_uart1_rx_config(uint8_t *dst, uint16_t len);
void dma2_uart1_tx_start(uint8_t *src, uint16_t len);

#endif // UART_H