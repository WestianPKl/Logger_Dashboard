#include "uart.h"
#include "app_flags.h"
#include "support.h"

#include "driver/uart.h"
#include "esp_log.h"

#include <string.h>
#include <inttypes.h>

TaskHandle_t uart_rx_task_handle = NULL;

void init_uart(void)
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_TX_GPIO, UART_RX_GPIO,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, UART_DRIVER_RX_BUF_SZ, 0, 0, NULL, 0));
}


esp_err_t send_frame_to_stm32(uint8_t status, uint8_t cmd, uint8_t param,
                                     const uint8_t *payload, uint32_t payload_len)
{
    uint8_t frame[FRAME_LEN_APP];
    memset(frame, 0, sizeof(frame));

    if (payload_len > FRAME_PAYLOAD_MAX) {
        ESP_LOGE(TAG, "Payload too large: %" PRIu32, payload_len);
        return ESP_ERR_INVALID_SIZE;
    }

    frame[0] = DEV_ADDR;
    frame[1] = status;
    frame[2] = cmd;
    frame[3] = param;

    if (payload && payload_len > 0) {
        memcpy(&frame[4], payload, payload_len);
    }

    frame[FRAME_LEN_APP - 1] = crc8_atm(frame, FRAME_LEN_APP - 1);

    int written = uart_write_bytes(UART_PORT, frame, FRAME_LEN_APP);
    if (written != FRAME_LEN_APP) {
        ESP_LOGE(TAG, "UART write failed: %d/%d", written, FRAME_LEN_APP);
        return ESP_FAIL;
    }

    return ESP_OK;
}

void uart_rx_task(void *pvParameters)
{
    uint8_t frame[FRAME_LEN_APP];
    size_t collected = 0;

    while (1) {
        int len = uart_read_bytes(
            UART_PORT,
            &frame[collected],
            FRAME_LEN_APP - collected,
            pdMS_TO_TICKS(50)
        );

        if (len > 0) {
            collected += (size_t)len;

            if (collected == FRAME_LEN_APP) {
                decode_response(frame);
                collected = 0;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}