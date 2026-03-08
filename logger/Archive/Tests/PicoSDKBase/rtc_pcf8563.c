#include "rtc_pcf8563.h"
#include <string.h>

#define REG_SECONDS 0x02

static uint8_t bcd2bin(uint8_t b) { return (b & 0x0F) + 10 * (b >> 4); }
static uint8_t bin2bcd(uint8_t v) { return ((v / 10) << 4) | (v % 10); }

static bool i2c_read_regs(i2c_inst_t *i2c, uint8_t addr, uint8_t reg, uint8_t *buf, size_t len) {
    if (i2c_write_blocking(i2c, addr, &reg, 1, true) < 0) return false;
    if (i2c_read_blocking(i2c, addr, buf, len, false) < 0) return false;
    return true;
}

static bool i2c_write_regs(i2c_inst_t *i2c, uint8_t addr, uint8_t reg, const uint8_t *buf, size_t len) {
    uint8_t tmp[1 + 16];
    if (len > 16) return false;
    tmp[0] = reg;
    memcpy(&tmp[1], buf, len);
    if (i2c_write_blocking(i2c, addr, tmp, 1 + len, false) < 0) return false;
    return true;
}

bool rtc_pcf8563_get_datetime(i2c_inst_t *i2c, uint8_t addr, datetime_t *out) {
    uint8_t r[7];
    if (!i2c_read_regs(i2c, addr, REG_SECONDS, r, sizeof(r))) return false;

    bool vl = (r[0] & 0x80) != 0;

    out->sec   = bcd2bin(r[0] & 0x7F);
    out->min   = bcd2bin(r[1] & 0x7F);
    out->hour  = bcd2bin(r[2] & 0x3F);
    out->day   = bcd2bin(r[3] & 0x3F);
    out->month = bcd2bin(r[5] & 0x1F);
    out->year  = (uint16_t)(2000 + bcd2bin(r[6]));

    return !vl;
}

bool rtc_pcf8563_set_datetime(i2c_inst_t *i2c, uint8_t addr, const datetime_t *dt) {
    uint8_t r[7];
    r[0] = bin2bcd(dt->sec)  & 0x7F;
    r[1] = bin2bcd(dt->min)  & 0x7F;
    r[2] = bin2bcd(dt->hour) & 0x3F;
    r[3] = bin2bcd(dt->day)  & 0x3F;
    r[4] = 0;
    r[5] = bin2bcd(dt->month) & 0x1F;
    r[6] = bin2bcd((uint8_t)(dt->year - 2000));

    return i2c_write_regs(i2c, addr, REG_SECONDS, r, sizeof(r));
}