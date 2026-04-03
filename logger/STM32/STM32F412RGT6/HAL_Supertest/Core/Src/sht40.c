#include "sht40.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include "app_flags.h"

#define SHT40_ADDR 0x44
#define SHT40_CMD_SINGLE_SHOT_HIGHREP 0xFD
#define SHT40_SERIAL_READ 0x89

#define SHT40_MEAS_TIME_MS  15U
#define SHT40_PERIOD_MS   1000U

typedef enum {
    SHT40_STATE_IDLE = 0,
    SHT40_STATE_WAIT_MEAS
} sht40_state_t;

static sht40_state_t sht40_state = SHT40_STATE_IDLE;
static uint8_t sht40_enabled = 1U;
static uint32_t sht40_period_start = 0U;
static uint32_t sht40_meas_start = 0U;

static uint8_t sht_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFFU;

    for (uint8_t j = 0U; j < len; j++) {
        crc ^= data[j];
        for (uint8_t i = 0U; i < 8U; i++) {
            crc = (crc & 0x80U) ? (uint8_t)((crc << 1) ^ 0x31U) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

uint32_t sht40_read_serial_number(void)
{
    uint8_t cmd = SHT40_SERIAL_READ;
    uint8_t data[6];

    if (i2c1_write_raw(SHT40_ADDR, &cmd, 1U) != 1) {
        return 0U;
    }

    HAL_Delay(1U);

    if (i2c1_read_raw(SHT40_ADDR, data, 6U) != 1) {
        return 0U;
    }

    if ((data[2] != sht_crc8(data, 2U)) || (data[5] != sht_crc8(&data[3], 2U))) {
        return 0U;
    }

    return ((uint32_t)data[0] << 24) |
           ((uint32_t)data[1] << 16) |
           ((uint32_t)data[3] << 8)  |
           ((uint32_t)data[4]);
}

static uint8_t sht40_read_raw(uint16_t *rawT, uint16_t *rawRH)
{
    uint8_t cmd = SHT40_CMD_SINGLE_SHOT_HIGHREP;
    uint8_t data[6];

    if ((rawT == NULL) || (rawRH == NULL)) return 1U;
    if (i2c1_write_raw(SHT40_ADDR, &cmd, 1U) != 1) return 2U;

    HAL_Delay(SHT40_MEAS_TIME_MS);

    if (i2c1_read_raw(SHT40_ADDR, data, 6U) != 1) return 3U;
    if ((data[2] != sht_crc8(data, 2U)) || (data[5] != sht_crc8(&data[3], 2U))) return 4U;

    *rawT  = ((uint16_t)data[0] << 8) | data[1];
    *rawRH = ((uint16_t)data[3] << 8) | data[4];
    return 0U;
}

uint8_t sht40_single_shot_measurement(uint8_t *data)
{
    uint8_t cmd = SHT40_CMD_SINGLE_SHOT_HIGHREP;

    if (data == NULL) return 1U;
    if (i2c1_write_raw(SHT40_ADDR, &cmd, 1U) != 1) return 1U;

    HAL_Delay(SHT40_MEAS_TIME_MS);

    if (i2c1_read_raw(SHT40_ADDR, data, 6U) != 1) return 2U;
    return 0U;
}

uint8_t sht40_read_data(float *temp_c, float *rh)
{
    uint16_t rawT, rawRH;
    uint8_t e;
    float r;

    if ((temp_c == NULL) || (rh == NULL)) return 1U;

    e = sht40_read_raw(&rawT, &rawRH);
    if (e != 0U) return e;

    *temp_c = -45.0f + 175.0f * ((float)rawT / 65535.0f);
    r = -6.0f + 125.0f * ((float)rawRH / 65535.0f);

    if (r < 0.0f)   r = 0.0f;
    if (r > 100.0f) r = 100.0f;

    *rh = r;
    return 0U;
}

uint8_t sht40_data_read_int(int16_t *temp_c, uint16_t *rh)
{
    uint16_t rawT, rawRH;
    uint8_t e;
    int32_t tx100, hx100;

    if ((temp_c == NULL) || (rh == NULL)) return 1U;

    e = sht40_read_raw(&rawT, &rawRH);
    if (e != 0U) return e;

    tx100 = -4500;
    tx100 += (int32_t)((17500UL * (uint32_t)rawT + 32767UL) / 65535UL);

    hx100 = -600;
    hx100 += (int32_t)((12500UL * (uint32_t)rawRH + 32767UL) / 65535UL);

    if (tx100 < -32768) tx100 = -32768;
    if (tx100 >  32767) tx100 =  32767;

    if (hx100 < 0)     hx100 = 0;
    if (hx100 > 10000) hx100 = 10000;

    *temp_c = (int16_t)tx100;
    *rh     = (uint16_t)hx100;
    return 0U;
}

void sht40_start_periodic(void)
{
    sht40_enabled = 1U;
    sht40_period_start = HAL_GetTick();
    sht40_state = SHT40_STATE_IDLE;
}

void sht40_stop_periodic(void)
{
    sht40_enabled = 0U;
    sht40_state = SHT40_STATE_IDLE;
}

uint8_t sht40_is_busy(void)
{
    return (sht40_state != SHT40_STATE_IDLE) ? 1U : 0U;
}

void sht40_task(void)
{
    static uint8_t rx[6];
    uint32_t now;

    if (!sht40_enabled) {
        sht40_state = SHT40_STATE_IDLE;
        return;
    }

    now = HAL_GetTick();

    switch (sht40_state) {
    case SHT40_STATE_IDLE:
    {
        uint8_t cmd;

        if ((uint32_t)(now - sht40_period_start) < SHT40_PERIOD_MS) {
            return;
        }

        cmd = SHT40_CMD_SINGLE_SHOT_HIGHREP;
        if (i2c1_write_raw(SHT40_ADDR, &cmd, 1U) == 1) {
            sht40_meas_start = now;
            sht40_state = SHT40_STATE_WAIT_MEAS;
        } else {
            sht40_error_flag = 1U;
            sht40_period_start = now;
        }
        break;
    }

    case SHT40_STATE_WAIT_MEAS:
    {
        if ((uint32_t)(now - sht40_meas_start) < SHT40_MEAS_TIME_MS) {
            return;
        }

        if (i2c1_read_raw(SHT40_ADDR, rx, 6U) == 1) {
            if ((rx[2] == sht_crc8(rx, 2U)) && (rx[5] == sht_crc8(&rx[3], 2U))) {
                uint16_t rawT  = ((uint16_t)rx[0] << 8) | rx[1];
                uint16_t rawRH = ((uint16_t)rx[3] << 8) | rx[4];
                int32_t tx100 = -4500;
                int32_t hx100 = -600;

                tx100 += (int32_t)((17500UL * (uint32_t)rawT + 32767UL) / 65535UL);
                hx100 += (int32_t)((12500UL * (uint32_t)rawRH + 32767UL) / 65535UL);

                if (tx100 < -32768) tx100 = -32768;
                if (tx100 >  32767) tx100 =  32767;

                if (hx100 < 0)     hx100 = 0;
                if (hx100 > 10000) hx100 = 10000;

                measurement_sht40.temperature = (int16_t)tx100;
                measurement_sht40.humidity    = (uint16_t)hx100;
                sht40_error_flag = 0U;
            } else {
                sht40_error_flag = 1U;
            }
        } else {
            sht40_error_flag = 1U;
        }

        sht40_period_start = now;
        sht40_state = SHT40_STATE_IDLE;
        break;
    }

    default:
        sht40_state = SHT40_STATE_IDLE;
        sht40_period_start = now;
        break;
    }
}