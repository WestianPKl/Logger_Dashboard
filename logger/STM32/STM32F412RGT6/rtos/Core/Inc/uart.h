#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "main.h"
#include "stm32f412rx.h"
#include "stm32f4xx_hal_uart.h"
#include "FreeRTOS.h"
#include "task.h"

extern uint8_t uart2_rx_buf[UART2_RX_BUFFER_SIZE];
extern uint8_t uart1_rx_buf[UART1_RX_BUFFER_SIZE];

extern uint8_t uart2_tx_frame[FRAME_LEN_APP];
extern uint8_t uart1_tx_frame[FRAME_LEN_APP];

extern volatile uint8_t uart1_tx_busy;
extern volatile uint8_t uart2_tx_busy;

void uart_feed_bytes(const uint8_t *data, uint16_t len, uint8_t use_uart1);
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);

#endif // UART_H