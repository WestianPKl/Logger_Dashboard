#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "hardware/i2c.h"

bool eeprom_m24c32_read(i2c_inst_t *i2c, uint8_t addr, uint16_t mem, uint8_t *buf, size_t len);
bool eeprom_m24c32_write(i2c_inst_t *i2c, uint8_t addr, uint16_t mem, const uint8_t *buf, size_t len);