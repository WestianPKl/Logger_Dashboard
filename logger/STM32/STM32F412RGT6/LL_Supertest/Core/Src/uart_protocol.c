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
volatile uint16_t uart2_rx_old_pos = 0;

uint8_t uart2_frame_acc[UART2_RX_FRAME_LEN];
volatile uint16_t uart2_frame_idx = 0;

uint8_t uart1_rx_buf[UART1_RX_BUFFER_SIZE];
uint8_t uart1_tx_frame[FRAME_LEN_APP];
volatile uint16_t uart1_rx_old_pos = 0;

uint8_t uart1_frame_acc[UART1_RX_FRAME_LEN];
volatile uint16_t uart1_frame_idx = 0;

uint16_t adc_data_buffer[ADC_BUFFER_SIZE];

static int uart2_dma_send(const uint8_t *data, uint16_t len);
static int uart1_dma_send(const uint8_t *data, uint16_t len);
static void handle_request(const uint8_t *req, uint8_t use_uart1);


static int uart2_dma_send(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return -1;
    if (len > sizeof(uart2_tx_frame)) len = sizeof(uart2_tx_frame);

    if (uart2_tx_busy) {
        return -1;
    }

    uart2_tx_busy = 1;
    memcpy(uart2_tx_frame, data, len);
    dma1_uart2_tx_start(uart2_tx_frame, len);
    return 1;
}

static int uart1_dma_send(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return -1;
    if (len > sizeof(uart1_tx_frame)) len = sizeof(uart1_tx_frame);

    if (uart1_tx_busy) {
        return -1;
    }

    uart1_tx_busy = 1;
    memcpy(uart1_tx_frame, data, len);
    dma2_uart1_tx_start(uart1_tx_frame, len);
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

    if (payload && payload_len) {
        memcpy(&resp[4], payload, payload_len);
    }

    resp[FRAME_LEN_APP - 1] = crc8_atm(resp, FRAME_LEN_APP - 1);

    if (use_uart1) uart1_dma_send(resp, FRAME_LEN_APP);
    else           uart2_dma_send(resp, FRAME_LEN_APP);
}

static void handle_request(const uint8_t *req, uint8_t use_uart1)
{
    uint8_t addr  = req[0];
    uint8_t cmd   = req[2];
    uint8_t param_addr = req[3];
    uint16_t cmd_combined = ((uint16_t)cmd << 8) | param_addr;

    if (addr != DEV_ADDR) return;

    switch (cmd_combined) {
        case 0x0000: /* No operation */
        {
            uint8_t data[3] = {0xAA, 0xAA, 0xAA};
            handle_response(STATUS_OK, cmd, param_addr, data, sizeof(data), use_uart1);
            break;
        }

        case 0x0100: /*READ: Get device serial number */
        {
            const device_info_t *info = device_info_get();
            uint32_t serial = 0;
            if (info->magic == INFO_MAGIC) serial = info->serial;

            uint8_t serial_bytes[4] = {
                (uint8_t)((serial >> 24) & 0xFF),
                (uint8_t)((serial >> 16) & 0xFF),
                (uint8_t)((serial >> 8) & 0xFF),
                (uint8_t)(serial & 0xFF)
            };
            handle_response(STATUS_OK, cmd, param_addr, serial_bytes, 4, use_uart1);
            break;
        }

        case 0x0101: /*READ: Get firmware and hardware version */
        {
            const device_info_t *info = device_info_get();
            uint8_t hwmaj = 0, hwmin = 0;
            if (info->magic == INFO_MAGIC) { hwmaj = info->hw_major; hwmin = info->hw_minor; }

            uint8_t payload[5] = {
                FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH, hwmaj, hwmin
            };
            handle_response(STATUS_OK, cmd, param_addr, payload, 5, use_uart1);
            break;
        }

        case 0x0102: /*READ: Get firmware build date */
        {
            uint8_t payload[10] = {0};
            memcpy(payload, FW_BUILD_DATE, 10);
            handle_response(STATUS_OK, cmd, param_addr, payload, 10, use_uart1);
            break;
        }

        case 0x0103: /*READ: Get production date */
        {
            const device_info_t *info = device_info_get();
            uint8_t payload[10] = {0};
            if (info->magic == INFO_MAGIC) memcpy(payload, info->prod_date, 10);
            handle_response(STATUS_OK, cmd, param_addr, payload, 10, use_uart1);
            break;
        }

        case 0x0200: /*READ: Get ADC measurements (up to 4 channels, 16-bit each) */
        {
            if (!adc_present) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (ADC_BUFFER_SIZE < 2) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }
            uint16_t v0 = adc_data_buffer[0];
            uint16_t v1 = adc_data_buffer[1];
            uint8_t data[4] = {(uint8_t)(v0 >> 8), (uint8_t)v0, (uint8_t)(v1 >> 8), (uint8_t)v1};
            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }

        case 0x0300: /*READ: Get SHT40 temperature and humidity */
        {

            if (!sht40_present) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (sht40_error_flag) {
                uint8_t err = 2;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            int16_t temp_c_x100 = measurement_sht40.temperature;
            uint16_t rh_x100 = measurement_sht40.humidity;

            uint8_t data[4];
            data[0] = (uint8_t)((temp_c_x100 >> 8) & 0xFF);
            data[1] = (uint8_t)(temp_c_x100 & 0xFF);
            data[2] = (uint8_t)((rh_x100 >> 8) & 0xFF);
            data[3] = (uint8_t)(rh_x100 & 0xFF);

            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }

        case 0x0301: /*READ: Get BME280 temperature, humidity and pressure */
        {
            if (!bme280_present) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (bme280_error_flag) {
                uint8_t err = 2;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            int32_t temp_x100   = measurement_bme280.temperature;
            uint32_t hum_x100   = measurement_bme280.humidity;
            uint32_t press_pa   = measurement_bme280.pressure;

            if (temp_x100 < -4000 || temp_x100 > 8500 || hum_x100 > 10000U || press_pa == 0) {
                uint8_t err = 3;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            uint8_t data[12];
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

            handle_response(STATUS_OK, cmd, param_addr, data, 12, use_uart1);
            break;
        }

        case 0x0401: /*OPERATIONAL: Set LED1 state (1 byte: 0 or 1) */
        {
            uint8_t val = req[4] ? 1U : 0U;

            if (val) LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);
            else     LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);
            
            led1_state = val;
            uint8_t data[1] = { led1_state };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }

        case 0x0402: /*OPERATIONAL: Set LED2 state (1 byte: 0 or 1) */
        {
            uint8_t val = req[4] ? 1U : 0U;

            if (val) LL_GPIO_SetOutputPin(LED2_GPIO_Port, LED2_Pin);
            else     LL_GPIO_ResetOutputPin(LED2_GPIO_Port, LED2_Pin);

            led2_state = val;
            uint8_t data[1] = { led2_state };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }

        case 0x0505: /*OPERATIONAL: Set RGB color for timer3 PWM (3 bytes: R, G, B) */
        {
            rgb_r = req[4];
            rgb_g = req[5];
            rgb_b = req[6];
            rgb_brightness = req[7];

            if (rgb_brightness > 100U) {
                rgb_brightness = 100U;
            }
            
            timer3_pwm_set_color(rgb_r, rgb_g, rgb_b, rgb_brightness);
            uint8_t data[4] = { rgb_r, rgb_g, rgb_b, rgb_brightness };
            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }

        case 0x0506: /*OPERATIONAL: Set buzzer frequency and volume for timer8 PWM (3 bytes: freq_H, freq_L, vol) */
        {
            if (req[4] == 0 && req[5] == 0) {
                timer8_pwm_set_buzzer(0, 0);
                handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            uint16_t freq = ((uint16_t)req[4] << 8) | req[5];
            uint8_t  vol  = req[6];

            if (vol > 100U) vol = 100U;

            timer8_pwm_set_buzzer((uint32_t)freq, (uint32_t)vol);
            uint8_t data[3] = { req[4], req[5], vol };
            handle_response(STATUS_OK, cmd, param_addr, data, 3, use_uart1);
            break;
        }

        case 0x0600: /*READ: Get date and time from RTC (7 bytes: year, month, day, weekday, hour, minute, second) */
        {
            uint8_t data[7];

            data[0] = datetime.year;
            data[1] = datetime.month;
            data[2] = datetime.day;
            data[3] = datetime.weekday;
            data[4] = datetime.hours;
            data[5] = datetime.minutes;
            data[6] = datetime.seconds;

            handle_response(STATUS_OK, cmd, param_addr, data, 7, use_uart1);
            break;
        }

        default:
            handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
            break;
    }
}

void uart2_process_rx(void)
{
    uint16_t ndtr = DMA1_Stream5->NDTR;
    if (ndtr > UART2_RX_BUFFER_SIZE) {
        return;
    }

    uint16_t pos = (uint16_t)(UART2_RX_BUFFER_SIZE - ndtr);
    if (pos >= UART2_RX_BUFFER_SIZE) {
        pos = 0;
    }

    while (uart2_rx_old_pos != pos) {
        uint8_t b = uart2_rx_buf[uart2_rx_old_pos++];
        if (uart2_rx_old_pos >= UART2_RX_BUFFER_SIZE) uart2_rx_old_pos = 0;

        if (uart2_frame_idx == 0 && b != DEV_ADDR) continue;

        uart2_frame_acc[uart2_frame_idx++] = b;
        if (uart2_frame_idx == UART2_RX_FRAME_LEN) {
            if (crc8_atm(uart2_frame_acc, FRAME_LEN_APP - 1) == uart2_frame_acc[FRAME_LEN_APP - 1]) {
                handle_request(uart2_frame_acc, 0);
            }
            uart2_frame_idx = 0;
            if (b == DEV_ADDR) {
                uart2_frame_acc[0] = b;
                uart2_frame_idx = 1;
            }
        }
    }
}

void uart1_process_rx(void)
{
    uint16_t ndtr = DMA2_Stream2->NDTR;
    if (ndtr > UART1_RX_BUFFER_SIZE) {
        return;
    }

    uint16_t pos = (uint16_t)(UART1_RX_BUFFER_SIZE - ndtr);
    if (pos >= UART1_RX_BUFFER_SIZE) {
        pos = 0;
    }

    while (uart1_rx_old_pos != pos) {
        uint8_t b = uart1_rx_buf[uart1_rx_old_pos++];
        if (uart1_rx_old_pos >= UART1_RX_BUFFER_SIZE) uart1_rx_old_pos = 0;

        if (uart1_frame_idx == 0 && b != DEV_ADDR) continue;

        uart1_frame_acc[uart1_frame_idx++] = b;
        if (uart1_frame_idx == UART1_RX_FRAME_LEN) {
            if (crc8_atm(uart1_frame_acc, FRAME_LEN_APP - 1) == uart1_frame_acc[FRAME_LEN_APP - 1]) {
                handle_request(uart1_frame_acc, 1);
            }
            uart1_frame_idx = 0;
            if (b == DEV_ADDR) {
                uart1_frame_acc[0] = b;
                uart1_frame_idx = 1;
            }
        }
    }
}