#include "uart.h"
#include "main.h"
#include "app_flags.h"
#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

extern uint8_t uart1_rx_buf[UART1_RX_BUFFER_SIZE];
extern uint8_t uart2_rx_buf[UART2_RX_BUFFER_SIZE];

void uart1_on_rx_event(const uint8_t *data, uint16_t len);
void uart2_on_rx_event(const uint8_t *data, uint16_t len);

static void uart_restart_rx_to_idle(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t len)
{
    if (HAL_UARTEx_ReceiveToIdle_DMA(huart, buf, len) == HAL_OK) {
        if (huart->hdmarx != NULL) {
            __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
        }
    }
}

void dma1_uart2_rx_config(uint8_t *dst, uint16_t len)
{
    (void)dst;
    uart_restart_rx_to_idle(&huart2, uart2_rx_buf, len);
}

void dma1_uart2_tx_start(uint8_t *src, uint16_t len)
{
    if ((src == NULL) || (len == 0U)) return;

    if (HAL_UART_Transmit_DMA(&huart2, src, len) != HAL_OK) {
        uart2_tx_busy = 0U;
    }
}

void dma2_uart1_rx_config(uint8_t *dst, uint16_t len)
{
    (void)dst;
    uart_restart_rx_to_idle(&huart1, uart1_rx_buf, len);
}

void dma2_uart1_tx_start(uint8_t *src, uint16_t len)
{
    if ((src == NULL) || (len == 0U)) return;

    if (HAL_UART_Transmit_DMA(&huart1, src, len) != HAL_OK) {
        uart1_tx_busy = 0U;
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if ((huart == NULL) || (Size == 0U)) {
        return;
    }

    if (huart->Instance == USART1) {
        uart1_on_rx_event(uart1_rx_buf, Size);
        uart_restart_rx_to_idle(&huart1, uart1_rx_buf, UART1_RX_BUFFER_SIZE);
    } else if (huart->Instance == USART2) {
        uart2_on_rx_event(uart2_rx_buf, Size);
        uart_restart_rx_to_idle(&huart2, uart2_rx_buf, UART2_RX_BUFFER_SIZE);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == NULL) return;

    if (huart->Instance == USART1) {
        uart1_tx_busy = 0U;
    } else if (huart->Instance == USART2) {
        uart2_tx_busy = 0U;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == NULL) return;

    if (huart->Instance == USART1) {
        uart1_tx_busy = 0U;
        uart_restart_rx_to_idle(&huart1, uart1_rx_buf, UART1_RX_BUFFER_SIZE);
    } else if (huart->Instance == USART2) {
        uart2_tx_busy = 0U;
        uart_restart_rx_to_idle(&huart2, uart2_rx_buf, UART2_RX_BUFFER_SIZE);
    }
}