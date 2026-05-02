#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <stdint.h>
#include "main.h"
#include "uart.h"

#define UART_RX_MSG_MAX_LEN ((UART1_RX_BUFFER_SIZE > UART2_RX_BUFFER_SIZE) ? UART1_RX_BUFFER_SIZE : UART2_RX_BUFFER_SIZE)

/*
    * @brief  Structure for passing a received UART frame through the queue.
*/
typedef struct {
    uint8_t data[UART_RX_MSG_MAX_LEN];
    uint16_t len;
    uint8_t use_uart1;
} uart_rx_msg_t;

/*
    * @brief  Dispatch a validated UART request frame and send the appropriate response.
    * @param  req: Pointer to the request frame (at least FRAME_LEN_APP bytes).
    * @param  use_uart1: Non-zero to respond via USART1, 0 for USART2.
*/
void handle_request(const uint8_t *req, uint8_t use_uart1);

#endif // UART_PROTOCOL_H