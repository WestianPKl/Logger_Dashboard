#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include "main.h"

void dma_i2c1_rx(uint32_t dst, uint16_t len);
void dma_i2c1_tx(uint32_t src, uint16_t len);
void dma_i2c1_abort(void);

int i2c1_write_raw(uint8_t dev_addr, const uint8_t *data, uint8_t len);
int i2c1_read_raw(uint8_t dev_addr, uint8_t *data, uint8_t len);
int i2c1_reg_write(uint8_t addr7, uint8_t reg, const uint8_t *data, uint16_t len);
int i2c1_reg_read(uint8_t addr7, uint8_t reg, uint8_t *data, uint16_t len);

int i2c1_write_raw_dma(uint8_t dev_addr, const uint8_t *data, uint16_t len);
int i2c1_read_raw_dma(uint8_t dev_addr, uint8_t *data, uint16_t len);

int i2c1_write_u8_u16_dma(uint8_t addr7, uint8_t reg, uint16_t value);
int i2c1_read_u8_u16_dma(uint8_t addr7, uint8_t reg, uint16_t *value);

#endif // I2C_H