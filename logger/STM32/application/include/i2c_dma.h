#ifndef I2C_DMA_H
#define I2C_DMA_H

#include <stdint.h>
#include "stm32l4xx.h"

void i2c1_init(void);

int i2c1_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint8_t len);
int i2c1_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);

int i2c1_write_cmd(uint8_t dev_addr, uint8_t cmd);
int i2c1_write_cmd16(uint8_t dev_addr, uint16_t cmd);

int i2c1_write_raw(uint8_t dev_addr, const uint8_t *data, uint8_t len);
int i2c1_read_raw(uint8_t dev_addr, uint8_t *data, uint8_t len);

#endif // I2C_DMA_H