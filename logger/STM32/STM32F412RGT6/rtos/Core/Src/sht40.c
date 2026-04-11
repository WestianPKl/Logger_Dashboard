#include "sht40.h"
#include "FreeRTOS.h"
#include "task.h"
#include "i2c.h"

#define SHT40_ADDR 0x44
#define SHT40_CMD_SINGLE_SHOT_HIGHREP 0xFD
#define SHT40_SERIAL_READ 0x89

#define SHT40_MEAS_TIME_MS  15U

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

    if (HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(SHT40_ADDR << 1U), &cmd, 1U, 20U) != HAL_OK) {
        return -1;
    }

    HAL_Delay(1U);

    if (HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(SHT40_ADDR << 1U), data, 6U, 20U) != HAL_OK) {
        return -1;
    }

    if ((data[2] != sht_crc8(data, 2U)) || (data[5] != sht_crc8(&data[3], 2U))) {
        return -1;
    }

    return ((uint32_t)data[0] << 24) |
           ((uint32_t)data[1] << 16) |
           ((uint32_t)data[3] << 8)  |
           ((uint32_t)data[4]);
}

static int8_t sht40_read_raw(uint16_t *rawT, uint16_t *rawRH)
{
    const uint8_t cmd = SHT40_CMD_SINGLE_SHOT_HIGHREP;
    uint8_t data[6];

    if ((rawT == NULL) || (rawRH == NULL)) return -1;

    if (HAL_I2C_Master_Transmit(&hi2c1,
                                (uint16_t)(SHT40_ADDR << 1U),
                                (uint8_t *)&cmd,
                                1U,
                                20U) != HAL_OK) {
        return -1;
    }
    
    vTaskDelay(pdMS_TO_TICKS(SHT40_MEAS_TIME_MS));

    if (HAL_I2C_Master_Receive(&hi2c1,
                               (uint16_t)(SHT40_ADDR << 1U),
                               data,
                               6U,
                               20U) != HAL_OK) {
        return -1;
    }

    if ((data[2] != sht_crc8(data, 2U)) ||
        (data[5] != sht_crc8(&data[3], 2U))) {
        return -1;
    }

    *rawT  = ((uint16_t)data[0] << 8) | data[1];
    *rawRH = ((uint16_t)data[3] << 8) | data[4];

    return 1;
}

int8_t sht40_data_read_int(int16_t *temp_c, uint16_t *rh)
{
    uint16_t rawT, rawRH;
    int8_t e;
    int32_t tx100, hx100;

    if ((temp_c == NULL) || (rh == NULL)) return -1;

    e = sht40_read_raw(&rawT, &rawRH);
    if (e != 1) return e;

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
    return 1;
}

static int8_t sht40_read_raw_it(uint16_t *rawT, uint16_t *rawRH)
{
    static uint8_t data[6];
    const uint8_t cmd = SHT40_CMD_SINGLE_SHOT_HIGHREP;
    int8_t e;

    if ((rawT == NULL) || (rawRH == NULL)) return -1;

    i2cOwnerTask = xTaskGetCurrentTaskHandle();
    i2cError = 0U;
    xTaskNotifyStateClear(NULL);
    (void)ulTaskNotifyTake(pdTRUE, 0);

    if (HAL_I2C_Master_Transmit_IT(&hi2c1,
                                   (uint16_t)(SHT40_ADDR << 1U),
                                   (uint8_t *)&cmd,
                                   1U) != HAL_OK) {
        i2cOwnerTask = NULL;
        return -1;
    }

    e = wait_i2c_done(pdMS_TO_TICKS(50));
    if (e != 1) {
        i2cOwnerTask = NULL;
        i2c_restart();
        return -1;
    }

    vTaskDelay(pdMS_TO_TICKS(SHT40_MEAS_TIME_MS));

    xTaskNotifyStateClear(NULL);
    (void)ulTaskNotifyTake(pdTRUE, 0);
    i2cError = 0U;

    if (HAL_I2C_Master_Receive_IT(&hi2c1,
                                  (uint16_t)(SHT40_ADDR << 1U),
                                  data,
                                  6U) != HAL_OK) {
        i2cOwnerTask = NULL;
        return -1;
    }

    e = wait_i2c_done(pdMS_TO_TICKS(50));
    if (e != 1) {
        i2cOwnerTask = NULL;
        i2c_restart();
        return -1;
    }

    i2cOwnerTask = NULL;

    if ((data[2] != sht_crc8(data, 2U)) ||
        (data[5] != sht_crc8(&data[3], 2U))) {
        return -1;
    }

    *rawT  = ((uint16_t)data[0] << 8) | data[1];
    *rawRH = ((uint16_t)data[3] << 8) | data[4];

    return 1;
}

int8_t sht40_data_read_int_it(int16_t *temp_c, uint16_t *rh)
{
    uint16_t rawT, rawRH;
    int8_t e;
    int32_t tx100, hx100;

    if ((temp_c == NULL) || (rh == NULL)) return -1;

    e = sht40_read_raw_it(&rawT, &rawRH);
    if (e != 1) return e;

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
    return 1;
}