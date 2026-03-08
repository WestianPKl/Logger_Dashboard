#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "hardware/i2c.h"

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} datetime_t;

bool rtc_pcf8563_get_datetime(i2c_inst_t *i2c, uint8_t addr, datetime_t *out);
bool rtc_pcf8563_set_datetime(i2c_inst_t *i2c, uint8_t addr, const datetime_t *dt);