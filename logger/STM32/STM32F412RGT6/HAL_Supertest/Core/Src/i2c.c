#include "i2c.h"
#include "stm32f4xx_hal.h"
#include <string.h>

#define I2C_TIMEOUT      100U
#define I2C_MEMADD_SIZE  I2C_MEMADD_SIZE_8BIT

int i2c1_write_raw(uint8_t dev_addr, const uint8_t *data, uint16_t len)
{
    if ((data == NULL) || (len == 0U)) {
        return -1;
    }

    if (HAL_I2C_Master_Transmit(&hi2c1,
                                (uint16_t)(dev_addr << 1),
                                (uint8_t *)data,
                                len,
                                I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 1;
}

int i2c1_read_raw(uint8_t dev_addr, uint8_t *data, uint16_t len)
{
    if ((data == NULL) || (len == 0U)) {
        return -1;
    }

    if (HAL_I2C_Master_Receive(&hi2c1,
                               (uint16_t)(dev_addr << 1),
                               data,
                               len,
                               I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 1;
}

int i2c1_reg_write(uint8_t addr7, uint8_t reg, const uint8_t *data, uint16_t len)
{
    if ((data == NULL) || (len == 0U)) {
        return -1;
    }

    if (HAL_I2C_Mem_Write(&hi2c1,
                          (uint16_t)(addr7 << 1),
                          reg,
                          I2C_MEMADD_SIZE,
                          (uint8_t *)data,
                          len,
                          I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }

    return 1;
}

int i2c1_reg_read(uint8_t addr7, uint8_t reg, uint8_t *data, uint16_t len)
{
    if ((data == NULL) || (len == 0U)) {
        return -1;
    }

    if (HAL_I2C_Mem_Read(&hi2c1,
                         (uint16_t)(addr7 << 1),
                         reg,
                         I2C_MEMADD_SIZE,
                         data,
                         len,
                         I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }

    return 1;
}


int i2c1_write_raw_dma(uint8_t dev_addr, const uint8_t *data, uint16_t len)
{
    return i2c1_write_raw(dev_addr, data, len);
}

int i2c1_read_raw_dma(uint8_t dev_addr, uint8_t *data, uint16_t len)
{
    return i2c1_read_raw(dev_addr, data, len);
}

int i2c1_write_u8_u16_dma(uint8_t addr7, uint8_t reg, uint16_t value)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)((value >> 8) & 0xFF);
    buf[1] = (uint8_t)(value & 0xFF);
    return i2c1_reg_write(addr7, reg, buf, sizeof(buf));
}

int i2c1_read_u8_u16_dma(uint8_t addr7, uint8_t reg, uint16_t *value)
{
    uint8_t buf[2];

    if (value == NULL) return -1;
    if (i2c1_reg_read(addr7, reg, buf, sizeof(buf)) != 1) return -1;

    *value = ((uint16_t)buf[0] << 8) | buf[1];
    return 1;
}