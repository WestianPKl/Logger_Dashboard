
#ifndef __APP_FLAGS_H
#define __APP_FLAGS_H

#include "stdint.h"

typedef struct {
    int16_t temperature;
    uint16_t humidity;
} measurement_sht40_t;

typedef struct {
    int32_t temperature;
    uint32_t humidity;
    uint32_t pressure;
} measurement_bme280_t;

typedef struct {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} datetime_t;

extern volatile uint8_t ext_rtc_present;
extern volatile uint8_t flash_present;
extern volatile uint8_t display_present;
extern volatile uint8_t sht40_present;
extern volatile uint8_t bme280_present;
extern volatile uint8_t ina226_present;
extern volatile uint8_t adc_present;
extern volatile uint8_t can_present;
extern volatile uint8_t adc_was_present;

extern volatile uint8_t fram_flags_toggle_flag;

extern volatile uint8_t rtc_wakeup_flag;
extern volatile uint8_t rtc_alarm_flag;
extern volatile uint8_t rtc_tampstamp_flag;

extern volatile uint32_t tick_1ms;
extern volatile uint8_t flag_100ms;
extern volatile uint8_t flag_1s;

extern volatile uint8_t btn1_flag;
extern volatile uint8_t btn2_flag;

extern volatile uint32_t btn1_last_tick;
extern volatile uint32_t btn2_last_tick;

extern volatile uint8_t led1_state;
extern volatile uint8_t led2_state;
extern volatile uint8_t pb12_state;
extern volatile uint8_t pc0_state;
extern volatile uint8_t pc1_state;
extern volatile uint8_t pc2_state;
extern volatile uint8_t pc3_state;
extern volatile uint8_t esp32_state;

extern volatile uint8_t rgb_r;
extern volatile uint8_t rgb_g;
extern volatile uint8_t rgb_b;
extern volatile uint8_t rgb_brightness;

extern volatile uint8_t backlight_on;
extern volatile uint8_t backlight_toggle_flag;
extern volatile uint8_t backlight_timer;

extern volatile uint8_t bme280_error_flag;
extern volatile uint8_t sht40_error_flag;

extern measurement_sht40_t measurement_sht40;
extern measurement_bme280_t measurement_bme280;
extern datetime_t datetime;

extern volatile uint8_t uart2_tx_busy;
extern volatile uint8_t uart1_tx_busy;

extern volatile uint8_t spi1_dma_rx_done;
extern volatile uint8_t spi1_dma_tx_done;

extern volatile uint8_t i2c1_dma_tx_done;
extern volatile uint8_t i2c1_dma_rx_done;
extern volatile uint8_t i2c1_dma_err;

#endif /* __APP_FLAGS_H */
