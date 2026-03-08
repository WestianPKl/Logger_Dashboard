#ifndef PCF8563T_DMA_H
#define PCF8563T_DMA_H

#include <stdint.h>

typedef enum {
    PCF8563_CLKOUT_32768HZ = 0x00,
    PCF8563_CLKOUT_1024HZ  = 0x01,
    PCF8563_CLKOUT_32HZ    = 0x02,
    PCF8563_CLKOUT_1HZ     = 0x03
} pcf8563_clkout_freq_t;

void pcf8563t_init(void);
uint8_t pcf8563t_get_vl_flag(void);
void pcf8563t_set_datetime(uint8_t seconds, uint8_t minutes, uint8_t hours,
                           uint8_t day, uint8_t weekday,
                           uint8_t month, uint8_t year);
int pcf8563t_get_datetime(uint8_t *seconds, uint8_t *minutes, uint8_t *hours,
                          uint8_t *day, uint8_t *weekday,
                          uint8_t *month, uint8_t *year);
void pcf8563t_clkout_set(uint8_t enable, pcf8563_clkout_freq_t freq);
void pcf8563t_clkout_1hz_enable(void);
void pcf8563t_alarm_set(uint8_t minute, uint8_t hour, uint8_t day, uint8_t weekday);
void pcf8563t_alarm_enable(uint8_t enable);
uint8_t pcf8563t_alarm_fired(void);
void pcf8563t_alarm_clear_flag(void);

#endif // PCF8563T_DMA_H