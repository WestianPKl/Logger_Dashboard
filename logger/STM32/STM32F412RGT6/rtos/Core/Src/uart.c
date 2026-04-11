#include "uart.h"
#include "uart_protocol.h"
#include "FreeRTOS.h"
#include "task.h"
#include "version.h"
#include <string.h>
#include "support.h"

volatile uint8_t uart1_tx_busy = 0U;
volatile uint8_t uart2_tx_busy = 0U;

void uart_feed_bytes(const uint8_t *data, uint16_t len, uint8_t use_uart1)
{
    if (data == NULL || len < 2U) {
        return;
    }

    if (len > FRAME_LEN_APP) {
        return;
    }

    if ((data[0] != DEV_ADDR)) {
        return;
    }

    if (data[len - 1] != crc8_atm(data, len - 1)) {
        return;
    }
    handle_request(data, use_uart1);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uart_rx_msg_t msg = {0};

    if (huart->Instance == USART1)
    {
        if (Size > UART1_RX_BUFFER_SIZE) Size = UART1_RX_BUFFER_SIZE;

        memcpy(msg.data, uart1_rx_buf, Size);
        msg.len = Size;
        msg.use_uart1 = 1U;

        (void)xQueueSendFromISR(uartRxQueue, &msg, &xHigherPriorityTaskWoken);

        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_rx_buf, UART1_RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    }
    else if (huart->Instance == USART2)
    {
        if (Size > UART2_RX_BUFFER_SIZE) Size = UART2_RX_BUFFER_SIZE;

        memcpy(msg.data, uart2_rx_buf, Size);
        msg.len = Size;
        msg.use_uart1 = 0U;

        (void)xQueueSendFromISR(uartRxQueue, &msg, &xHigherPriorityTaskWoken);

        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart2_rx_buf, UART2_RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uart1_tx_busy = 0U;
    }
    else if (huart->Instance == USART2)
    {
        uart2_tx_busy = 0U;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uart1_tx_busy = 0U;
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_rx_buf, UART1_RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    }
    else if (huart->Instance == USART2)
    {
        uart2_tx_busy = 0U;
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart2_rx_buf, UART2_RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
    }
}