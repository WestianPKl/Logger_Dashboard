#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <stdint.h>
#include "main.h"
#include "uart.h"

#define UART_RX_MSG_MAX_LEN ((UART1_RX_BUFFER_SIZE > UART2_RX_BUFFER_SIZE) ? UART1_RX_BUFFER_SIZE : UART2_RX_BUFFER_SIZE)

typedef struct {
    uint8_t data[UART_RX_MSG_MAX_LEN];
    uint16_t len;
    uint8_t use_uart1;
} uart_rx_msg_t;

void handle_request(const uint8_t *req, uint8_t use_uart1);

#endif // UART_PROTOCOL_H