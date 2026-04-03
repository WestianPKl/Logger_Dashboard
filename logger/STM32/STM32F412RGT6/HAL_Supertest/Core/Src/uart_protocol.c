#include <stdint.h>
#include <string.h>
#include "uart_protocol.h"
#include "app_flags.h"
#include "version.h"
#include "support.h"
#include "uart.h"
#include "main.h"
#include "timer.h"

#define STATUS_OK       0x40
#define ERROR_RESPONSE  0x7F

uint8_t uart2_rx_buf[UART2_RX_BUFFER_SIZE];
uint8_t uart2_tx_frame[FRAME_LEN_APP];
uint8_t uart2_frame_acc[UART2_RX_FRAME_LEN];
volatile uint16_t uart2_frame_idx = 0;

uint8_t uart1_rx_buf[UART1_RX_BUFFER_SIZE];
uint8_t uart1_tx_frame[FRAME_LEN_APP];
uint8_t uart1_frame_acc[UART1_RX_FRAME_LEN];
volatile uint16_t uart1_frame_idx = 0;

uint16_t adc_data_buffer[ADC_BUFFER_SIZE];

typedef struct
{
    uint8_t *tx_frame;
    volatile uint8_t *tx_busy;
    uint8_t *frame_acc;
    volatile uint16_t *frame_idx;
    void (*tx_start)(uint8_t *src, uint16_t len);
} uart_port_ctx_t;

static void handle_request(const uint8_t *req, uint8_t use_uart1);

static uart_port_ctx_t *uart_get_ctx(uint8_t use_uart1)
{
    static uart_port_ctx_t uart1_ctx;
    static uart_port_ctx_t uart2_ctx;
    static uint8_t init_done = 0U;

    if (!init_done) {
        uart1_ctx.tx_frame  = uart1_tx_frame;
        uart1_ctx.tx_busy   = &uart1_tx_busy;
        uart1_ctx.frame_acc = uart1_frame_acc;
        uart1_ctx.frame_idx = &uart1_frame_idx;
        uart1_ctx.tx_start  = dma2_uart1_tx_start;

        uart2_ctx.tx_frame  = uart2_tx_frame;
        uart2_ctx.tx_busy   = &uart2_tx_busy;
        uart2_ctx.frame_acc = uart2_frame_acc;
        uart2_ctx.frame_idx = &uart2_frame_idx;
        uart2_ctx.tx_start  = dma1_uart2_tx_start;

        init_done = 1U;
    }

    return use_uart1 ? &uart1_ctx : &uart2_ctx;
}

static int uart_dma_send(uint8_t use_uart1, const uint8_t *data, uint16_t len)
{
    uart_port_ctx_t *ctx = uart_get_ctx(use_uart1);

    if ((data == NULL) || (len == 0U)) return -1;
    if (len > FRAME_LEN_APP) len = FRAME_LEN_APP;

    if (*(ctx->tx_busy)) {
        return -1;
    }

    *(ctx->tx_busy) = 1U;
    memcpy(ctx->tx_frame, data, len);
    ctx->tx_start(ctx->tx_frame, len);
    return 1;
}

static void handle_response(uint8_t status, uint8_t cmd, uint8_t param,
                            const uint8_t *payload, uint32_t payload_len, uint8_t use_uart1)
{
    uint8_t resp[FRAME_LEN_APP];

    memset(resp, 0, sizeof(resp));

    if (payload_len > FRAME_PAYLOAD) {
        return;
    }

    resp[0] = DEV_ADDR;
    resp[1] = status;
    resp[2] = cmd;
    resp[3] = param;

    if ((payload != NULL) && (payload_len != 0U)) {
        memcpy(&resp[4], payload, payload_len);
    }

    resp[FRAME_LEN_APP - 1U] = crc8_atm(resp, FRAME_LEN_APP - 1U);
    (void)uart_dma_send(use_uart1, resp, FRAME_LEN_APP);
}

static void handle_request(const uint8_t *req, uint8_t use_uart1)
{
    uint8_t addr;
    uint8_t cmd;
    uint8_t param_addr;
    uint16_t cmd_combined;

    if (req == NULL) return;

    addr = req[0];
    cmd = req[2];
    param_addr = req[3];
    cmd_combined = ((uint16_t)cmd << 8) | param_addr;

    if (addr != DEV_ADDR) return;

    switch (cmd_combined) {
        case 0x0000:
        {
            uint8_t data[3] = {0xAA, 0xAA, 0xAA};
            handle_response(STATUS_OK, cmd, param_addr, data, sizeof(data), use_uart1);
            break;
        }

        case 0x0100:
        {
            const device_info_t *info = device_info_get();
            uint32_t serial = 0U;
            uint8_t serial_bytes[4];

            if (info->magic == INFO_MAGIC) {
                serial = info->serial;
            }

            serial_bytes[0] = (uint8_t)((serial >> 24) & 0xFF);
            serial_bytes[1] = (uint8_t)((serial >> 16) & 0xFF);
            serial_bytes[2] = (uint8_t)((serial >> 8) & 0xFF);
            serial_bytes[3] = (uint8_t)(serial & 0xFF);

            handle_response(STATUS_OK, cmd, param_addr, serial_bytes, 4U, use_uart1);
            break;
        }

        case 0x0101:
        {
            const device_info_t *info = device_info_get();
            uint8_t hwmaj = 0U;
            uint8_t hwmin = 0U;
            uint8_t payload[5];

            if (info->magic == INFO_MAGIC) {
                hwmaj = info->hw_major;
                hwmin = info->hw_minor;
            }

            payload[0] = FW_VERSION_MAJOR;
            payload[1] = FW_VERSION_MINOR;
            payload[2] = FW_VERSION_PATCH;
            payload[3] = hwmaj;
            payload[4] = hwmin;

            handle_response(STATUS_OK, cmd, param_addr, payload, sizeof(payload), use_uart1);
            break;
        }

        case 0x0102:
        {
            uint8_t payload[10] = {0};
            memcpy(payload, FW_BUILD_DATE, 10);
            handle_response(STATUS_OK, cmd, param_addr, payload, 10U, use_uart1);
            break;
        }

        case 0x0103:
        {
            const device_info_t *info = device_info_get();
            uint8_t payload[10] = {0};

            if (info->magic == INFO_MAGIC) {
                memcpy(payload, info->prod_date, 10);
            }

            handle_response(STATUS_OK, cmd, param_addr, payload, 10U, use_uart1);
            break;
        }

        case 0x0200:
        {
            uint16_t v0, v1;
            uint8_t data[4];

            if (!adc_present) {
                uint8_t err = 1U;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1U, use_uart1);
                break;
            }

            if (ADC_BUFFER_SIZE < 2) {
                uint8_t err = 2U;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1U, use_uart1);
                break;
            }

            v0 = adc_data_buffer[0];
            v1 = adc_data_buffer[1];

            data[0] = (uint8_t)(v0 >> 8);
            data[1] = (uint8_t)(v0);
            data[2] = (uint8_t)(v1 >> 8);
            data[3] = (uint8_t)(v1);

            handle_response(STATUS_OK, cmd, param_addr, data, sizeof(data), use_uart1);
            break;
        }

        case 0x0300:
        {
            int16_t temp_c_x100;
            uint16_t rh_x100;
            uint8_t data[4];

            if (!sht40_present) {
                uint8_t err = 1U;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1U, use_uart1);
                break;
            }

            if (sht40_error_flag) {
                uint8_t err = 2U;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1U, use_uart1);
                break;
            }

            temp_c_x100 = measurement_sht40.temperature;
            rh_x100 = measurement_sht40.humidity;

            data[0] = (uint8_t)((temp_c_x100 >> 8) & 0xFF);
            data[1] = (uint8_t)(temp_c_x100 & 0xFF);
            data[2] = (uint8_t)((rh_x100 >> 8) & 0xFF);
            data[3] = (uint8_t)(rh_x100 & 0xFF);

            handle_response(STATUS_OK, cmd, param_addr, data, sizeof(data), use_uart1);
            break;
        }

        case 0x0301:
        {
            int32_t temp_x100;
            uint32_t hum_x100;
            uint32_t press_pa;
            uint8_t data[12];

            if (!bme280_present) {
                uint8_t err = 1U;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1U, use_uart1);
                break;
            }

            if (bme280_error_flag) {
                uint8_t err = 2U;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1U, use_uart1);
                break;
            }

            temp_x100 = measurement_bme280.temperature;
            hum_x100  = measurement_bme280.humidity;
            press_pa  = measurement_bme280.pressure;

            if ((temp_x100 < -4000) || (temp_x100 > 8500) ||
                (hum_x100 > 10000U) || (press_pa == 0U)) {
                uint8_t err = 3U;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1U, use_uart1);
                break;
            }

            data[0]  = (uint8_t)((temp_x100 >> 24) & 0xFF);
            data[1]  = (uint8_t)((temp_x100 >> 16) & 0xFF);
            data[2]  = (uint8_t)((temp_x100 >> 8) & 0xFF);
            data[3]  = (uint8_t)(temp_x100 & 0xFF);

            data[4]  = (uint8_t)((hum_x100 >> 24) & 0xFF);
            data[5]  = (uint8_t)((hum_x100 >> 16) & 0xFF);
            data[6]  = (uint8_t)((hum_x100 >> 8) & 0xFF);
            data[7]  = (uint8_t)(hum_x100 & 0xFF);

            data[8]  = (uint8_t)((press_pa >> 24) & 0xFF);
            data[9]  = (uint8_t)((press_pa >> 16) & 0xFF);
            data[10] = (uint8_t)((press_pa >> 8) & 0xFF);
            data[11] = (uint8_t)(press_pa & 0xFF);

            handle_response(STATUS_OK, cmd, param_addr, data, sizeof(data), use_uart1);
            break;
        }

        case 0x0401:
        {
            uint8_t val = req[4] ? 1U : 0U;
            uint8_t data[1];

            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
            led1_state = val;

            data[0] = led1_state;
            handle_response(STATUS_OK, cmd, param_addr, data, 1U, use_uart1);
            break;
        }

        case 0x0402:
        {
            uint8_t val = req[4] ? 1U : 0U;
            uint8_t data[1];

            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
            led2_state = val;

            data[0] = led2_state;
            handle_response(STATUS_OK, cmd, param_addr, data, 1U, use_uart1);
            break;
        }

        case 0x0505:
        {
            uint8_t data[4];

            rgb_r = req[4];
            rgb_g = req[5];
            rgb_b = req[6];
            rgb_brightness = req[7];

            if (rgb_brightness > 100U) {
                rgb_brightness = 100U;
            }

            timer3_pwm_set_color(rgb_r, rgb_g, rgb_b, rgb_brightness);

            data[0] = rgb_r;
            data[1] = rgb_g;
            data[2] = rgb_b;
            data[3] = rgb_brightness;

            handle_response(STATUS_OK, cmd, param_addr, data, sizeof(data), use_uart1);
            break;
        }

        case 0x0506:
        {
            uint16_t freq;
            uint8_t vol;
            uint8_t data[3];

            if ((req[4] == 0U) && (req[5] == 0U)) {
                timer8_pwm_set_buzzer(0U, 0U);
                handle_response(STATUS_OK, cmd, param_addr, NULL, 0U, use_uart1);
                break;
            }

            freq = ((uint16_t)req[4] << 8) | req[5];
            vol  = req[6];

            if (vol > 100U) vol = 100U;

            timer8_pwm_set_buzzer((uint32_t)freq, (uint32_t)vol);

            data[0] = req[4];
            data[1] = req[5];
            data[2] = vol;

            handle_response(STATUS_OK, cmd, param_addr, data, sizeof(data), use_uart1);
            break;
        }

        case 0x0600:
        {
            uint8_t data[7];

            data[0] = datetime.year;
            data[1] = datetime.month;
            data[2] = datetime.day;
            data[3] = datetime.weekday;
            data[4] = datetime.hours;
            data[5] = datetime.minutes;
            data[6] = datetime.seconds;

            handle_response(STATUS_OK, cmd, param_addr, data, sizeof(data), use_uart1);
            break;
        }

        default:
            handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0U, use_uart1);
            break;
    }
}

static void uart_feed_bytes(const uint8_t *data, uint16_t len, uint8_t use_uart1)
{
    uart_port_ctx_t *ctx = uart_get_ctx(use_uart1);
    uint16_t i;

    if ((data == NULL) || (len == 0U)) return;

    for (i = 0U; i < len; i++) {
        uint8_t b = data[i];

        if ((*(ctx->frame_idx) == 0U) && (b != DEV_ADDR)) {
            continue;
        }

        ctx->frame_acc[*(ctx->frame_idx)] = b;
        (*(ctx->frame_idx))++;

        if (*(ctx->frame_idx) >= FRAME_LEN_APP) {
            if (crc8_atm(ctx->frame_acc, FRAME_LEN_APP - 1U) == ctx->frame_acc[FRAME_LEN_APP - 1U]) {
                handle_request(ctx->frame_acc, use_uart1);
            }

            *(ctx->frame_idx) = 0U;

            if (b == DEV_ADDR) {
                ctx->frame_acc[0] = b;
                *(ctx->frame_idx) = 1U;
            }
        }
    }
}

void uart1_on_rx_event(const uint8_t *data, uint16_t len)
{
    uart_feed_bytes(data, len, 1U);
}

void uart2_on_rx_event(const uint8_t *data, uint16_t len)
{
    uart_feed_bytes(data, len, 0U);
}
