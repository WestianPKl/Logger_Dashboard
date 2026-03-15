#include "fm24cl16b.h"
#include "i2c.h"
#include <string.h>

int fm24cl16b_write(uint16_t mem_addr, const uint8_t *src, uint16_t len)
{
    if (!src || !len) return -1;
    if ((uint32_t)mem_addr + len > FM24CL16B_SIZE_BYTES) return -1;

    while (len) {
        uint8_t dev = FM24CL16B_DEV_ADDR(mem_addr);
        uint8_t reg = FM24CL16B_WORD_ADDR(mem_addr);

        uint16_t chunk = 256U - (uint16_t)(mem_addr & 0xFFU);
        if (chunk > len) chunk = len;

        if (i2c1_reg_write(dev, reg, src, chunk) != 1) return -1;

        mem_addr = (uint16_t)(mem_addr + chunk);
        src += chunk;
        len -= chunk;
    }

    return 1;
}

int fm24cl16b_read(uint16_t mem_addr, uint8_t *dst, uint16_t len)
{
    if (!dst || !len) return -1;
    if ((uint32_t)mem_addr + len > FM24CL16B_SIZE_BYTES) return -1;

    while (len) {
        uint8_t dev = FM24CL16B_DEV_ADDR(mem_addr);
        uint8_t reg = FM24CL16B_WORD_ADDR(mem_addr);

        uint16_t chunk = 256U - (uint16_t)(mem_addr & 0xFFU);
        if (chunk > len) chunk = len;

        if (i2c1_reg_read(dev, reg, dst, chunk) != 1) return -1;

        mem_addr = (uint16_t)(mem_addr + chunk);
        dst += chunk;
        len -= chunk;
    }

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
