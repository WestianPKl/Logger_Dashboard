#include "eeprom_m24c32.h"
#include "pico/stdlib.h"

#ifndef EEPROM_PAGE_SIZE
#define EEPROM_PAGE_SIZE 32
#endif

static bool ack_poll(i2c_inst_t *i2c, uint8_t addr, uint32_t timeout_ms) {
    absolute_time_t until = make_timeout_time_ms(timeout_ms);
    while (!time_reached(until)) {
        int rc = i2c_write_blocking(i2c, addr, NULL, 0, false);
        if (rc >= 0) return true;
        sleep_ms(2);
    }
    return false;
}

bool eeprom_m24c32_read(i2c_inst_t *i2c, uint8_t addr, uint16_t mem, uint8_t *buf, size_t len) {
    uint8_t a[2] = { (uint8_t)(mem >> 8), (uint8_t)(mem & 0xFF) };
    if (i2c_write_blocking(i2c, addr, a, 2, true) < 0) return false;
    if (i2c_read_blocking(i2c, addr, buf, len, false) < 0) return false;
    return true;
}

bool eeprom_m24c32_write(i2c_inst_t *i2c, uint8_t addr, uint16_t mem, const uint8_t *buf, size_t len) {
    while (len) {
        uint16_t page_off = mem % EEPROM_PAGE_SIZE;
        uint16_t chunk = EEPROM_PAGE_SIZE - page_off;
        if (chunk > len) chunk = (uint16_t)len;

        uint8_t tmp[2 + EEPROM_PAGE_SIZE];
        tmp[0] = (uint8_t)(mem >> 8);
        tmp[1] = (uint8_t)(mem & 0xFF);
        for (uint16_t i = 0; i < chunk; i++) tmp[2 + i] = buf[i];

        if (i2c_write_blocking(i2c, addr, tmp, 2 + chunk, false) < 0) return false;
        if (!ack_poll(i2c, addr, 50)) return false;

        mem += chunk;
        buf += chunk;
        len -= chunk;
    }
    return true;
}