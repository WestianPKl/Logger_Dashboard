#include "fm24cl16b.h"
#include "i2c.h"

int fm24cl16b_write(uint16_t mem_addr, const uint8_t *src, uint16_t len)
{
    if ((src == NULL) || (len == 0U)) return -1;
    if ((uint32_t)mem_addr + len > FM24CL16B_SIZE_BYTES) return -1;

    while (len > 0U)
    {
        uint16_t chunk = 256U - (uint16_t)(mem_addr & 0x00FFU);
        uint8_t dev = FM24CL16B_DEV_ADDR(mem_addr);
        uint8_t reg = FM24CL16B_WORD_ADDR(mem_addr);

        if (chunk > len) chunk = len;

        i2cOwnerTask = xTaskGetCurrentTaskHandle();
        i2cError = 0U;
        xTaskNotifyStateClear(NULL);
        (void)ulTaskNotifyTake(pdTRUE, 0);

        if (HAL_I2C_Mem_Write_IT(&hi2c1,
                                 (uint16_t)dev,
                                 reg,
                                 I2C_MEMADD_SIZE_8BIT,
                                 (uint8_t *)src,
                                 chunk) != HAL_OK)
        {
            i2cOwnerTask = NULL;
            return -1;
        }

        if (wait_i2c_done(pdMS_TO_TICKS(100U)) != 1)
        {
            i2cOwnerTask = NULL;
            i2c_restart();
            return -1;
        }

        mem_addr = (uint16_t)(mem_addr + chunk);
        src += chunk;
        len = (uint16_t)(len - chunk);
    }

    i2cOwnerTask = NULL;
    return 1;
}

int fm24cl16b_read(uint16_t mem_addr, uint8_t *dst, uint16_t len)
{
    if ((dst == NULL) || (len == 0U)) return -1;
    if ((uint32_t)mem_addr + len > FM24CL16B_SIZE_BYTES) return -1;

    while (len > 0U)
    {
        uint16_t chunk = 256U - (uint16_t)(mem_addr & 0x00FFU);
        uint8_t dev = FM24CL16B_DEV_ADDR(mem_addr);
        uint8_t reg = FM24CL16B_WORD_ADDR(mem_addr);

        if (chunk > len) chunk = len;

        i2cOwnerTask = xTaskGetCurrentTaskHandle();
        i2cError = 0U;
        xTaskNotifyStateClear(NULL);
        (void)ulTaskNotifyTake(pdTRUE, 0);

        if (HAL_I2C_Mem_Read_IT(&hi2c1,
                                (uint16_t)dev,
                                reg,
                                I2C_MEMADD_SIZE_8BIT,
                                dst,
                                chunk) != HAL_OK)
        {
            i2cOwnerTask = NULL;
            return -1;
        }

        if (wait_i2c_done(pdMS_TO_TICKS(100U)) != 1)
        {
            i2cOwnerTask = NULL;
            i2c_restart();
            return -1;
        }

        mem_addr = (uint16_t)(mem_addr + chunk);
        dst += chunk;
        len = (uint16_t)(len - chunk);
    }

    i2cOwnerTask = NULL;
    return 1;
}

int fm24cl16b_write_byte(uint16_t mem_addr, uint8_t value)
{
    return fm24cl16b_write(mem_addr, &value, 1);
}

int fm24cl16b_read_byte(uint16_t mem_addr, uint8_t *value)
{
    return fm24cl16b_read(mem_addr, value, 1);
}

int fm24cl16b_write_u32(uint16_t mem_addr, uint32_t value)
{
    uint8_t buf[4];

    buf[0] = (uint8_t)(value >> 0);
    buf[1] = (uint8_t)(value >> 8);
    buf[2] = (uint8_t)(value >> 16);
    buf[3] = (uint8_t)(value >> 24);

    return fm24cl16b_write(mem_addr, buf, 4);
}

int fm24cl16b_read_u32(uint16_t mem_addr, uint32_t *value)
{
    uint8_t buf[4];

    if (!value) return -1;
    if (fm24cl16b_read(mem_addr, buf, 4) != 1) return -1;

    *value = ((uint32_t)buf[0] << 0)  |
             ((uint32_t)buf[1] << 8)  |
             ((uint32_t)buf[2] << 16) |
             ((uint32_t)buf[3] << 24);

    return 1;
}