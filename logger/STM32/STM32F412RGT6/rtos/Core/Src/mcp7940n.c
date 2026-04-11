#include "mcp7940n.h"
#include "i2c.h"

static uint8_t bin2bcd(uint8_t v)
{
    return (uint8_t)(((v / 10U) << 4) | (v % 10U));
}

static uint8_t bcd2bin(uint8_t v)
{
    return (uint8_t)(((v >> 4) * 10U) + (v & 0x0FU));
}

int mcp7940n_read_reg(uint8_t reg, uint8_t *value)
{
    if (value == NULL) return -1;

    i2cOwnerTask = xTaskGetCurrentTaskHandle();
    i2cError = 0U;
    xTaskNotifyStateClear(NULL);
    (void)ulTaskNotifyTake(pdTRUE, 0);

    if (HAL_I2C_Mem_Read_IT(&hi2c1,
                            (uint16_t)(MCP7940N_ADDR << 1),
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            value,
                            1) != HAL_OK)
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

    i2cOwnerTask = NULL;
    return 1;
}

int mcp7940n_write_reg(uint8_t reg, uint8_t value)
{
    static uint8_t val;
    val = value;
    i2cOwnerTask = xTaskGetCurrentTaskHandle();
    i2cError = 0U;
    xTaskNotifyStateClear(NULL);
    (void)ulTaskNotifyTake(pdTRUE, 0);

    if (HAL_I2C_Mem_Write_IT(&hi2c1,
                             (uint16_t)(MCP7940N_ADDR << 1),
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &val,
                             1) != HAL_OK)
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

    i2cOwnerTask = NULL;
    return 1;
}

int mcp7940n_read_regs(uint8_t reg, uint8_t *data, uint16_t len)
{
    if ((data == NULL) || (len == 0U)) return -1;

    i2cOwnerTask = xTaskGetCurrentTaskHandle();
    i2cError = 0U;
    xTaskNotifyStateClear(NULL);
    (void)ulTaskNotifyTake(pdTRUE, 0);

    if (HAL_I2C_Mem_Read_IT(&hi2c1,
                            (uint16_t)(MCP7940N_ADDR << 1),
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            data,
                            len) != HAL_OK)
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

    i2cOwnerTask = NULL;
    return 1;
}

int mcp7940n_write_regs(uint8_t reg, const uint8_t *data, uint16_t len)
{
    if ((data == NULL) || (len == 0U)) return -1;

    i2cOwnerTask = xTaskGetCurrentTaskHandle();
    i2cError = 0U;
    xTaskNotifyStateClear(NULL);
    (void)ulTaskNotifyTake(pdTRUE, 0);

    if (HAL_I2C_Mem_Write_IT(&hi2c1,
                             (uint16_t)(MCP7940N_ADDR << 1),
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             (uint8_t *)data,
                             len) != HAL_OK)
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

    i2cOwnerTask = NULL;
    return 1;
}

int mcp7940n_init(uint8_t enable_battery_backup)
{
    uint8_t sec;
    uint8_t wkday;

    if (mcp7940n_read_reg(MCP7940N_REG_RTCSEC, &sec) != 1) return -1;
    sec |= MCP7940N_BIT_ST;
    if (mcp7940n_write_reg(MCP7940N_REG_RTCSEC, sec) != 1) return -1;

    if (mcp7940n_read_reg(MCP7940N_REG_RTCWKDAY, &wkday) != 1) return -1;

    if (enable_battery_backup) wkday |= MCP7940N_BIT_VBATEN;
    else                       wkday &= (uint8_t)~MCP7940N_BIT_VBATEN;

    wkday &= (uint8_t)~MCP7940N_BIT_PWRFAIL;

    if (mcp7940n_write_reg(MCP7940N_REG_RTCWKDAY, wkday) != 1) return -1;

    return 1;
}

int mcp7940n_set_datetime(const mcp7940n_datetime_t *dt)
{
    if (!dt) return -1;
    if (dt->sec   > 59U) return -1;
    if (dt->min   > 59U) return -1;
    if (dt->hour  > 23U) return -1;
    if (dt->wday  < 1U || dt->wday  > 7U)  return -1;
    if (dt->mday  < 1U || dt->mday  > 31U) return -1;
    if (dt->month < 1U || dt->month > 12U) return -1;
    if (dt->year  > 99U ) return -1;

    uint8_t buf[7];
    uint8_t wkday_old = 0;

    if (mcp7940n_read_reg(MCP7940N_REG_RTCWKDAY, &wkday_old) != 1) return -1;

    buf[0] = (uint8_t)(bin2bcd(dt->sec)  | MCP7940N_BIT_ST);
    buf[1] = bin2bcd(dt->min);
    buf[2] = bin2bcd(dt->hour);
    buf[3] = (uint8_t)((wkday_old & (MCP7940N_BIT_VBATEN | MCP7940N_BIT_PWRFAIL)) |
                       (dt->wday & 0x07U));
    buf[4] = bin2bcd(dt->mday);
    buf[5] = bin2bcd(dt->month);
    buf[6] = bin2bcd(dt->year);

    return mcp7940n_write_regs(MCP7940N_REG_RTCSEC, buf, 7);
}

int mcp7940n_get_datetime(mcp7940n_datetime_t *dt)
{
    if (!dt) return -1;

    uint8_t buf[7];
    if (mcp7940n_read_regs(MCP7940N_REG_RTCSEC, buf, 7) != 1) return -1;

    dt->sec   = bcd2bin(buf[0] & 0x7FU);
    dt->min   = bcd2bin(buf[1] & 0x7FU);
    dt->hour  = bcd2bin(buf[2] & 0x3FU);
    dt->wday  = (uint8_t)(buf[3] & 0x07U);
    dt->mday  = bcd2bin(buf[4] & 0x3FU);
    dt->month = bcd2bin(buf[5] & 0x1FU);
    dt->year  = bcd2bin(buf[6]);

    return 1;
}

int mcp7940n_read_sram(uint8_t offset, uint8_t *data, uint8_t len)
{
    if (!data || !len) return -1;
    if ((uint16_t)offset + len > MCP7940N_SRAM_SIZE) return -1;

    return mcp7940n_read_regs((uint8_t)(MCP7940N_REG_SRAM_START + offset), data, len);
}

int mcp7940n_write_sram(uint8_t offset, const uint8_t *data, uint8_t len)
{
    if (!data || !len) return -1;
    if ((uint16_t)offset + len > MCP7940N_SRAM_SIZE) return -1;

    return mcp7940n_write_regs((uint8_t)(MCP7940N_REG_SRAM_START + offset), data, len);
}

int mcp7940n_mfp_alarm0_enable(void)
{
    uint8_t ctrl;

    if (mcp7940n_read_reg(MCP7940N_REG_CONTROL, &ctrl) != 1) return -1;

    ctrl &= ~MCP7940N_BIT_SQWEN;
    ctrl &= ~MCP7940N_BIT_ALM1EN;
    ctrl |=  MCP7940N_BIT_ALM0EN;

    return mcp7940n_write_reg(MCP7940N_REG_CONTROL, ctrl);
}

int mcp7940n_mfp_sqw_1hz(void)
{
    uint8_t ctrl;

    if (mcp7940n_read_reg(MCP7940N_REG_CONTROL, &ctrl) != 1) return -1;

    ctrl &= ~(MCP7940N_BIT_ALM0EN | MCP7940N_BIT_ALM1EN);
    ctrl |= MCP7940N_BIT_SQWEN;
    ctrl &= ~(MCP7940N_BIT_SQWFS1 | MCP7940N_BIT_SQWFS0);

    return mcp7940n_write_reg(MCP7940N_REG_CONTROL, ctrl);
}