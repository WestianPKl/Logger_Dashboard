#include "bme280.h"
#include "spi.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

#define REG_ID          0xD0
#define REG_RESET       0xE0
#define REG_STATUS      0xF3
#define REG_CTRL_HUM    0xF2
#define REG_CTRL_MEAS   0xF4
#define REG_CONFIG      0xF5
#define REG_PRESS_MSB   0xF7

#define REG_DIG_T1      0x88
#define REG_DIG_H1      0xA1
#define REG_DIG_H2      0xE1

#define BME280_CHIP_ID             0x60U
#define BME280_STATUS_IM_UPDATE    0x01U
#define BME280_STATUS_MEASURING    0x08U

#define OSRS_1X        0x01U
#define MODE_SLEEP     0x00U
#define MODE_FORCED    0x01U

#define BME_MEAS_CHECK_MS   2U
#define BME_MEAS_TIMEOUT_MS 30U
#define BME_RESET_WAIT_MS   100U
#define BME_XFER_MAX_DATA   32U

typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} bme280_calib_t;

static bme280_calib_t calib;
static int32_t t_fine = 0;

HAL_StatusTypeDef spi1_xfer_sync(const uint8_t *tx, uint8_t *rx, uint16_t len)
{
    if ((tx == NULL) || (rx == NULL) || (len == 0U)) {
        return HAL_ERROR;
    }

    spiOwnerTask = xTaskGetCurrentTaskHandle();
    spiError = 0U;
    xTaskNotifyStateClear(NULL);
    (void)ulTaskNotifyTake(pdTRUE, 0);

    if (HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)tx, rx, len) != HAL_OK) {
        spiOwnerTask = NULL;
        return HAL_ERROR;
    }

    if (wait_spi_done(pdMS_TO_TICKS(100U)) != 1) {
        (void)HAL_SPI_Abort(&hspi1);
        spiOwnerTask = NULL;
        return HAL_ERROR;
    }

    spiOwnerTask = NULL;
    return HAL_OK;
}

static int8_t bme280_read_registers(uint8_t reg, uint8_t *data, uint32_t size)
{
    uint32_t len;
    uint8_t txbuf[1U + BME_XFER_MAX_DATA];
    uint8_t rxbuf[1U + BME_XFER_MAX_DATA];

    if ((data == NULL) || (size == 0U) || (size > BME_XFER_MAX_DATA)) {
        return -1;
    }

    len = size + 1U;

    txbuf[0] = reg | 0x80U;
    memset(&txbuf[1], 0, size);

    spi1_cs1_low();
    if (spi1_xfer_sync(txbuf, rxbuf, (uint16_t)len) != HAL_OK) {
        spi1_cs1_high();
        return -1;
    }
    spi1_cs1_high();

    memcpy(data, &rxbuf[1], size);
    return 1;
}

static int8_t bme280_write_register(uint8_t reg, uint8_t value)
{
    uint8_t txbuf[2];
    uint8_t rxbuf[2];

    txbuf[0] = reg & 0x7FU;
    txbuf[1] = value;

    spi1_cs1_low();
    if (spi1_xfer_sync(txbuf, rxbuf, 2U) != HAL_OK) {
        spi1_cs1_high();
        return -1;
    }
    spi1_cs1_high();

    return 1;
}

static int16_t sign_extend_12(int16_t v)
{
    if ((v & 0x0800) != 0) {
        v |= (int16_t)0xF000;
    }
    return v;
}

static int8_t bme280_wait_nvm_copy_done(void)
{
    uint8_t status = 0U;
    TickType_t start = xTaskGetTickCount();

    do {
        if (bme280_read_registers(REG_STATUS, &status, 1U) != 1) {
            return -1;
        }

        if ((status & BME280_STATUS_IM_UPDATE) == 0U) {
            return 1;
        }

        vTaskDelay(pdMS_TO_TICKS(BME_MEAS_CHECK_MS));
    } while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(BME_RESET_WAIT_MS));

    return -1;
}

static int8_t bme280_wait_measurement_done(void)
{
    uint8_t status = 0U;
    TickType_t start = xTaskGetTickCount();

    do {
        if (bme280_read_registers(REG_STATUS, &status, 1U) != 1) {
            return -1;
        }

        if ((status & BME280_STATUS_MEASURING) == 0U) {
            return 1;
        }

        vTaskDelay(pdMS_TO_TICKS(BME_MEAS_CHECK_MS));
    } while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(BME_MEAS_TIMEOUT_MS));

    return -1;
}

static int8_t bme280_read_calibration_data(void)
{
    uint8_t data[26];
    int16_t h4, h5;

    memset(&calib, 0, sizeof(calib));

    if (bme280_read_registers(REG_DIG_T1, data, 24U) != 1) {
        return -1;
    }

    calib.dig_T1 = (uint16_t)(data[0]  | (data[1]  << 8));
    calib.dig_T2 = (int16_t)(data[2]   | (data[3]  << 8));
    calib.dig_T3 = (int16_t)(data[4]   | (data[5]  << 8));

    calib.dig_P1 = (uint16_t)(data[6]  | (data[7]  << 8));
    calib.dig_P2 = (int16_t)(data[8]   | (data[9]  << 8));
    calib.dig_P3 = (int16_t)(data[10]  | (data[11] << 8));
    calib.dig_P4 = (int16_t)(data[12]  | (data[13] << 8));
    calib.dig_P5 = (int16_t)(data[14]  | (data[15] << 8));
    calib.dig_P6 = (int16_t)(data[16]  | (data[17] << 8));
    calib.dig_P7 = (int16_t)(data[18]  | (data[19] << 8));
    calib.dig_P8 = (int16_t)(data[20]  | (data[21] << 8));
    calib.dig_P9 = (int16_t)(data[22]  | (data[23] << 8));

    if (bme280_read_registers(REG_DIG_H1, &data[0], 1U) != 1) {
        return -1;
    }
    calib.dig_H1 = data[0];

    if (bme280_read_registers(REG_DIG_H2, &data[0], 7U) != 1) {
        return -1;
    }

    calib.dig_H2 = (int16_t)(data[0] | (data[1] << 8));
    calib.dig_H3 = data[2];

    h4 = (int16_t)((((int16_t)data[3]) << 4) | (data[4] & 0x0F));
    h5 = (int16_t)((((int16_t)data[5]) << 4) | (data[4] >> 4));

    calib.dig_H4 = sign_extend_12(h4);
    calib.dig_H5 = sign_extend_12(h5);
    calib.dig_H6 = (int8_t)data[6];

    return 1;
}

int8_t bme280_read_id(void)
{
    uint8_t id = 0U;

    if (bme280_read_registers(REG_ID, &id, 1U) != 1) {
        return -1;
    }

    return id;
}

int8_t bme280_init(void)
{
    if (bme280_write_register(REG_RESET, 0xB6U) != 1) {
        return -1;
    }

    vTaskDelay(pdMS_TO_TICKS(BME_RESET_WAIT_MS));

    if (bme280_wait_nvm_copy_done() != 1) {
        return -1;
    }

    if (bme280_read_id() != BME280_CHIP_ID) {
        return -1;
    }

    if (bme280_read_calibration_data() != 1) {
        return -1;
    }

    if (bme280_write_register(REG_CTRL_HUM, OSRS_1X) != 1) {
        return -1;
    }

    if (bme280_write_register(REG_CONFIG, (uint8_t)((0x05U << 5) | (0x00U << 2) | 0x00U)) != 1) {
        return -1;
    }

    if (bme280_write_register(REG_CTRL_MEAS,
                              (uint8_t)((OSRS_1X << 5) | (OSRS_1X << 2) | MODE_SLEEP)) != 1) {
        return -1;
    }

    return 1;
}

int8_t bme280_trigger_forced(void)
{
    if (bme280_write_register(REG_CTRL_HUM, OSRS_1X) != 1) {
        return -1;
    }

    if (bme280_write_register(REG_CTRL_MEAS,
                              (uint8_t)((OSRS_1X << 5) | (OSRS_1X << 2) | MODE_FORCED)) != 1) {
        return -1;
    }

    return 1;
}

static int8_t bme280_read_raw(int32_t *adc_P, int32_t *adc_T, int32_t *adc_H)
{
    uint8_t data[8];

    if ((adc_P == NULL) || (adc_T == NULL) || (adc_H == NULL)) {
        return -1;
    }

    if (bme280_read_registers(REG_PRESS_MSB, data, 8U) != 1) {
        return -1;
    }

    *adc_P = (int32_t)((((uint32_t)data[0]) << 12) |
                       (((uint32_t)data[1]) << 4)  |
                       (((uint32_t)data[2]) >> 4));

    *adc_T = (int32_t)((((uint32_t)data[3]) << 12) |
                       (((uint32_t)data[4]) << 4)  |
                       (((uint32_t)data[5]) >> 4));

    *adc_H = (int32_t)((((uint32_t)data[6]) << 8) |
                       ((uint32_t)data[7]));

    return 1;
}

static int32_t bme280_compensate_T_x100(int32_t adc_T)
{
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) *
            ((int32_t)calib.dig_T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) *
             ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) *
             ((int32_t)calib.dig_T3)) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;

    return T;
}

static uint32_t bme280_compensate_H_x1024(int32_t adc_H)
{
    int32_t v_x1_u32r;

    v_x1_u32r = (t_fine - ((int32_t)76800));

    v_x1_u32r = (((((adc_H << 14) -
                  (((int32_t)calib.dig_H4) << 20) -
                  (((int32_t)calib.dig_H5) * v_x1_u32r)) +
                  ((int32_t)16384)) >> 15) *
                  (((((((v_x1_u32r * ((int32_t)calib.dig_H6)) >> 10) *
                  (((v_x1_u32r * ((int32_t)calib.dig_H3)) >> 11) +
                  ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
                  ((int32_t)calib.dig_H2) + 8192) >> 14));

    v_x1_u32r = (v_x1_u32r -
                (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                ((int32_t)calib.dig_H1)) >> 4));

    if (v_x1_u32r < 0) {
        v_x1_u32r = 0;
    }
    if (v_x1_u32r > 419430400) {
        v_x1_u32r = 419430400;
    }

    return (uint32_t)(v_x1_u32r >> 12);
}

static uint32_t bme280_compensate_P_q24_8(int32_t adc_P)
{
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);

    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) +
           ((var1 * (int64_t)calib.dig_P2) << 12);

    var1 = (((((int64_t)1) << 47) + var1) * ((int64_t)calib.dig_P1)) >> 33;

    if (var1 == 0) {
        return 0xFFFFFFFFu;
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;

    var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);

    return (uint32_t)p;
}

int8_t bme280_read_data(int32_t *temp_x100, uint32_t *hum_x100, uint32_t *press_pa)
{
    int32_t adc_P, adc_T, adc_H;
    uint32_t hum_x1024;
    uint32_t press_q24_8;

    if ((temp_x100 == NULL) || (hum_x100 == NULL) || (press_pa == NULL)) {
        return -1;
    }

    if (bme280_wait_measurement_done() != 1) {
        return -1;
    }

    if (bme280_read_raw(&adc_P, &adc_T, &adc_H) != 1) {
        return -1;
    }

    *temp_x100 = bme280_compensate_T_x100(adc_T);

    hum_x1024 = bme280_compensate_H_x1024(adc_H);
    if (hum_x1024 > 102400U) {
        hum_x1024 = 102400U;
    }
    *hum_x100 = (hum_x1024 * 100U) / 1024U;

    press_q24_8 = bme280_compensate_P_q24_8(adc_P);
    if (press_q24_8 == 0xFFFFFFFFu) {
        return -1;
    }

    *press_pa = press_q24_8 / 256U;

    return 1;
}