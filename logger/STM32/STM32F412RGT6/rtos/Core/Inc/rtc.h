#ifndef RTC_H
#define RTC_H

#include <stdint.h>

typedef struct {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} rtc_date_time_t;


#include "main.h"


int8_t rtc_set_datetime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday,
                      uint8_t hours, uint8_t minutes, uint8_t seconds);
int8_t rtc_get_datetime(uint8_t *year, uint8_t *month, uint8_t *day, uint8_t *weekday,
                      uint8_t *hours, uint8_t *minutes, uint8_t *seconds);                      


#endif // RTC_H