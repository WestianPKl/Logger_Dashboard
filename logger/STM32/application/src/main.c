#include <stdint.h>
#include <string.h>
#include "stm32l4xx.h"
#include "systick.h"
#include "dma.h"
#include "adc_dma.h"
#include "timer.h"
#include "uart_dma.h"
#include "spi_dma.h"
#include "bme280_dma.h"
#include "i2c_dma.h"
#include "sht40.h"
#include "irq.h"
#include "lcd.h"
#include "rtc.h"
#include "pcf8563t_dma.h"
#include "support.h"
#include "version.h"

#define STATUS_OK       0x40
#define ERROR_RESPONSE  0x7F

volatile uint8_t button_pressed = 0;
volatile uint8_t irq_counter = 0;

volatile uint8_t dma2_adc_transfer_complete = 0;

volatile uint8_t spi1_dma_rx_done = 0;
volatile uint8_t spi1_dma_tx_done = 0;

volatile uint8_t uart2_rx_pending = 0;
volatile uint8_t uart2_tx_busy = 0;

volatile uint8_t uart1_rx_pending = 0;
volatile uint8_t uart1_tx_busy = 0;

volatile uint8_t i2c1_dma_tx_done = 0;
volatile uint8_t i2c1_dma_rx_done = 0;
volatile uint8_t i2c1_dma_err     = 0;

#define ADC_BUFFER_SIZE 2
static uint16_t adc_data_buffer[ADC_BUFFER_SIZE];
static volatile uint16_t last_adc_value = 0;

#define UART2_RX_BUFFER_SIZE 128
static uint8_t uart2_rx_buf[UART2_RX_BUFFER_SIZE];
static uint8_t uart2_tx_frame[16];
static volatile uint16_t uart2_rx_old_pos = 0;

#define UART2_RX_FRAME_LEN 16
static uint8_t uart2_frame_acc[UART2_RX_FRAME_LEN];
static uint8_t uart2_frame_idx = 0;

#define UART1_RX_BUFFER_SIZE 128
static uint8_t uart1_rx_buf[UART1_RX_BUFFER_SIZE];
static uint8_t uart1_tx_frame[16];
static volatile uint16_t uart1_rx_old_pos = 0;

#define UART1_RX_FRAME_LEN 16
static uint8_t uart1_frame_acc[UART1_RX_FRAME_LEN];
static uint8_t uart1_frame_idx = 0;

static volatile uint32_t adc_seq = 0;

static volatile uint8_t rtc_wakeup_flag = 0;
static volatile uint8_t rtc_alarm_flag = 0;
static volatile uint8_t rtc_tampstamp_flag = 0;

void button_handler(void);
static void uart2_dma_send(const uint8_t *data, uint16_t len);
static void uart2_process_rx(void);
static void uart1_dma_send(const uint8_t *data, uint16_t len);
static void uart1_process_rx(void);
static void handle_request(const uint8_t *req, uint8_t use_uart1);
static void handle_response(uint8_t status, uint8_t cmd, uint8_t param_addr,
                            const uint8_t *payload, uint32_t payload_len, uint8_t use_uart1);

static inline void led_off(uint8_t pin)
{
    GPIOA->BSRR = (1U << (pin + 16U));
}

static inline void led_on(uint8_t pin)
{
    GPIOA->BSRR = (1U << pin);
}

static void uart2_dma_send(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return;
    if (len > sizeof(uart2_tx_frame)) len = sizeof(uart2_tx_frame);

    while (uart2_tx_busy) {}
    uart2_tx_busy = 1;

    memcpy(uart2_tx_frame, data, len);
    dma1_uart2_tx_start((uint32_t)uart2_tx_frame, len);
}

static void uart1_dma_send(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return;
    if (len > sizeof(uart1_tx_frame)) len = sizeof(uart1_tx_frame);

    while (uart1_tx_busy) {}
    uart1_tx_busy = 1;

    memcpy(uart1_tx_frame, data, len);
    dma1_uart1_tx_start((uint32_t)uart1_tx_frame, len);
}

static void handle_response(uint8_t status, uint8_t cmd, uint8_t param,
                            const uint8_t *payload, uint32_t payload_len, uint8_t use_uart1)
{
    static uint8_t resp[16];
    memset(resp, 0, sizeof(resp));

    resp[0] = DEV_ADDR;
    resp[1] = status;
    resp[2] = cmd;
    resp[3] = param;

    if (payload && payload_len) {
        if (payload_len > 12) payload_len = 12;
        memcpy(&resp[4], payload, payload_len);
    }

    resp[15] = crc8_atm(resp, 15);

    if (use_uart1)
    {
        uart1_dma_send(resp, 16);
    }
    else
    {
        uart2_dma_send(resp, 16);
    }
}

static void handle_request(const uint8_t *req, uint8_t use_uart1)
{
    uint8_t addr  = req[0];
    uint8_t cmd   = req[2];
    uint8_t param_addr = req[3];
    uint8_t param   = req[4];
    uint16_t cmd_combined = ((uint16_t)cmd << 8) | param_addr;

    if (addr != DEV_ADDR) {
        return;
    }

    switch (cmd_combined)
    {
        case 0x0000: {
            uint8_t data[3] = {0xAA, 0xAA, 0xAA};
            handle_response(STATUS_OK, cmd, param_addr, data, sizeof(data), use_uart1);
            GPIOA->ODR ^= (1U << 5U);
            break;
        }
        case 0x0100: /* Read serial number */
        {
            const device_info_t *info = device_info_get();

            uint32_t serial = 0;
            if (info->magic == INFO_MAGIC) {
                serial = info->serial;
            }
            
            uint8_t serial_bytes[4];
            serial_bytes[0] = (uint8_t)((serial >> 24) & 0xFF);
            serial_bytes[1] = (uint8_t)((serial >> 16) & 0xFF);
            serial_bytes[2] = (uint8_t)((serial >> 8) & 0xFF);
            serial_bytes[3] = (uint8_t)(serial & 0xFF);
            handle_response(STATUS_OK, cmd, param_addr, serial_bytes, sizeof(serial_bytes), use_uart1);
            break;
        }
        case 0x0101: /* Read firmware and hardware version + device info */
        {

            const device_info_t *info = device_info_get();

            uint8_t hwmaj = 0;
            uint8_t hwmin = 0;
            if (info->magic == INFO_MAGIC) {
                hwmaj = info->hw_major;
                hwmin = info->hw_minor;
            }

            uint8_t payload[5] = {0};

            payload[0] = FW_VERSION_MAJOR;
            payload[1] = FW_VERSION_MINOR;
            payload[2] = FW_VERSION_PATCH;

            payload[3] = hwmaj;
            payload[4] = hwmin;

            handle_response(STATUS_OK, cmd, param_addr, payload, sizeof(payload), use_uart1);
            break;
        }
        case 0x0102: /* Read firmware build date (ASCII, first 10 chars) */
        {
            uint8_t payload[10] = {0};
            memcpy(payload, FW_BUILD_DATE, sizeof(payload));
            handle_response(STATUS_OK, cmd, param_addr, payload, sizeof(payload), use_uart1);
            break;
        }
        case 0x0103: /* Read production date (10 ASCII) */
        {
            const device_info_t *info = device_info_get();
            uint8_t payload[10] = {0};

            if (info->magic == INFO_MAGIC) {
                memcpy(payload, info->prod_date, 10);
            }
            handle_response(STATUS_OK, cmd, param_addr, payload, 10, use_uart1);
            break;
        }
        case 0x0200: /* Read ADC value */
        {
            uint16_t v0 = adc_data_buffer[0];
            uint16_t v1 = adc_data_buffer[1];
            uint8_t data[4] = {
                (uint8_t)(v0 >> 8), (uint8_t)v0,
                (uint8_t)(v1 >> 8), (uint8_t)v1
            };
            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }
        case 0x0300: /* Read Temperature and Humidity SHT40 */
        {
            int16_t temp_c_x100 = 0;
            uint16_t rh_x100 = 0;

            uint8_t e = sht40_data_read_int(&temp_c_x100, &rh_x100);
            if (e != 0) {
                uint8_t err = e;
                handle_response(ERROR_RESPONSE, cmd, param_addr, &err, 1, use_uart1);
                break;
            }

            uint16_t t = (uint16_t)temp_c_x100;
            uint8_t data[4];
            data[0] = (uint8_t)(t >> 8);
            data[1] = (uint8_t)(t);
            data[2] = (uint8_t)(rh_x100 >> 8);
            data[3] = (uint8_t)(rh_x100);

            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }
        case 0x0401: /* Write LED state ON */
        {
            if (param == 0x01) 
            {
                led_on(5U);
                handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
                
            } else if (param == 0x00)
            {
                led_off(5U);
                handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            } else 
            {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
            }
            break;
        }
        case 0x0402: /* Set LED PWM (param=0..255) on CH1 */
        {
            uint32_t period = TIM2->ARR + 1U;
            uint32_t ccr = ((uint32_t)param * period) / 255U;
            if (ccr >= period) ccr = period - 1U;

            tim2_pa0_pa1_pwm_set_duty(1U, (int32_t)ccr);
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }
        case 0x0500: /* Read Temperature/Humidity/Pressure (BME280) */
        {
            bme280_trigger_forced();

            systick_delay_ms(50);

            int32_t temp_c;
            uint32_t hum_pct, press_q24_8;
            if (bme280_read_data(&temp_c, &hum_pct, &press_q24_8) != 0) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
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
        case 0x0600: /* Read RTC Date and Time -> payload[0..6] = YY MM DD WD hh mm ss */
        {
            uint8_t year, month, day, weekday;
            uint8_t hours, minutes, seconds;

            rtc_read_date(&year, &month, &day, &weekday);
            rtc_read_time(&hours, &minutes, &seconds);

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

            if (month < 1U || month > 12U ||
                day   < 1U || day   > 31U ||
                weekday < 1U || weekday > 7U ||
                hours > 23U || minutes > 59U || seconds > 59U) {
                handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
                break;
            }

            rtc_write_date(year, month, day, weekday);
            rtc_write_time(hours, minutes, seconds);

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
        case 0x0606: /* TAMPER1 enable */
        {
            rtc_tamper1_enable();
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }
        case 0x0607: /* GET tamper status (1 byte) */
        {
            uint8_t t = rtc_tamper1_get_and_clear();
            handle_response(STATUS_OK, cmd, param_addr, &t, 1, use_uart1);
            break;
        }
        case 0x0900: /* SET TIME PCF8563T*/
        {
            uint8_t sec   = req[4];
            uint8_t min   = req[5];
            uint8_t hour  = req[6];
            uint8_t day   = req[7];
            uint8_t weekday = req[8];
            uint8_t month = req[9];
            uint8_t year  = req[10];
            pcf8563t_set_datetime(sec, min, hour, day, weekday, month, year);

            uint8_t data[7] = {sec, min, hour, day, weekday, month, year};
            handle_response(STATUS_OK, cmd, param_addr, data, 7, use_uart1);
            break;
        }
        case 0x0901: /* READ TIME PCF8563T */
        {
            uint8_t data[8];
            pcf8563t_get_datetime(&data[0], &data[1], &data[2], &data[3],
                                &data[4], &data[5], &data[6]);

            data[7] = pcf8563t_get_vl_flag();
            handle_response(STATUS_OK, cmd, param_addr, data, 8, use_uart1);
            break;
        }
        case 0x0902: /* SET CLOCK 1Hz PCF8563T*/
        {
            pcf8563t_clkout_set(1, PCF8563_CLKOUT_1HZ);
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }
        case 0x0903: /* DISABLE CLOCK PCF8563T*/
        {
            pcf8563t_clkout_set(0, PCF8563_CLKOUT_1HZ);
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }
        case 0x0904: /* SET ALARM PCF8563T (min,hour,day,wday) */
        {
            uint8_t minute  = req[4];
            uint8_t hour    = req[5];
            uint8_t day     = req[6];
            uint8_t weekday = req[7];

            pcf8563t_alarm_set(minute, hour, day, weekday);

            uint8_t data[4] = { minute, hour, day, weekday };
            handle_response(STATUS_OK, cmd, param_addr, data, 4, use_uart1);
            break;
        }

        case 0x0905: /* ENABLE ALARM PCF8563T */
        {
            pcf8563t_alarm_enable(1);
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }

        case 0x0906: /* DISABLE ALARM PCF8563T */
        {
            pcf8563t_alarm_enable(0);
            handle_response(STATUS_OK, cmd, param_addr, NULL, 0, use_uart1);
            break;
        }
        case 0x0907: /* READ+CLEAR ALARM FLAG (AF) PCF8563T */
        {
            uint8_t fired = pcf8563t_alarm_fired() ? 1u : 0u;
            if (fired) {
                pcf8563t_alarm_clear_flag();
            }
            handle_response(STATUS_OK, cmd, param_addr, &fired, 1, use_uart1);
            break;
        }
        default:
            handle_response(ERROR_RESPONSE, cmd, param_addr, NULL, 0, use_uart1);
            break;
    }
}

static void uart2_process_rx(void)
{
    uint16_t pos = (uint16_t)(UART2_RX_BUFFER_SIZE - DMA1_Channel6->CNDTR);

    while (uart2_rx_old_pos != pos) {
        uint8_t b = uart2_rx_buf[uart2_rx_old_pos];
        uart2_rx_old_pos++;
        if (uart2_rx_old_pos >= UART2_RX_BUFFER_SIZE) uart2_rx_old_pos = 0;

        if (uart2_frame_idx == 0 && b != DEV_ADDR) {
            continue;
        }

        uart2_frame_acc[uart2_frame_idx++] = b;
        if (uart2_frame_idx == UART2_RX_FRAME_LEN) {
            if (crc8_atm(uart2_frame_acc, 15) == uart2_frame_acc[15]) {
                handle_request(uart2_frame_acc, 0);
            }
            uart2_frame_idx = 0;
        }
    }
}

static void uart1_process_rx(void)
{
    uint16_t pos = (uint16_t)(UART1_RX_BUFFER_SIZE - DMA1_Channel5->CNDTR);

    while (uart1_rx_old_pos != pos) {
        uint8_t b = uart1_rx_buf[uart1_rx_old_pos];
        uart1_rx_old_pos++;
        if (uart1_rx_old_pos >= UART1_RX_BUFFER_SIZE) uart1_rx_old_pos = 0;

        if (uart1_frame_idx == 0 && b != DEV_ADDR) {
            continue;
        }

        uart1_frame_acc[uart1_frame_idx++] = b;
        if (uart1_frame_idx == UART1_RX_FRAME_LEN) {
            if (crc8_atm(uart1_frame_acc, 15) == uart1_frame_acc[15]) {
                handle_request(uart1_frame_acc, 1);
            }
            uart1_frame_idx = 0;
        }
    }
}

int main(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    (void)RCC->AHB2ENR;

    GPIOA->MODER &= ~(3U << (5U * 2U));
    GPIOA->MODER |=  (1U << (5U * 2U));

    GPIOA->OTYPER &= ~(1U << 5U);
    GPIOA->PUPDR &= ~(3U << (5U * 2U));

    uart2_rxtx_init();
    dma1_uart2_rx_config((uint32_t)uart2_rx_buf, UART2_RX_BUFFER_SIZE);
    dma1_uart2_tx_init();

    uart1_rxtx_init();
    dma1_uart1_rx_config((uint32_t)uart1_rx_buf, UART1_RX_BUFFER_SIZE);
    dma1_uart1_tx_init();

    adc_dma_init(2, adc_data_buffer, ADC_BUFFER_SIZE);
    tim1_init(4000, 1000);

    spi1_init();
    dma1_spi_rx_init();
    dma1_spi_tx_init();

    i2c1_init();
    dma_i2c1_rx_init();
    dma_i2c1_tx_init();

    bme280_init();

    rtc_init();
    gpio_irq_init();
    // lcd_init();
    // lcd_backlight(1);

    tim2_pa0_pa1_pwm_init(4, 1000);

    while (1)
    {
        if (dma2_adc_transfer_complete) {
            dma2_adc_transfer_complete = 0;
            last_adc_value = adc_data_buffer[0];
        }

        if (uart2_rx_pending) {
            uart2_rx_pending = 0;
            uart2_process_rx();
        }

        if (uart1_rx_pending) {
            uart1_rx_pending = 0;
            uart1_process_rx();
        }

        if (spi1_dma_rx_done && spi1_dma_tx_done) {
            spi1_dma_rx_done = 0;
            spi1_dma_tx_done = 0;
        }
        if (button_pressed)
        {
            button_handler();
            button_pressed = 0;
        }
    }
}

void DMA2_Channel3_IRQHandler(void)
{
    if (DMA2->ISR & DMA_ISR_TCIF3) {
        dma2_adc_transfer_complete = 1;
        adc_seq++;
        DMA2->IFCR = DMA_IFCR_CTCIF3;
    }
    if (DMA2->ISR & DMA_ISR_TEIF3) {
        DMA2->IFCR = DMA_IFCR_CTEIF3;
    }
}

void DMA1_Channel6_IRQHandler(void)
{
    if (DMA1->ISR & DMA_ISR_TEIF6) {
        DMA1->IFCR = DMA_IFCR_CTEIF6;
    }
}

void DMA1_Channel7_IRQHandler(void)
{
    if (DMA1->ISR & DMA_ISR_TCIF7) {
        DMA1->IFCR = DMA_IFCR_CTCIF7;
        uart2_tx_busy = 0;
    }
    if (DMA1->ISR & DMA_ISR_TEIF7) {
        DMA1->IFCR = DMA_IFCR_CTEIF7;
        uart2_tx_busy = 0;
    }
}

void USART2_IRQHandler(void)
{
    if (USART2->ISR & USART_ISR_IDLE) {
        USART2->ICR = USART_ICR_IDLECF;
        uart2_rx_pending = 1;
    }
}

void DMA1_Channel5_IRQHandler(void)
{
    if (DMA1->ISR & DMA_ISR_TEIF5) {
        DMA1->IFCR = DMA_IFCR_CTEIF5;
    }
}

void DMA1_Channel4_IRQHandler(void)
{
    if (DMA1->ISR & DMA_ISR_TCIF4) {
        DMA1->IFCR = DMA_IFCR_CTCIF4;
        uart1_tx_busy = 0;
    }
    if (DMA1->ISR & DMA_ISR_TEIF4) {
        DMA1->IFCR = DMA_IFCR_CTEIF4;
        uart1_tx_busy = 0;
    }
}

void USART1_IRQHandler(void)
{
    if (USART1->ISR & USART_ISR_IDLE) {
        USART1->ICR = USART_ICR_IDLECF;
        uart1_rx_pending = 1;
    }
}

void DMA1_Channel2_IRQHandler(void)
{
    if (DMA1->ISR & DMA_ISR_TCIF2) {
        spi1_dma_rx_done = 1;
        DMA1->IFCR = DMA_IFCR_CTCIF2;
    }
    if (DMA1->ISR & DMA_ISR_TEIF2) {
        DMA1->IFCR = DMA_IFCR_CTEIF2;
    }
}

void DMA1_Channel3_IRQHandler(void)
{
    if (DMA1->ISR & DMA_ISR_TCIF3) {
        spi1_dma_tx_done = 1;
        DMA1->IFCR = DMA_IFCR_CTCIF3;
    }
    if (DMA1->ISR & DMA_ISR_TEIF3) {
        DMA1->IFCR = DMA_IFCR_CTEIF3;
    }
}

void DMA2_Channel6_IRQHandler(void)
{
    if (DMA2->ISR & DMA_ISR_TCIF6) {
        DMA2->IFCR = DMA_IFCR_CTCIF6;
        i2c1_dma_tx_done = 1;
    }
    if (DMA2->ISR & DMA_ISR_TEIF6) {
        DMA2->IFCR = DMA_IFCR_CTEIF6;
        i2c1_dma_err = 1;
        i2c1_dma_tx_done = 1;
    }
}

void DMA2_Channel7_IRQHandler(void)
{
    if (DMA2->ISR & DMA_ISR_TCIF7) {
        DMA2->IFCR = DMA_IFCR_CTCIF7;
        i2c1_dma_rx_done = 1;
    }
    if (DMA2->ISR & DMA_ISR_TEIF7) {
        DMA2->IFCR = DMA_IFCR_CTEIF7;
        i2c1_dma_err = 1;
        i2c1_dma_rx_done = 1;
    }
}

void button_handler(void)
{
    lcd_set_cursor(0, 0);
    char buf[4];

    buf[0] = (irq_counter / 100) ? (irq_counter / 100) + '0' : ' ';
    buf[1] = (irq_counter / 10 % 10) + '0';
    buf[2] = (irq_counter % 10) + '0';
    buf[3] = '\0';
    lcd_send_string(buf[0] == ' ' ? &buf[1] : buf);
}

void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR1 & EXTI_PR1_PIF13)
    {
        EXTI->PR1 |= EXTI_PR1_PIF13;
        button_pressed = 1;
        irq_counter++;
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