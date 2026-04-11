#include <stdint.h>
#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
#include "uart_protocol.h"
#include "uart.h"
#include "lcd.h"
#include "sht40.h"
#include "bme280.h"
#include "rtc.h"
#include "timer.h"
#include "spi.h"
#include "i2c.h"
#include "support.h"
#include "app_flags.h"
#include "mcp7940n.h"
#include "fm24cl16b.h"
#include "mx25l25673gm2i.h"
#include "version.h"
#include "flash_log.h"

#define STATUS_OK       0x40
#define ERROR_RESPONSE  0x7F

extern SemaphoreHandle_t i2cMutex;
extern SemaphoreHandle_t spiMutex;

uint8_t uart2_rx_buf[UART2_RX_BUFFER_SIZE];
uint8_t uart2_tx_frame[FRAME_LEN_APP];

uint8_t uart1_rx_buf[UART1_RX_BUFFER_SIZE];
uint8_t uart1_tx_frame[FRAME_LEN_APP];

static void handle_response(uint8_t status, uint8_t cmd, uint8_t param,
                            const uint8_t *payload, uint32_t payload_len, uint8_t use_uart1)
{
    uint8_t resp[FRAME_LEN_APP];
    memset(resp, 0, sizeof(resp));

    if (payload_len > FRAME_PAYLOAD_SIZE) {
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

    if (use_uart1) {
        if (!uart1_tx_busy) {
            uart1_tx_busy = 1U;
            memcpy(uart1_tx_frame, resp, FRAME_LEN_APP);
            if (HAL_UART_Transmit_DMA(&huart1, uart1_tx_frame, FRAME_LEN_APP) != HAL_OK) {
                uart1_tx_busy = 0U;
            }
        }
    } else {
        if (!uart2_tx_busy) {
            uart2_tx_busy = 1U;
            memcpy(uart2_tx_frame, resp, FRAME_LEN_APP);
            if (HAL_UART_Transmit_DMA(&huart2, uart2_tx_frame, FRAME_LEN_APP) != HAL_OK) {
                uart2_tx_busy = 0U;
            }
        }
    }
}

void handle_request(const uint8_t *req, uint8_t use_uart1)
{
    uint8_t addr = req[0];
    uint8_t cmd = req[2];
    uint8_t param_addr = req[3];
    uint16_t cmd_combined = ((uint16_t)cmd << 8) | param_addr;

    if (addr != DEV_ADDR) {
        return;
    }

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
            uint32_t serial = 0;
            uint8_t serial_bytes[4];

            if (info->magic == INFO_MAGIC) {
                serial = info->serial;
            }

            serial_bytes[0] = (uint8_t)((serial >> 24) & 0xFF);
            serial_bytes[1] = (uint8_t)((serial >> 16) & 0xFF);
            serial_bytes[2] = (uint8_t)((serial >> 8) & 0xFF);
            serial_bytes[3] = (uint8_t)(serial & 0xFF);

            handle_response(STATUS_OK, cmd, param_addr, serial_bytes, 4, use_uart1);
            break;
        }

        case 0x0101:
        {
            const device_info_t *info = device_info_get();
            uint8_t hwmaj = 0;
            uint8_t hwmin = 0;
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

            handle_response(STATUS_OK, cmd, param_addr, payload, 5, use_uart1);
            break;
        }

        case 0x0102:
        {
            uint8_t payload[10] = {0};
            memcpy(payload, FW_BUILD_DATE, 10);
            handle_response(STATUS_OK, cmd, param_addr, payload, 10, use_uart1);
            break;
        }

        case 0x0103:
        {
            const device_info_t *info = device_info_get();
            uint8_t payload[10] = {0};

            if (info->magic == INFO_MAGIC) {
                memcpy(payload, info->prod_date, 10);
            }

            handle_response(STATUS_OK, cmd, param_addr, payload, 10, use_uart1);
            break;
        }

        case 0x0200:
        {
            uint16_t v0;
            uint16_t v1;
            uint8_t data[4];

            if ((xEventGroupGetBits(appEvents) & EVT_ADC_PRESENT) == 0U) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (ADC_BUFFER_SIZE < 2) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            v0 = adc_data_buffer[0];
            v1 = adc_data_buffer[1];

            data[0] = (uint8_t)(v0 >> 8);
            data[1] = (uint8_t)(v0);
            data[2] = (uint8_t)(v1 >> 8);
            data[3] = (uint8_t)(v1);

            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }

        case 0x0201:
        {
            uint8_t state;
            state = (HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_SET) ? 1U : 0U;
            app_io_state.btn1_pressed = state;
            handle_response(STATUS_OK, cmd, param_addr, &state, 1, use_uart1);
            break;
        }

        case 0x0202:
        {
            uint8_t state;
            state = (HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_SET) ? 1U : 0U;
            app_io_state.btn2_pressed = state;
            handle_response(STATUS_OK, cmd, param_addr, &state, 1, use_uart1);
            break;
        }

        case 0x0203:
        {
            uint8_t state;
            state = (HAL_GPIO_ReadPin(ESP32_Status_GPIO_Port, ESP32_Status_Pin) == GPIO_PIN_SET) ? 1U : 0U;
            app_io_state.esp32_state = state;
            handle_response(STATUS_OK, cmd, param_addr, &state, 1, use_uart1);
            break;
        }

        case 0x0300:
        {
            int16_t temp_c_x100;
            uint16_t rh_x100;
            uint8_t data[4];

            if ((xEventGroupGetBits(appEvents) & EVT_SHT40_PRESENT) == 0U) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (xEventGroupGetBits(appEvents) & EVT_SHT40_ERROR) {
                uint8_t err = 2;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
                uint8_t err = 4;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            temp_c_x100 = sht40_data.temperature;
            rh_x100 = sht40_data.humidity;
            xSemaphoreGive(sensorDataMutex);

            data[0] = (uint8_t)((temp_c_x100 >> 8) & 0xFF);
            data[1] = (uint8_t)(temp_c_x100 & 0xFF);
            data[2] = (uint8_t)((rh_x100 >> 8) & 0xFF);
            data[3] = (uint8_t)(rh_x100 & 0xFF);

            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }

        case 0x0301:
        {
            int32_t temp_x100;
            uint32_t hum_x100;
            uint32_t press_pa;
            uint8_t data[12];

            if ((xEventGroupGetBits(appEvents) & EVT_BME280_PRESENT) == 0U) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (xEventGroupGetBits(appEvents) & EVT_BME280_ERROR) {
                uint8_t err = 2;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
                uint8_t err = 4;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            temp_x100 = bme280_data.temperature;
            hum_x100 = bme280_data.humidity;
            press_pa = bme280_data.pressure;
            xSemaphoreGive(sensorDataMutex);

            if ((temp_x100 < -4000) || (temp_x100 > 8500) || (hum_x100 > 10000U) || (press_pa == 0U)) {
                uint8_t err = 3;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
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

            handle_response(STATUS_OK, cmd, param_addr, data, 12, use_uart1);
            break;
        }

        case 0x0302:
        {
            lcd_msg_t msg = { .cmd = LCD_CLEAR };

            if ((xEventGroupGetBits(appEvents) & EVT_LCD_PRESENT) == 0U) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (lcdCmdQueue == NULL) {
                uint8_t err = 2;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (xQueueSend(lcdCmdQueue, &msg, pdMS_TO_TICKS(100)) != pdTRUE) {
                uint8_t err = 3;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (LCDTaskHandle != NULL) {
                xTaskNotify(LCDTaskHandle, LCD_NOTIFY_COMMAND, eSetBits);
            }

            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }

        case 0x0303:
        {
            uint8_t backlight_on;
            lcd_msg_t msg;

            if ((xEventGroupGetBits(appEvents) & EVT_LCD_PRESENT) == 0U) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (lcdCmdQueue == NULL) {
                uint8_t err = 2;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            backlight_on = req[4] ? 1U : 0U;
            msg.cmd = LCD_BACKLIGHT;
            msg.flag = backlight_on;

            if (xQueueSend(lcdCmdQueue, &msg, pdMS_TO_TICKS(100)) != pdTRUE) {
                uint8_t err = 3;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (LCDTaskHandle != NULL) {
                xTaskNotify(LCDTaskHandle, LCD_NOTIFY_COMMAND, eSetBits);
            }

            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }

        case 0x0400:
        {
            uint8_t data[6] = {
                app_io_state.led1_state,
                app_io_state.led2_state,
                app_rgb.r,
                app_rgb.g,
                app_rgb.b,
                app_rgb.brightness
            };
            handle_response(STATUS_OK, cmd, param_addr, (const uint8_t *)data, 6, use_uart1);
            break;
        }

        case 0x0401:
        {
            uint8_t val = req[4] ? 1U : 0U;
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
            app_io_state.led1_state = val;
            handle_response(STATUS_OK, cmd, param_addr, (const uint8_t *)&app_io_state.led1_state, 1, use_uart1);
            break;
        }

        case 0x0402:
        {
            uint8_t val = req[4] ? 1U : 0U;
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
            app_io_state.led2_state = val;
            handle_response(STATUS_OK, cmd, param_addr, (const uint8_t *)&app_io_state.led2_state, 1, use_uart1);
            break;
        }

        case 0x0403:
        {
            uint8_t val = req[4] ? 1U : 0U;
            HAL_GPIO_WritePin(CON_5_GPIO_Port, CON_5_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
            app_io_state.pb12_state = val;
            handle_response(STATUS_OK, cmd, param_addr, (const uint8_t *)&app_io_state.pb12_state, 1, use_uart1);
            break;
        }

        case 0x0404:
        {
            uint8_t val = req[4] ? 1U : 0U;
            HAL_GPIO_WritePin(CON_1_GPIO_Port, CON_1_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
            app_io_state.pc0_state = val;
            handle_response(STATUS_OK, cmd, param_addr, (const uint8_t *)&app_io_state.pc0_state, 1, use_uart1);
            break;
        }

        case 0x0405:
        {
            uint8_t val = req[4] ? 1U : 0U;
            HAL_GPIO_WritePin(CON_2_GPIO_Port, CON_2_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
            app_io_state.pc1_state = val;
            handle_response(STATUS_OK, cmd, param_addr, (const uint8_t *)&app_io_state.pc1_state, 1, use_uart1);
            break;
        }

        case 0x0406:
        {
            uint8_t val = req[4] ? 1U : 0U;
            HAL_GPIO_WritePin(CON_3_GPIO_Port, CON_3_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
            app_io_state.pc2_state = val;
            handle_response(STATUS_OK, cmd, param_addr, (const uint8_t *)&app_io_state.pc2_state, 1, use_uart1);
            break;
        }

        case 0x0407:
        {
            uint8_t val = req[4] ? 1U : 0U;
            HAL_GPIO_WritePin(CON_4_GPIO_Port, CON_4_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
            app_io_state.pc3_state = val;
            handle_response(STATUS_OK, cmd, param_addr, (const uint8_t *)&app_io_state.pc3_state, 1, use_uart1);
            break;
        }

        case 0x0408:
        {
            uint8_t val = req[4] ? 1U : 0U;
            HAL_GPIO_WritePin(ESP32_Write_GPIO_Port, ESP32_Write_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
            handle_response(STATUS_OK, cmd, param_addr, &val, 1, use_uart1);
            break;
        }

        case 0x0501:
        {
            uint8_t duty = req[4];
            if (duty > 100U) duty = 100U;
            timer1_pwm_ch1_set_duty(duty);
            handle_response(STATUS_OK, cmd, param_addr, &duty, 1, use_uart1);
            break;
        }

        case 0x0502:
        {
            uint8_t duty = req[4];
            if (duty > 100U) duty = 100U;
            timer2_pwm_ch3_set_duty(duty);
            handle_response(STATUS_OK, cmd, param_addr, &duty, 1, use_uart1);
            break;
        }

        case 0x0503:
        {
            uint8_t duty = req[4];
            if (duty > 100U) duty = 100U;
            timer4_pwm_ch3_set_duty(duty);
            handle_response(STATUS_OK, cmd, param_addr, &duty, 1, use_uart1);
            break;
        }

        case 0x0504:
        {
            uint8_t duty = req[4];
            if (duty > 100U) duty = 100U;
            timer4_pwm_ch4_set_duty(duty);
            handle_response(STATUS_OK, cmd, param_addr, &duty, 1, use_uart1);
            break;
        }

        case 0x0505:
        {
            uint8_t data[4];

            app_rgb.r = req[4];
            app_rgb.g = req[5];
            app_rgb.b = req[6];
            app_rgb.brightness = req[7];

            if (app_rgb.brightness > 100U) {
                app_rgb.brightness = 100U;
            }

            timer3_pwm_set_color(app_rgb.r, app_rgb.g, app_rgb.b, app_rgb.brightness);

            data[0] = app_rgb.r;
            data[1] = app_rgb.g;
            data[2] = app_rgb.b;
            data[3] = app_rgb.brightness;

            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }

        case 0x0506:
        {
            uint16_t freq;
            uint8_t vol;
            uint8_t data[3];

            if ((req[4] == 0U) && (req[5] == 0U)) {
                timer8_pwm_set_buzzer(0, 0);
                handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            freq = ((uint16_t)req[4] << 8) | req[5];
            vol = req[6];

            if (vol > 100U) {
                vol = 100U;
            }

            app_buzzer.freq = (uint32_t)freq;
            app_buzzer.duty_cycle = (uint32_t)vol;

            timer8_pwm_set_buzzer(app_buzzer.freq, app_buzzer.duty_cycle);

            data[0] = req[4];
            data[1] = req[5];
            data[2] = vol;

            handle_response(STATUS_OK, cmd, param_addr, data, 3, use_uart1);
            break;
        }

        case 0x0600:
        {
            uint8_t data[7];

            if (xSemaphoreTake(rtcDataMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
                uint8_t err = 4;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            data[0] = rtc_date_time.year;
            data[1] = rtc_date_time.month;
            data[2] = rtc_date_time.day;
            data[3] = rtc_date_time.weekday;
            data[4] = rtc_date_time.hours;
            data[5] = rtc_date_time.minutes;
            data[6] = rtc_date_time.seconds;

            xSemaphoreGive(rtcDataMutex);

            handle_response(STATUS_OK, cmd, param_addr, data, 7, use_uart1);
            break;
        }

        case 0x0601:
        {
            uint8_t year = req[4];
            uint8_t month = req[5];
            uint8_t day = req[6];
            uint8_t weekday = req[7];
            uint8_t hours = req[8];
            uint8_t minutes = req[9];
            uint8_t seconds = req[10];
            rtc_msg_t msg = {0};
            uint8_t data[7];

            if ((year > 99U) ||
                (month < 1U) || (month > 12U) ||
                (day < 1U) || (day > 31U) ||
                (weekday < 1U) || (weekday > 7U) ||
                (hours > 23U) || (minutes > 59U) || (seconds > 59U))
            {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            msg.cmd = RTC_CMD_SET_DATETIME;

            if (xEventGroupGetBits(appEvents) & EVT_EXT_RTC_PRESENT) {
                msg.ext_datetime.year  = year;
                msg.ext_datetime.month = month;
                msg.ext_datetime.mday  = day;
                msg.ext_datetime.wday  = weekday;
                msg.ext_datetime.hour  = hours;
                msg.ext_datetime.min   = minutes;
                msg.ext_datetime.sec   = seconds;
            } else {
                msg.datetime.year = year;
                msg.datetime.month = month;
                msg.datetime.day = day;
                msg.datetime.weekday = weekday;
                msg.datetime.hours = hours;
                msg.datetime.minutes = minutes;
                msg.datetime.seconds = seconds;
            }

            if ((rtcCmdQueue == NULL) ||
                (xQueueSend(rtcCmdQueue, &msg, pdMS_TO_TICKS(100)) != pdPASS))
            {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            data[0] = year;
            data[1] = month;
            data[2] = day;
            data[3] = weekday;
            data[4] = hours;
            data[5] = minutes;
            data[6] = seconds;

            handle_response(STATUS_OK, cmd, param_addr, data, 7, use_uart1);
            break;
        }

        case 0x0800:
        {
            uint16_t addr16 = ((uint16_t)req[4] << 8) | req[5];
            uint8_t data_len = req[6];

            if ((data_len == 0U) || (data_len > 16U)) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (((uint32_t)addr16 + data_len) > FM24CL16B_SIZE_BYTES) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (fm24cl16b_write(addr16, &req[7], data_len) != 1) {
                xSemaphoreGive(i2cMutex);
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            xSemaphoreGive(i2cMutex);
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }

        case 0x0801:
        {
            uint16_t addr16 = ((uint16_t)req[4] << 8) | req[5];
            uint8_t data_len = req[6];
            uint8_t data[16] = {0};

            if ((data_len == 0U) || (data_len > 16U)) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (((uint32_t)addr16 + data_len) > FM24CL16B_SIZE_BYTES) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (fm24cl16b_read(addr16, data, data_len) != 1) {
                xSemaphoreGive(i2cMutex);
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            xSemaphoreGive(i2cMutex);
            handle_response(STATUS_OK, cmd, param_addr, data, data_len, use_uart1);
            break;
        }

        case 0x0802:
        {
            uint8_t cfg_flags = 0;

            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (fm24cl16b_read_byte(FRAM_ADDR_FLAGS, &cfg_flags) != 1) {
                xSemaphoreGive(i2cMutex);
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            xSemaphoreGive(i2cMutex);
            handle_response(STATUS_OK, cmd, param_addr, &cfg_flags, 1, use_uart1);
            break;
        }

        case 0x0803:
        {
            uint8_t flags = req[4];

            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (fm24cl16b_write_byte(FRAM_ADDR_FLAGS, flags) != 1) {
                xSemaphoreGive(i2cMutex);
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            xSemaphoreGive(i2cMutex);

            {
                EventBits_t bits_to_set = 0;
                EventBits_t bits_to_clear = EVT_EXT_RTC_PRESENT |
                                            EVT_FLASH_PRESENT |
                                            EVT_LCD_PRESENT |
                                            EVT_SHT40_PRESENT |
                                            EVT_BME280_PRESENT |
                                            EVT_ADC_PRESENT |
                                            EVT_CAN_PRESENT;

                if (flags & FRAM_FLAG_EXT_RTC_PRESENT) bits_to_set |= EVT_EXT_RTC_PRESENT;
                if (flags & FRAM_FLAG_FLASH_PRESENT)   bits_to_set |= EVT_FLASH_PRESENT;
                if (flags & FRAM_FLAG_DISPLAY_PRESENT) bits_to_set |= EVT_LCD_PRESENT;
                if (flags & FRAM_FLAG_SHT40_PRESENT)   bits_to_set |= EVT_SHT40_PRESENT;
                if (flags & FRAM_FLAG_BME280_PRESENT)  bits_to_set |= EVT_BME280_PRESENT;
                if (flags & FRAM_FLAG_ADC_PRESENT)     bits_to_set |= EVT_ADC_PRESENT;
                if (flags & FRAM_FLAG_CAN_PRESENT)     bits_to_set |= EVT_CAN_PRESENT;

                xEventGroupClearBits(appEvents, bits_to_clear);
                xEventGroupSetBits(appEvents, bits_to_set);
                xEventGroupSetBits(appEvents, EVT_LCD_REINIT | EVT_RTC_REINIT | EVT_BME280_REINIT);

                if (LCDTaskHandle != NULL) {
                    xTaskNotify(LCDTaskHandle, LCD_NOTIFY_REINIT, eSetBits);
                }

                if (flags & FRAM_FLAG_FLASH_PRESENT)
                {   
                    if (flash_log_init() != 1)
                    {
                        xEventGroupClearBits(appEvents, EVT_FLASH_PRESENT);
                    }
                }
            }

            handle_response(STATUS_OK, cmd, param_addr, &flags, 1, use_uart1);
            break;
        }

        case 0x0810:
        {
            uint32_t addr32 = ((uint32_t)req[4] << 24) |
                              ((uint32_t)req[5] << 16) |
                              ((uint32_t)req[6] << 8)  |
                              ((uint32_t)req[7]);
            uint8_t data_len = req[8];

            if (!(xEventGroupGetBits(appEvents) & EVT_FLASH_PRESENT)) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if ((data_len == 0U) || (data_len > 15U)) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if ((addr32 >= MX25_SIZE_BYTES) || (data_len > (MX25_SIZE_BYTES - addr32))) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (xSemaphoreTake(spiMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (mx25_write(addr32, &req[9], data_len) != 1) {
                xSemaphoreGive(spiMutex);
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            xSemaphoreGive(spiMutex);
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }

        case 0x0811:
        {
            uint32_t addr32 = ((uint32_t)req[4] << 24) |
                              ((uint32_t)req[5] << 16) |
                              ((uint32_t)req[6] << 8)  |
                              ((uint32_t)req[7]);
            uint8_t data_len = req[8];
            uint8_t data[16] = {0};

            if (!(xEventGroupGetBits(appEvents) & EVT_FLASH_PRESENT)) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if ((data_len == 0U) || (data_len > 16U)) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if ((addr32 >= MX25_SIZE_BYTES) || (data_len > (MX25_SIZE_BYTES - addr32))) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (xSemaphoreTake(spiMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            if (mx25_read(addr32, data, data_len) != 1) {
                xSemaphoreGive(spiMutex);
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            xSemaphoreGive(spiMutex);
            handle_response(STATUS_OK, cmd, param_addr, data, data_len, use_uart1);
            break;
        }

        case 0x0812:
        {
            uint32_t timestamp;
            int32_t temp;
            uint32_t hum;
            uint32_t press;

            if (!(xEventGroupGetBits(appEvents) & EVT_FLASH_PRESENT)) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            timestamp = ((uint32_t)req[4] << 24) |
                        ((uint32_t)req[5] << 16) |
                        ((uint32_t)req[6] << 8)  |
                        ((uint32_t)req[7]);

            temp = ((int32_t)req[8] << 24) |
                   ((int32_t)req[9] << 16) |
                   ((int32_t)req[10] << 8) |
                   ((int32_t)req[11]);

            hum = ((uint32_t)req[12] << 24) |
                  ((uint32_t)req[13] << 16) |
                  ((uint32_t)req[14] << 8)  |
                  ((uint32_t)req[15]);

            press = ((uint32_t)req[16] << 24) |
                    ((uint32_t)req[17] << 16) |
                    ((uint32_t)req[18] << 8)  |
                    ((uint32_t)req[19]);

            if (press == 0u) {
                sht40_data_t sht = {0};

                if ((temp < -32768L) || (temp > 32767L) || (hum > 65535u)) {
                    uint8_t err = 2;
                    handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                    break;
                }

                sht.temperature = (int16_t)temp;
                sht.humidity = (uint16_t)hum;

                if (flash_log_append(timestamp, NULL, &sht) != 1) {
                    uint8_t err = 3;
                    handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                    break;
                }
            } else {
                bme280_data_t bme = {0};

                bme.temperature = temp;
                bme.humidity = hum;
                bme.pressure = press;

                if (flash_log_append(timestamp, &bme, NULL) != 1) {
                    uint8_t err = 3;
                    handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                    break;
                }
            }

            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }

        case 0x0813:
        {
            flash_log_record_t rec = {0};
            uint8_t mode = req[4];
            uint32_t index;
            uint8_t data[24];
            int rc;

            if (!(xEventGroupGetBits(appEvents) & EVT_FLASH_PRESENT)) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (sizeof(flash_log_record_t) > FRAME_PAYLOAD_SIZE) {
                uint8_t err = 4;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            index = ((uint32_t)req[5] << 24) |
                    ((uint32_t)req[6] << 16) |
                    ((uint32_t)req[7] << 8)  |
                    ((uint32_t)req[8]);

            if (mode == 0u) {
                rc = flash_log_read_oldest(index, &rec);
            } else if (mode == 1u) {
                rc = flash_log_read_latest(index, &rec);
            } else {
                uint8_t err = 2;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (rc != 1) {
                uint8_t err = 3;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            data[0]  = (uint8_t)(rec.sequence >> 24);
            data[1]  = (uint8_t)(rec.sequence >> 16);
            data[2]  = (uint8_t)(rec.sequence >> 8);
            data[3]  = (uint8_t)(rec.sequence);
            data[4]  = (uint8_t)(rec.timestamp >> 24);
            data[5]  = (uint8_t)(rec.timestamp >> 16);
            data[6]  = (uint8_t)(rec.timestamp >> 8);
            data[7]  = (uint8_t)(rec.timestamp);
            data[8]  = (uint8_t)(((uint32_t)rec.temp_x100) >> 24);
            data[9]  = (uint8_t)(((uint32_t)rec.temp_x100) >> 16);
            data[10] = (uint8_t)(((uint32_t)rec.temp_x100) >> 8);
            data[11] = (uint8_t)(((uint32_t)rec.temp_x100));
            data[12] = (uint8_t)(rec.hum_x100 >> 24);
            data[13] = (uint8_t)(rec.hum_x100 >> 16);
            data[14] = (uint8_t)(rec.hum_x100 >> 8);
            data[15] = (uint8_t)(rec.hum_x100);
            data[16] = (uint8_t)(rec.press_pa >> 24);
            data[17] = (uint8_t)(rec.press_pa >> 16);
            data[18] = (uint8_t)(rec.press_pa >> 8);
            data[19] = (uint8_t)(rec.press_pa);
            data[20] = (uint8_t)(rec.crc32 >> 24);
            data[21] = (uint8_t)(rec.crc32 >> 16);
            data[22] = (uint8_t)(rec.crc32 >> 8);
            data[23] = (uint8_t)(rec.crc32);

            handle_response(STATUS_OK, cmd, param_addr, data, sizeof(data), use_uart1);
            break;
        }

        case 0x0814:
        {
            uint32_t cnt;
            uint8_t data[4];

            if (!(xEventGroupGetBits(appEvents) & EVT_FLASH_PRESENT)) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            cnt = flash_log_count();

            data[0] = (uint8_t)(cnt >> 24);
            data[1] = (uint8_t)(cnt >> 16);
            data[2] = (uint8_t)(cnt >> 8);
            data[3] = (uint8_t)(cnt);

            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }

        case 0x0815:
        {
            if (!(xEventGroupGetBits(appEvents) & EVT_FLASH_PRESENT)) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (flash_log_clear() != 1) {
                uint8_t err = 2;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }

        case 0x9999:
        {
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            HAL_Delay(100);
            NVIC_SystemReset();
            break;
        }

        default:
            handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
            break;
    }
}