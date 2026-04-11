#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <stdint.h>
#include "main.h"

void handle_request(const uint8_t *req, uint8_t use_uart1);

#endif // UART_PROTOCOL_H