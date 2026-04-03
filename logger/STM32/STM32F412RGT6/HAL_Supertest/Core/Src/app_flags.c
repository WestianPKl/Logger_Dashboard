#include "app_flags.h"

volatile uint8_t ext_rtc_present = 0U;
volatile uint8_t flash_present = 0U;
volatile uint8_t display_present = 1U;
volatile uint8_t sht40_present = 1U;
volatile uint8_t bme280_present = 1U;
volatile uint8_t ina226_present = 0U;
volatile uint8_t adc_present = 1U;
volatile uint8_t can_present = 1U;
volatile uint8_t adc_was_present = 0U;

volatile uint8_t fram_flags_toggle_flag = 0U;

volatile uint8_t rtc_wakeup_flag = 0U;
volatile uint8_t rtc_alarm_flag = 0U;
volatile uint8_t rtc_tampstamp_flag = 0U;

volatile uint32_t tick_1ms = 0U;
volatile uint8_t flag_100ms = 0U;
volatile uint8_t flag_1s = 0U;

volatile uint8_t btn1_flag = 0U;
volatile uint8_t btn2_flag = 0U;

volatile uint32_t btn1_last_tick = 0U;
volatile uint32_t btn2_last_tick = 0U;

volatile uint8_t led1_state = 0U;
volatile uint8_t led2_state = 0U;
volatile uint8_t pb12_state = 0U;
volatile uint8_t pc0_state = 0U;
volatile uint8_t pc1_state = 0U;
volatile uint8_t pc2_state = 0U;
volatile uint8_t pc3_state = 0U;
volatile uint8_t esp32_state = 0U;

volatile uint8_t rgb_r = 0U;
volatile uint8_t rgb_g = 0U;
volatile uint8_t rgb_b = 0U;
volatile uint8_t rgb_brightness = 0U;

volatile uint8_t backlight_on = 1U;
volatile uint8_t backlight_toggle_flag = 0U;
volatile uint8_t backlight_timer = 0U;

volatile uint8_t bme280_error_flag = 1U;
volatile uint8_t sht40_error_flag = 1U;

measurement_sht40_t measurement_sht40 = {0};
measurement_bme280_t measurement_bme280 = {0};
datetime_t datetime = {0};

volatile uint8_t uart1_tx_busy = 0U;
volatile uint8_t uart2_tx_busy = 0U;