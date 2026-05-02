
#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#define UART_TX_GPIO           20
#define UART_RX_GPIO           21

#define UART_PORT              UART_NUM_1
#define UART_BAUD              115200
#define UART_DRIVER_RX_BUF_SZ  2048

#define FRAME_LEN_APP          32
#define FRAME_PAYLOAD_MAX      (FRAME_LEN_APP - 5)

extern TaskHandle_t uart_rx_task_handle;

/*
    * @brief  Initialize UART1 peripheral (115200 8N1) and install the driver.
*/
void init_uart(void);

/*
    * @brief  Build a 32-byte frame and send it to STM32 over UART.
    * @param  status: Status byte (typically 0x00 for a request).
    * @param  cmd: Command byte.
    * @param  param: Parameter / sub-command byte.
    * @param  payload: Pointer to additional data, or NULL.
    * @param  payload_len: Payload length in bytes (max FRAME_PAYLOAD_MAX).
    * @retval ESP_OK on success, ESP_ERR_INVALID_SIZE or ESP_FAIL on error.
*/
esp_err_t send_frame_to_stm32(uint8_t status, uint8_t cmd, uint8_t param,
                                     const uint8_t *payload, uint32_t payload_len);

/*
    * @brief  FreeRTOS task that reads 32-byte frames from UART and calls decode_response.
    * @param  pvParameters: Unused.
*/
void uart_rx_task(void *pvParameters);

/*
    * @brief  Decode a response frame received from STM32 and publish the result via MQTT.
    * @param  req: Pointer to a 32-byte response frame.
*/
void decode_response(const uint8_t *req);


#endif // UART_H



