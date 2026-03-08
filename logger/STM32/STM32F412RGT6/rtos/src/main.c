#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "stm32f4xx.h"
#include "outputs.h"
#include "adc.h"
#include "dma.h"
#include "uart.h"
#include "version.h"
#include "support.h"

#define STATUS_OK       0x40
#define ERROR_RESPONSE  0x7F

#define FRAME_LEN_APP   24
#define FRAME_PAYLOAD   (FRAME_LEN_APP - 5)

#define ADC_BUFFER_SIZE 4
static uint16_t adc_data_buffer[ADC_BUFFER_SIZE];

#define UART2_RX_BUFFER_SIZE 128
static uint8_t uart2_rx_buf[UART2_RX_BUFFER_SIZE];
static uint8_t uart2_tx_frame[FRAME_LEN_APP];
static volatile uint16_t uart2_rx_old_pos = 0;
static uint8_t uart2_frame_acc[FRAME_LEN_APP];
static uint8_t uart2_frame_idx = 0;

#define UART1_RX_BUFFER_SIZE 128
static uint8_t uart1_rx_buf[UART1_RX_BUFFER_SIZE];
static uint8_t uart1_tx_frame[FRAME_LEN_APP];
static volatile uint16_t uart1_rx_old_pos = 0;
static uint8_t uart1_frame_acc[FRAME_LEN_APP];
static uint8_t uart1_frame_idx = 0;

static SemaphoreHandle_t uart2TxMutex = NULL;
static SemaphoreHandle_t uart1TxMutex = NULL;

static TaskHandle_t uart2TxWaiter = NULL;
static TaskHandle_t uart1TxWaiter = NULL;
static TaskHandle_t uart2RxTask   = NULL;
static TaskHandle_t uart1RxTask   = NULL;

volatile uint8_t uart2_tx_busy = 0;
volatile uint8_t uart1_tx_busy = 0;

static int  uart2_dma_send(const uint8_t *data, uint16_t len);
static int  uart1_dma_send(const uint8_t *data, uint16_t len);
static void uart2_process_rx(void);
static void uart1_process_rx(void);

static void handle_request(const uint8_t *req, uint8_t use_uart1);
static void handle_response(uint8_t status, uint8_t cmd, uint8_t param,
                            const uint8_t *payload, uint32_t payload_len, uint8_t use_uart1);

void uart2_rx_task(void *arg);
void uart1_rx_task(void *arg);
void redLedControl(void *arg);
void greenLedControl(void *arg);

static int uart2_dma_send(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return -1;
    if (len > sizeof(uart2_tx_frame)) len = sizeof(uart2_tx_frame);
    if (!uart2TxMutex) return -1;

    if (xSemaphoreTake(uart2TxMutex, pdMS_TO_TICKS(100)) != pdTRUE) return -1;

    (void)ulTaskNotifyTake(pdTRUE, 0);

    uart2TxWaiter = xTaskGetCurrentTaskHandle();
    uart2_tx_busy = 1;

    memcpy(uart2_tx_frame, data, len);
    dma1_uart2_tx_start(uart2_tx_frame, len);

    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100)) == 0) {
        uart2TxWaiter = NULL;
        uart2_tx_busy = 0;
        xSemaphoreGive(uart2TxMutex);
        return -1;
    }

    uart2TxWaiter = NULL;
    xSemaphoreGive(uart2TxMutex);
    return 1;
}

static int uart1_dma_send(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return -1;
    if (len > sizeof(uart1_tx_frame)) len = sizeof(uart1_tx_frame);
    if (!uart1TxMutex) return -1;

    if (xSemaphoreTake(uart1TxMutex, pdMS_TO_TICKS(100)) != pdTRUE) return -1;

    (void)ulTaskNotifyTake(pdTRUE, 0);

    uart1TxWaiter = xTaskGetCurrentTaskHandle();
    uart1_tx_busy = 1;

    memcpy(uart1_tx_frame, data, len);
    dma2_uart1_tx_start(uart1_tx_frame, len);

    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100)) == 0) {
        uart1TxWaiter = NULL;
        uart1_tx_busy = 0;
        xSemaphoreGive(uart1TxMutex);
        return -1;
    }

    uart1TxWaiter = NULL;
    xSemaphoreGive(uart1TxMutex);
    return 1;
}

static void handle_response(uint8_t status, uint8_t cmd, uint8_t param,
                            const uint8_t *payload, uint32_t payload_len, uint8_t use_uart1)
{
    uint8_t resp[FRAME_LEN_APP];
    memset(resp, 0, sizeof(resp));

    resp[0] = DEV_ADDR;
    resp[1] = status;
    resp[2] = cmd;
    resp[3] = param;

    if (payload && payload_len) {
        if (payload_len > FRAME_PAYLOAD) payload_len = FRAME_PAYLOAD;
        memcpy(&resp[4], payload, payload_len);
    }

    resp[FRAME_LEN_APP - 1] = crc8_atm(resp, FRAME_LEN_APP - 1);

    if (use_uart1) (void)uart1_dma_send(resp, FRAME_LEN_APP);
    else          (void)uart2_dma_send(resp, FRAME_LEN_APP);
}

static void handle_request(const uint8_t *req, uint8_t use_uart1)
{
    uint8_t addr  = req[0];
    uint8_t cmd   = req[2];
    uint8_t param = req[3];
    uint16_t key  = ((uint16_t)cmd << 8) | param;

    if (addr != DEV_ADDR) return;

    switch (key) {
        case 0x0000: {
            uint8_t data[3] = {0xAA, 0xAA, 0xAA};
            handle_response(STATUS_OK, cmd, param, data, sizeof(data), use_uart1);
        } break;

        case 0x0200: {
            uint16_t v0 = adc_data_buffer[0];
            uint16_t v1 = adc_data_buffer[1];
            uint8_t data[4] = { (uint8_t)(v0 >> 8), (uint8_t)v0, (uint8_t)(v1 >> 8), (uint8_t)v1 };
            handle_response(STATUS_OK, cmd, param, data, 4, use_uart1);
        } break;

        default:
            handle_response(ERROR_RESPONSE, cmd, param, NULL, 0, use_uart1);
            break;
    }
}

static void uart2_process_rx(void)
{
    uint16_t pos = (uint16_t)(UART2_RX_BUFFER_SIZE - DMA1_Stream5->NDTR);

    while (uart2_rx_old_pos != pos) {
        uint8_t b = uart2_rx_buf[uart2_rx_old_pos++];
        if (uart2_rx_old_pos >= UART2_RX_BUFFER_SIZE) uart2_rx_old_pos = 0;

        if (uart2_frame_idx == 0 && b != DEV_ADDR) continue;

        uart2_frame_acc[uart2_frame_idx++] = b;

        if (uart2_frame_idx == FRAME_LEN_APP) {
            if (crc8_atm(uart2_frame_acc, FRAME_LEN_APP - 1) == uart2_frame_acc[FRAME_LEN_APP - 1]) {
                handle_request(uart2_frame_acc, 0);
            }
            uart2_frame_idx = 0;
        }
    }
}

static void uart1_process_rx(void)
{
    uint16_t pos = (uint16_t)(UART1_RX_BUFFER_SIZE - DMA2_Stream5->NDTR);

    while (uart1_rx_old_pos != pos) {
        uint8_t b = uart1_rx_buf[uart1_rx_old_pos++];
        if (uart1_rx_old_pos >= UART1_RX_BUFFER_SIZE) uart1_rx_old_pos = 0;

        if (uart1_frame_idx == 0 && b != DEV_ADDR) continue;

        uart1_frame_acc[uart1_frame_idx++] = b;

        if (uart1_frame_idx == FRAME_LEN_APP) {
            if (crc8_atm(uart1_frame_acc, FRAME_LEN_APP - 1) == uart1_frame_acc[FRAME_LEN_APP - 1]) {
                handle_request(uart1_frame_acc, 1);
            }
            uart1_frame_idx = 0;
        }
    }
}

int main(void)
{
    __disable_irq();
    SCB->VTOR = 0x08008000U;
    __DSB(); __ISB();

    portc_init();
    portb_init();

    dma1_init();
    dma2_init();

    adc1_init(1, adc_data_buffer, ADC_BUFFER_SIZE);
    adc1_start_conversion();

    uart1_rxtx_init();
    uart2_rxtx_init();

    dma1_uart2_rx_config(uart2_rx_buf, UART2_RX_BUFFER_SIZE);
    dma2_uart1_rx_config(uart1_rx_buf, UART1_RX_BUFFER_SIZE);

    __enable_irq();

    uart2TxMutex = xSemaphoreCreateMutex();
    uart1TxMutex = xSemaphoreCreateMutex();

    xTaskCreate(redLedControl,   "Red",   256, NULL, 1, NULL);
    xTaskCreate(greenLedControl, "Green", 256, NULL, 1, NULL);
    xTaskCreate(uart2_rx_task,   "U2RX",  512, NULL, 2, NULL);
    xTaskCreate(uart1_rx_task,   "U1RX",  512, NULL, 2, NULL);

    vTaskStartScheduler();
    while (1) {}
}

void uart2_rx_task(void *arg)
{
    (void)arg;
    uart2RxTask = xTaskGetCurrentTaskHandle();

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        uart2_process_rx();
    }
}

void uart1_rx_task(void *arg)
{
    (void)arg;
    uart1RxTask = xTaskGetCurrentTaskHandle();

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        uart1_process_rx();
    }
}

void DMA1_Stream6_IRQHandler(void)
{
    BaseType_t hpw = pdFALSE;

    uint32_t hisr = DMA1->HISR;
    if (hisr & (DMA_HISR_TCIF6 | DMA_HISR_TEIF6 | DMA_HISR_DMEIF6 | DMA_HISR_FEIF6)) {

        DMA1->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6;

        uart2_tx_busy = 0;
        if (uart2TxWaiter) vTaskNotifyGiveFromISR(uart2TxWaiter, &hpw);

        portYIELD_FROM_ISR(hpw);
    }
}

void DMA2_Stream7_IRQHandler(void)
{
    BaseType_t hpw = pdFALSE;

    uint32_t hisr = DMA2->HISR;
    if (hisr & (DMA_HISR_TCIF7 | DMA_HISR_TEIF7 | DMA_HISR_DMEIF7 | DMA_HISR_FEIF7)) {

        DMA2->HIFCR = DMA_HIFCR_CTCIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7;

        uart1_tx_busy = 0;
        if (uart1TxWaiter) vTaskNotifyGiveFromISR(uart1TxWaiter, &hpw);

        portYIELD_FROM_ISR(hpw);
    }
}

void USART2_IRQHandler(void)
{
    BaseType_t hpw = pdFALSE;

    if (USART2->SR & USART_SR_IDLE) {
        (void)USART2->SR;
        (void)USART2->DR;

        if (uart2RxTask) vTaskNotifyGiveFromISR(uart2RxTask, &hpw);
        portYIELD_FROM_ISR(hpw);
    }
}

void USART1_IRQHandler(void)
{
    BaseType_t hpw = pdFALSE;

    if (USART1->SR & USART_SR_IDLE) {
        (void)USART1->SR;
        (void)USART1->DR;

        if (uart1RxTask) vTaskNotifyGiveFromISR(uart1RxTask, &hpw);
        portYIELD_FROM_ISR(hpw);
    }
}

void redLedControl(void *arg)
{
    (void)arg;
    for (;;) {
        pin_set_high('B', 15U);
        vTaskDelay(pdMS_TO_TICKS(500));
        pin_set_low('B', 15U);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void greenLedControl(void *arg)
{
    (void)arg;
    for (;;) {
        pin_set_high('B', 14U);
        vTaskDelay(pdMS_TO_TICKS(200));
        pin_set_low('B', 14U);
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}

void DMA2_Stream0_IRQHandler(void)
{
    if (DMA2->LISR & DMA_LISR_TCIF0) {
        DMA2->LIFCR = DMA_LIFCR_CTCIF0;
    }
}