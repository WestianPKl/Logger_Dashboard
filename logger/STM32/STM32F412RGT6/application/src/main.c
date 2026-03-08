#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"
#include "systick.h"
#include "inputs.h"
#include "support.h"
#include "version.h"
#include "outputs.h"
#include "timer.h"
#include "adc.h"
#include "uart.h"
#include "dma.h"
#include "i2c.h"
#include "lcd.h"
#include "sht40.h"
#include "rtc.h"
#include "bme280.h"
#include "spi.h"
#include "rtc_locale.h"
#include "ina.h"

#define STATUS_OK       0x40
#define ERROR_RESPONSE  0x7F

void btn1_handler(void);
void btn2_handler(void);

volatile uint32_t tick_10ms = 0;
volatile uint8_t lcd_reinit_5s_flag = 0;
static uint16_t tick_10ms_5s = 0;
volatile uint8_t esp32_on = 0;
volatile uint8_t esp32_toggle_flag = 0;
volatile uint16_t esp32_timer = 0;
volatile uint8_t lcd_refresh_flag = 0;
volatile uint8_t measure_flag_1s = 1;
volatile uint8_t measure_flag_10min = 0;
volatile uint8_t sht40_error_flag = 0;
volatile uint8_t bme280_error_flag = 0;
volatile uint8_t second_marker = 0;
volatile uint8_t backlight_on = 1;
volatile uint8_t backlight_toggle_flag = 0;
volatile uint8_t backlight_timer = 0;
// volatile uint8_t hb = 0;

struct {
    int16_t temperature;
    uint16_t humidity;
} measurement_sht40;

struct
{
    int32_t temperature;
    uint32_t humidity;
    uint32_t pressure;
} measurement_bme280;

struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} datetime;

volatile uint8_t rtc_wakeup_flag = 0;
volatile uint8_t rtc_alarm_flag = 0;
volatile uint8_t rtc_tampstamp_flag = 0;

struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} date;

volatile uint8_t btn1_pressed = 0;
volatile uint8_t btn2_pressed = 0;

volatile uint8_t led1_state = 0;
volatile uint8_t led2_state = 0;
volatile uint8_t pb12_state = 0;
volatile uint8_t pc0_state = 0;
volatile uint8_t pc1_state = 0;
volatile uint8_t pc2_state = 0;
volatile uint8_t pc3_state = 0;
volatile uint8_t esp32_state = 0;

static uint8_t rgb_r = 0;
static uint8_t rgb_g = 0;
static uint8_t rgb_b = 0;

volatile uint8_t uart2_tx_busy = 0;
volatile uint8_t uart1_tx_busy = 0;

volatile uint8_t i2c1_dma_tx_done = 0;
volatile uint8_t i2c1_dma_rx_done = 0;
volatile uint8_t i2c1_dma_err     = 0;

volatile uint8_t spi1_dma_rx_done = 0;
volatile uint8_t spi1_dma_tx_done = 0;

#define FRAME_LEN_APP   24
#define FRAME_PAYLOAD   (FRAME_LEN_APP - 5)

#define ADC_BUFFER_SIZE 4
static uint16_t adc_data_buffer[ADC_BUFFER_SIZE];

#define UART2_RX_BUFFER_SIZE 128
static uint8_t uart2_rx_buf[UART2_RX_BUFFER_SIZE];
static uint8_t uart2_tx_frame[FRAME_LEN_APP];
static volatile uint16_t uart2_rx_old_pos = 0;

#define UART2_RX_FRAME_LEN FRAME_LEN_APP
static uint8_t uart2_frame_acc[UART2_RX_FRAME_LEN];
static uint8_t uart2_frame_idx = 0;

#define UART1_RX_BUFFER_SIZE 128
static uint8_t uart1_rx_buf[UART1_RX_BUFFER_SIZE];
static uint8_t uart1_tx_frame[FRAME_LEN_APP];
static volatile uint16_t uart1_rx_old_pos = 0;

#define UART1_RX_FRAME_LEN FRAME_LEN_APP
static uint8_t uart1_frame_acc[UART1_RX_FRAME_LEN];
static uint8_t uart1_frame_idx = 0;

static int uart2_dma_send(const uint8_t *data, uint16_t len);
static int uart1_dma_send(const uint8_t *data, uint16_t len);
static void uart2_process_rx(void);
static void uart1_process_rx(void);

static void handle_request(const uint8_t *req, uint8_t use_uart1);
static void handle_response(uint8_t status, uint8_t cmd, uint8_t param,
                            const uint8_t *payload, uint32_t payload_len, uint8_t use_uart1);

static int uart2_dma_send(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return -1;
    if (len > sizeof(uart2_tx_frame)) len = sizeof(uart2_tx_frame);

    uint32_t t = 2000000U;
    while (uart2_tx_busy) {
        if (--t == 0U) return -1;
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

    uint32_t t = 2000000U;
    while (uart1_tx_busy) {
        if (--t == 0U) return -1;
    }

    uart1_tx_busy = 1;
    memcpy(uart1_tx_frame, data, len);
    dma2_uart1_tx_start(uart1_tx_frame, len);
    return 1;
}

static void handle_response(uint8_t status, uint8_t cmd, uint8_t param,
                            const uint8_t *payload, uint32_t payload_len, uint8_t use_uart1)
{
    static uint8_t resp[FRAME_LEN_APP];
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
        case 0x0000: /* Ping */
        {
            uint8_t data[3] = {0xAA, 0xAA, 0xAA};
            handle_response(STATUS_OK, cmd, param_addr, data, sizeof(data), use_uart1);
            break;
        }

        case 0x0100: /* Read device serial number */
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

        case 0x0101: /* Read firmware and hardware version */
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

        case 0x0102: /* Read firmware build date */
        {
            uint8_t payload[10] = {0};
            memcpy(payload, FW_BUILD_DATE, 10);
            handle_response(STATUS_OK, cmd, param_addr, payload, 10, use_uart1);
            break;
        }

        case 0x0103: /* Read production date */
        {
            const device_info_t *info = device_info_get();
            uint8_t payload[10] = {0};
            if (info->magic == INFO_MAGIC) memcpy(payload, info->prod_date, 10);
            handle_response(STATUS_OK, cmd, param_addr, payload, 10, use_uart1);
            break;
        }

        case 0x0200: /* Read ADC values */
        {
            uint16_t v0 = adc_data_buffer[0];
            uint16_t v1 = adc_data_buffer[1];
            uint8_t data[4] = {(uint8_t)(v0 >> 8), (uint8_t)v0, (uint8_t)(v1 >> 8), (uint8_t)v1};
            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }
        case 0x0201: /* Read BTN1 input status */
        {
            uint8_t btn_pressed = GPIOB->IDR & (1U << 0U) ? 0U : 1U;
            uint8_t data[1] = { btn_pressed ? 1U : 0U };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0202: /* Read BTN2 input status */
        {
            uint8_t btn_pressed = GPIOB->IDR & (1U << 1U) ? 0U : 1U;
            uint8_t data[1] = { btn_pressed ? 1U : 0U };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0203: /* Read ESP32 input status */
        {
            uint8_t esp32_input = (GPIOC->IDR & (1U << 5U)) ? 1U : 0U;
            uint8_t data[1] = { esp32_input ? 1U : 0U };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0300: /* Read Temperature/Humidity (SHT40) */
        {
            int16_t temp_c_x100 = 0;
            uint16_t rh_x100 = 0;

            uint8_t e = sht40_data_read_int(&temp_c_x100, &rh_x100);
            if (e != 0) {
                uint8_t err = e;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            uint8_t data[4];
            data[0] = (uint8_t)((temp_c_x100 >> 8) & 0xFF);
            data[1] = (uint8_t)( temp_c_x100       & 0xFF);
            data[2] = (uint8_t)((rh_x100 >> 8) & 0xFF);
            data[3] = (uint8_t)( rh_x100       & 0xFF);

            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }
        case 0x0301: /* Read Temperature/Humidity/Pressure (BME280) */
        {
            uint8_t id = bme280_read_id();
            if (id != 0x60) {
                uint8_t err = 1;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            bme280_trigger_forced();
            systick_delay_ms(10);

            int32_t temp_c;
            uint32_t hum_pct, press_q24_8;

            if (bme280_read_data(&temp_c, &hum_pct, &press_q24_8) != 0) {
                uint8_t err = 2;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            if (temp_c < -4000 || temp_c > 8500 || hum_pct > 102400U || press_q24_8 == 0) {
                uint8_t err = 3;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            uint8_t data[12];
            data[0]  = (uint8_t)((temp_c >> 24) & 0xFF);
            data[1]  = (uint8_t)((temp_c >> 16) & 0xFF);
            data[2]  = (uint8_t)((temp_c >> 8) & 0xFF);
            data[3]  = (uint8_t)(temp_c & 0xFF);

            data[4]  = (uint8_t)((hum_pct >> 24) & 0xFF);
            data[5]  = (uint8_t)((hum_pct >> 16) & 0xFF);
            data[6]  = (uint8_t)((hum_pct >> 8) & 0xFF);
            data[7]  = (uint8_t)(hum_pct & 0xFF);

            data[8]  = (uint8_t)((press_q24_8 >> 24) & 0xFF);
            data[9]  = (uint8_t)((press_q24_8 >> 16) & 0xFF);
            data[10] = (uint8_t)((press_q24_8 >> 8) & 0xFF);
            data[11] = (uint8_t)(press_q24_8 & 0xFF);

            handle_response(STATUS_OK, cmd, param_addr, data, 12, use_uart1);
            break;
        }
        case 0x0400: /* Read output states: LED1, LED2, R, G, B */
        {
            uint8_t data[5] = { led1_state, led2_state, rgb_r, rgb_g, rgb_b };
            handle_response(STATUS_OK, cmd, param_addr, data, 5, use_uart1);
            break;
        }
        case 0x0401: /* Write LED1: req[4] = VALUE */
        {
            uint8_t val = req[4] ? 1U : 0U;
            if (val) pin_set_high('B', 14U);
            else     pin_set_low('B', 14U);
            led1_state = val;
            uint8_t data[1] = { led1_state };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0402: /* Write LED2: req[4] = VALUE */
        {
            uint8_t val = req[4] ? 1U : 0U;
            if (val) pin_set_high('B', 15U);
            else     pin_set_low('B', 15U);
            led2_state = val;
            uint8_t data[1] = { led2_state };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0403: /* Write PB12: req[4] = VALUE */
        {
            uint8_t val = req[4] ? 1U : 0U;
            if (val) pin_set_high('B', 12U);
            else     pin_set_low('B', 12U);
            pb12_state = val;
            uint8_t data[1] = { pb12_state };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0404: /* Write PC0: req[4] = VALUE */
        {
            uint8_t val = req[4] ? 1U : 0U;
            if (val) pin_set_high('C', 0U);
            else     pin_set_low('C', 0U);
            pc0_state = val;
            uint8_t data[1] = { pc0_state };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0405: /* Write PC1: req[4] = VALUE */
        {
            uint8_t val = req[4] ? 1U : 0U;
            if (val) pin_set_high('C', 1U);
            else     pin_set_low('C', 1U);
            pc1_state = val;
            uint8_t data[1] = { pc1_state };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0406: /* Write PC2: req[4] = VALUE */
        {
            uint8_t val = req[4] ? 1U : 0U;
            if (val) pin_set_high('C', 2U);
            else     pin_set_low('C', 2U);
            pc2_state = val;
            uint8_t data[1] = { pc2_state };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0407: /* Write PC3: req[4] = VALUE */
        {
            uint8_t val = req[4] ? 1U : 0U;
            if (val) pin_set_high('C', 3U);
            else     pin_set_low('C', 3U);
            pc3_state = val;
            uint8_t data[1] = { pc3_state };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0408: /* Write ESP32: req[4] = VALUE */
        {
            uint8_t val = req[4] ? 1U : 0U;
            if (val) pin_set_high('C', 4U);
            else     pin_set_low('C', 4U);
            esp32_state = val;
            uint8_t data[1] = { esp32_state };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0501: /* Set TIM1 CH1 req[4]=duty cycle*/
        {
            uint8_t duty = req[4];
            timer1_pwm_ch1_set_duty(duty);
            uint8_t data[1] = { duty };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0502: /* Set TIM2 CH3 req[4]=duty cycle*/
        {
            uint8_t duty = req[4];
            timer2_pwm_ch3_set_duty(duty);
            uint8_t data[1] = { duty };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0503: /* Set TIM4 CH3 req[4]=duty cycle*/
        {
            uint8_t duty = req[4];
            timer4_pwm_ch3_set_duty(duty);
            uint8_t data[1] = { duty };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0504: /* Set TIM4 CH4 req[4]=duty cycle*/
        {
            uint8_t duty = req[4];
            timer4_pwm_ch4_set_duty(duty);
            uint8_t data[1] = { duty };
            handle_response(STATUS_OK, cmd, param_addr, data, 1, use_uart1);
            break;
        }
        case 0x0505: /* Set RGB color: req[4]=R, req[5]=G, req[6]=B */
        {
            rgb_r = req[4];
            rgb_g = req[5];
            rgb_b = req[6];
            timer3_pwm_set_color(rgb_r, rgb_g, rgb_b);
            uint8_t data[3] = { rgb_r, rgb_g, rgb_b };
            handle_response(STATUS_OK, cmd, param_addr, data, 3, use_uart1);
            break;
        }
        case 0x0506: /* Set buzzer: req[4..5]=freq BE, req[6]=volume 0-100 */
        {
            if (req[4] == 0 && req[5] == 0) {
                timer3_pwm_set_buzzer_freq(0, 0);
                handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }
            uint16_t freq = ((uint16_t)req[4] << 8) | req[5];
            uint8_t  vol  = req[6];
            if (vol > 100U) vol = 100U;
            timer3_pwm_set_buzzer_freq((uint32_t)freq, (uint32_t)vol);
            uint8_t data[3] = { req[4], req[5], vol };
            handle_response(STATUS_OK, cmd, param_addr, data, 3, use_uart1);
            break;
        }
        case 0x0600: /* Read RTC Date and Time -> payload[0..6] = YY MM DD WD hh mm ss */
        {
            uint8_t year, month, day, weekday;
            uint8_t hours, minutes, seconds;

            rtc_read_datetime(&year, &month, &day, &weekday, &hours, &minutes, &seconds);

            uint8_t data[7];
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
        case 0x0601: /* Write RTC Date and Time (req[4..10] = YY MM DD WD hh mm ss) */
        {
            uint8_t year    = req[4];
            uint8_t month   = req[5];
            uint8_t day     = req[6];
            uint8_t weekday = req[7];
            uint8_t hours   = req[8];
            uint8_t minutes = req[9];
            uint8_t seconds = req[10];

            if (year > 99U ||
                month < 1U || month > 12U ||
                day   < 1U || day   > 31U ||
                weekday < 1U || weekday > 7U ||
                hours > 23U || minutes > 59U || seconds > 59U) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            __disable_irq();
            int rc = rtc_set_datetime(year, month, day, weekday, hours, minutes, seconds);
            __enable_irq();

            if (rc != 0) {
                uint8_t err = (uint8_t)(-rc);
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            uint8_t data[7] = { year, month, day, weekday, hours, minutes, seconds };
            handle_response(STATUS_OK, cmd, param_addr, data, 7, use_uart1);
            break;
        }
        case 0x0602: /* SET wakeup seconds: payload[0..1] = seconds uint16 BE */
        {
            uint16_t sec = ((uint16_t)req[4] << 8) | req[5];
            if (rtc_wakeup_start_seconds(sec) != 0) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }
        case 0x0603: /* ALARM A set: payload = hh mm ss + daily(0/1) */
        {
            uint8_t hh = req[4];
            uint8_t mm = req[5];
            uint8_t ss = req[6];
            uint8_t daily = req[7];

            if (rtc_alarmA_set_hms(hh, mm, ss, daily ? 1U : 0U) != 0) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }
        case 0x0604: /* ALARM A off */
        {
            rtc_alarmA_disable();
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }
        case 0x0605: /* GET timestamp -> 7 bytes: YY MM DD WD hh mm ss (YY=0xFF) */
        {
            uint8_t mo, dd, wd, hh, mi, ss;
            int r = rtc_timestamp_read(&mo, &dd, &wd, &hh, &mi, &ss);
            if (r != 0) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }
            uint8_t data[7] = { 0xFF, mo, dd, wd, hh, mi, ss };
            handle_response(STATUS_OK, cmd, param_addr, data, 7, use_uart1);
            break;
        }
        case 0x7000: /* INA226 read */
        {
            uint16_t id = 0, cal = 0;
            int r = ina226_id(&id, &cal);
            if (r < 0) {
                uint8_t err = (uint8_t)(-r);
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }
            uint32_t bus_uV = ina226_bus_uV();
            int32_t shunt_uV = ina226_shunt_uV();
            int32_t current_uA = ina226_current_uA();
            uint32_t power_uW = ina226_power_uW();

            uint8_t data[18];
            data[0] = (bus_uV >> 24) & 0xFF;
            data[1] = (bus_uV >> 16) & 0xFF;
            data[2] = (bus_uV >> 8) & 0xFF;
            data[3] = bus_uV & 0xFF;
            data[4] = (shunt_uV >> 24) & 0xFF;
            data[5] = (shunt_uV >> 16) & 0xFF;
            data[6] = (shunt_uV >> 8) & 0xFF;
            data[7] = shunt_uV & 0xFF;
            data[8] = (current_uA >> 24) & 0xFF;
            data[9] = (current_uA >> 16) & 0xFF;
            data[10] = (current_uA >> 8) & 0xFF;
            data[11] = current_uA & 0xFF;
            data[12] = (power_uW >> 24) & 0xFF;
            data[13] = (power_uW >> 16) & 0xFF;
            data[14] = (power_uW >> 8) & 0xFF;
            data[15] = power_uW & 0xFF;
            data[16] = (id >> 8) & 0xFF;
            data[17] = id & 0xFF;

            handle_response(STATUS_OK, cmd, param_addr, data, 18, use_uart1);
            break;
        }
        default:
            handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
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
        if (uart2_frame_idx == UART2_RX_FRAME_LEN) {
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
        if (uart1_frame_idx == UART1_RX_FRAME_LEN) {
            if (crc8_atm(uart1_frame_acc, FRAME_LEN_APP - 1) ==
                uart1_frame_acc[FRAME_LEN_APP - 1]) {
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

    btn1_irq_init();
    btn2_irq_init();

    timer13_init_10ms();

    timer3_pwm_ch1_init(84, 1000);
    timer3_pwm_ch2_init(84, 1000);
    timer3_pwm_ch3_init(84, 1000);

    timer1_pwm_ch1_init(84, 1000);
    timer2_pwm_ch3_init(84, 1000);
    timer4_pwm_ch3_init(84, 1000);
    timer4_pwm_ch4_init(84, 1000);

    dma1_init();
    dma2_init();

    adc1_init(1, adc_data_buffer, ADC_BUFFER_SIZE);
    adc1_start_conversion();

    uart1_rxtx_init();
    uart2_rxtx_init();

    dma1_uart2_rx_config(uart2_rx_buf, UART2_RX_BUFFER_SIZE);
    dma2_uart1_rx_config(uart1_rx_buf, UART1_RX_BUFFER_SIZE);

    i2c1_init();
    dma_i2c1_rx_init();
    dma_i2c1_tx_init();

    lcd_init();
    lcd_set_cursor(0, 0);
    lcd_send_string("Initializing...");

    rtc_init();

    spi1_init();
    dma_spi1_rx_init();
    dma_spi1_tx_init();

    __enable_irq();

    bme280_init();

    ina226_init(0x40, 500, 2000);

    while (1) {
        if (btn1_pressed) { btn1_pressed = 0; btn1_handler(); }
        if (btn2_pressed) { btn2_pressed = 0; btn2_handler(); }

        uart2_process_rx();
        uart1_process_rx();

        if(measure_flag_1s){
            measure_flag_1s = 0;

            if (lcd_reinit_5s_flag) {
                lcd_reinit_5s_flag = 0;

                if (!lcd_is_present()) {
                    lcd_mark_present(1);
                    lcd_init();
                    if (lcd_is_present()) {
                        backlight_toggle_flag = 1;
                        backlight_on = 1;
                        lcd_clear();
                    }
                }
            }

            int16_t  temp_c_x100 = 0;
            uint16_t rh_x100     = 0;

            uint8_t e = sht40_data_read_int(&temp_c_x100, &rh_x100);
            if (e != 0) {
                measurement_sht40.temperature = 0;
                measurement_sht40.humidity    = 0;
                sht40_error_flag = 1;

            } else {
                sht40_error_flag = 0;
                measurement_sht40.temperature = temp_c_x100;
                measurement_sht40.humidity    = rh_x100;
            }

            rtc_read_datetime(&datetime.year, &datetime.month, &datetime.day, &datetime.weekday,
                &datetime.hours, &datetime.minutes, &datetime.seconds);

            rtc_utc_to_warsaw(&datetime.year, &datetime.month, &datetime.day, &datetime.weekday,
                &datetime.hours, &datetime.minutes, &datetime.seconds);


            int32_t  temp_c;
            uint32_t hum_x1024, press_q24_8;

            bme280_error_flag = 1;

            uint8_t id = bme280_read_id();
            if (id == 0x60) {

                bme280_trigger_forced();
                systick_delay_ms(10);

                if (bme280_read_data(&temp_c, &hum_x1024, &press_q24_8) == 0) {

                    uint32_t hum_x100 = (hum_x1024 * 100U) / 1024U;

                    if (!(temp_c < -4000 || temp_c > 8500 || hum_x100 > 10000 || press_q24_8 == 0)) {
                        measurement_bme280.temperature = temp_c;
                        measurement_bme280.humidity    = hum_x100;
                        measurement_bme280.pressure    = press_q24_8;
                        bme280_error_flag = 0;
                    }
                }
            }

        }

        if (backlight_toggle_flag) {
            backlight_toggle_flag = 0;
            lcd_backlight(backlight_on);
        }

        if (lcd_refresh_flag && lcd_is_present()) {
            lcd_refresh_flag = 0;
            if (second_marker) second_marker = 0;
            else                second_marker = 1;

            uint16_t current_year = 2000 + datetime.year;
            
            if (current_year > 2000){
                lcd_set_cursor(0, 0);
                lcd_send_decimal(current_year, 4);
                lcd_send_string("-");
                lcd_send_decimal(datetime.month, 2);
                lcd_send_string("-");
                lcd_send_decimal(datetime.day, 2);
                lcd_send_string(" ");
                lcd_send_decimal(datetime.hours, 2);
                if(second_marker) lcd_send_string(":");
                else lcd_send_string(" ");
                lcd_send_decimal(datetime.minutes, 2);

                if (sht40_error_flag) {
                    lcd_set_cursor(0, 1);
                    lcd_send_string("SHT40 ERROR   ");
                } else {
                    lcd_set_cursor(0, 1);
                    lcd_send_string("TH:");
                    lcd_send_string(" ");
                    lcd_send_temp_1dp_from_x100(measurement_sht40.temperature);
                    lcd_send_string(" ");
                    lcd_send_hum_1dp_from_x100(measurement_sht40.humidity);
                    lcd_send_string("  ");
                }
            } else {
                lcd_set_cursor(0, 0);
                lcd_send_string("Initializing... ");
                lcd_set_cursor(0, 1);
                lcd_send_string("                ");
            }



            // if (bme280_error_flag) {
            //     lcd_set_cursor(0, 1);
            //     lcd_send_string("BME280 ERROR  ");
            // } else {
            //     lcd_set_cursor(0, 1);
            //     lcd_send_string("TH:");
            //     lcd_send_string(" ");
            //     lcd_send_temp_1dp_from_x100((int16_t)measurement_bme280.temperature);
            //     lcd_send_string(" ");
            //     lcd_send_hum_1dp_from_x100((uint16_t)measurement_bme280.humidity);
            //     lcd_send_string(" ");
            // }
        }

        if (measure_flag_10min) {
            measure_flag_10min = 0;

            esp32_status_set(0);
            esp32_timer = 1;
            esp32_on = 1;
        }
    }
}

void btn1_handler(void)
{
    systick_delay_ms(10);
    if (GPIOB->IDR & (1U << 0U)) return;

    backlight_on = 1;
    backlight_timer = 0;
    backlight_toggle_flag = 1;
}

void btn2_handler(void)
{
    systick_delay_ms(10);
    if (GPIOB->IDR & (1U << 1U)) return;

    if (led2_state) { pin_set_low('B', 15U); led2_state = 0; }
    else            { pin_set_high('B',15U); led2_state = 1; }
}

void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR0) { EXTI->PR = EXTI_PR_PR0; btn1_pressed = 1; }
}

void EXTI1_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR1) { EXTI->PR = EXTI_PR_PR1; btn2_pressed = 1; }
}

void DMA2_Stream0_IRQHandler(void)
{
    if (DMA2->LISR & DMA_LISR_TCIF0) {
        DMA2->LIFCR = DMA_LIFCR_CTCIF0;
    }
}

void DMA1_Stream6_IRQHandler(void)
{
    if (DMA1->HISR & DMA_HISR_TCIF6) {
        DMA1->HIFCR = DMA_HIFCR_CTCIF6;
        uart2_tx_busy = 0;
    }
}

void DMA2_Stream7_IRQHandler(void)
{
    if (DMA2->HISR & DMA_HISR_TCIF7) {
        DMA2->HIFCR = DMA_HIFCR_CTCIF7;
        uart1_tx_busy = 0;
    }
}

void DMA1_Stream0_IRQHandler(void)
{
    if (DMA1->LISR & DMA_LISR_TEIF0) {
        DMA1->LIFCR = DMA_LIFCR_CTEIF0 | DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0;
        i2c1_dma_err = 1;
        i2c1_dma_rx_done = 1;
        return;
    }
    if (DMA1->LISR & DMA_LISR_TCIF0) {
        DMA1->LIFCR = DMA_LIFCR_CTCIF0;
        i2c1_dma_rx_done = 1;
    }
}

void DMA1_Stream1_IRQHandler(void)
{
    if (DMA1->LISR & DMA_LISR_TEIF1) {
        DMA1->LIFCR = DMA_LIFCR_CTEIF1 | DMA_LIFCR_CTCIF1 | DMA_LIFCR_CHTIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CFEIF1;
        i2c1_dma_err = 1;
        i2c1_dma_tx_done = 1;
        return;
    }
    if (DMA1->LISR & DMA_LISR_TCIF1) {
        DMA1->LIFCR = DMA_LIFCR_CTCIF1;
        i2c1_dma_tx_done = 1;
    }
}

void TIM8_UP_TIM13_IRQHandler(void)
{
    if (TIM13->SR & TIM_SR_UIF) {
        TIM13->SR &= ~TIM_SR_UIF;

        tick_10ms++;
        tick_10ms_5s++;
        if (tick_10ms_5s >= 500U) {
            tick_10ms_5s = 0;
            lcd_reinit_5s_flag = 1;
        }

        if ((tick_10ms % 100U) == 0U) {
            measure_flag_1s = 1;
            lcd_refresh_flag = 1;

            if (backlight_on) {
                backlight_timer++;

                if (backlight_timer >= 20U) {
                    backlight_on = 0;
                    backlight_timer = 0;
                    backlight_toggle_flag = 1;
                }
            }
        }

        if (esp32_timer > 0) {
            esp32_timer--;
            if (esp32_timer == 0) {
                if (esp32_on) {
                    esp32_status_set(1);
                    esp32_timer = 2;
                    esp32_on = 0;
                } else {
                    esp32_status_set(0);
                }
            }
        }

        if ((tick_10ms % 52000U) == 0U) {
            measure_flag_10min = 1;
        }
    }
}

void RTC_WKUP_IRQHandler(void)
{
    if (RTC->ISR & RTC_ISR_WUTF) {
        rtc_write_protect_disable();
        RTC->ISR &= ~RTC_ISR_WUTF;
        rtc_write_protect_enable();

        rtc_exti_clear(20U);
        rtc_wakeup_flag = 1;
    }
}

void RTC_Alarm_IRQHandler(void)
{
    if (RTC->ISR & RTC_ISR_ALRAF) {
        rtc_write_protect_disable();
        RTC->ISR &= ~RTC_ISR_ALRAF;
        rtc_write_protect_enable();

        rtc_exti_clear(18U);
        rtc_alarm_flag = 1;
    }
}

void TAMP_STAMP_IRQHandler(void)
{
    rtc_exti_clear(19U);
    rtc_tampstamp_flag = 1;
}

void DMA2_Stream2_IRQHandler(void)
{
    if (DMA2->LISR & DMA_LISR_TCIF2) {
        spi1_dma_rx_done = 1;
        DMA2->LIFCR = DMA_LIFCR_CTCIF2;
    }
    if (DMA2->LISR & DMA_LISR_TEIF2) {
        DMA2->LIFCR = DMA_LIFCR_CTEIF2;
    }
}

void DMA2_Stream3_IRQHandler(void)
{
    if (DMA2->LISR & DMA_LISR_TCIF3) {
        spi1_dma_tx_done = 1;
        DMA2->LIFCR = DMA_LIFCR_CTCIF3;
    }
    if (DMA2->LISR & DMA_LISR_TEIF3) {
        DMA2->LIFCR = DMA_LIFCR_CTEIF3;
    }
}