#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <stdint.h>
#include "main.h"

#define FRAME_PAYLOAD   (FRAME_LEN_APP - FRAME_HEADER_SIZE)

extern uint8_t uart2_rx_buf[UART2_RX_BUFFER_SIZE];
extern uint8_t uart1_rx_buf[UART1_RX_BUFFER_SIZE];

extern uint8_t uart2_tx_frame[FRAME_LEN_APP];
extern uint8_t uart1_tx_frame[FRAME_LEN_APP];

extern uint16_t adc_data_buffer[ADC_BUFFER_SIZE];


#endif // UART_PROTOCOL_H