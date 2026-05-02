#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

#define FRAME_LEN_APP 32
#define FRAME_HEADER_SIZE 5
#define FRAME_PAYLOAD_SIZE (FRAME_LEN_APP - FRAME_HEADER_SIZE)
#define UART1_RX_BUFFER_SIZE 128
#define UART1_RX_FRAME_LEN FRAME_LEN_APP
#define UART2_RX_BUFFER_SIZE 128
#define UART2_RX_FRAME_LEN FRAME_LEN_APP

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

extern QueueHandle_t uartRxQueue;

extern uint8_t uart2_rx_buf[UART2_RX_BUFFER_SIZE];
extern uint8_t uart1_rx_buf[UART1_RX_BUFFER_SIZE];

extern uint8_t uart2_tx_frame[FRAME_LEN_APP];
extern uint8_t uart1_tx_frame[FRAME_LEN_APP];

extern volatile uint8_t uart1_tx_busy;
extern volatile uint8_t uart2_tx_busy;

extern TaskHandle_t ReceiverTaskHandle;

/*
    * @brief  Initialize USART1 peripheral (115200 8N1).
*/
void MX_USART1_UART_Init(void);

/*
    * @brief  Initialize USART2 peripheral (115200 8N1), disable DMA half-transfer interrupt.
*/
void MX_USART2_UART_Init(void);

/*
    * @brief  FreeRTOS task that dequeues received UART frames and dispatches them via uart_feed_bytes.
    * @param  argument: Unused.
*/
void ReceiverTask(void *argument);

/*
    * @brief  Validate an incoming frame (address, CRC) and forward it to handle_request.
    * @param  data: Pointer to the received frame data.
    * @param  len: Number of received bytes.
    * @param  use_uart1: Non-zero if the frame came from USART1, 0 for USART2.
*/
void uart_feed_bytes(const uint8_t *data, uint16_t len, uint8_t use_uart1);

/*
    * @brief  HAL UART receive-to-idle DMA callback. Copies received data into the queue.
    * @param  huart: Pointer to the UART handle.
    * @param  Size: Number of bytes received.
*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);

/*
    * @brief  HAL UART transmit-complete callback. Clears the TX busy flag.
    * @param  huart: Pointer to the UART handle.
*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

/*
    * @brief  HAL UART error callback. Clears the TX busy flag and restarts DMA reception.
    * @param  huart: Pointer to the UART handle.
*/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);

#endif // UART_H