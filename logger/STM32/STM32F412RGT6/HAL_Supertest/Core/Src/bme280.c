#include "bme280.h"
#include "spi.h"
#include "support.h"
#include "app_flags.h"

#define REG_ID          0xD0
#define REG_RESET       0xE0
#define REG_STATUS      0xF3
#define REG_CTRL_HUM    0xF2
#define REG_CTRL_MEAS   0xF4
#define REG_CONFIG      0xF5
#define REG_PRESS_MSB   0xF7

#define REG_DIG_T1 0x88
#define REG_DIG_T2 0x8A
#define REG_DIG_T3 0x8C
#define REG_DIG_P1 0x8E
#define REG_DIG_P2 0x90
#define REG_DIG_P3 0x92
#define REG_DIG_P4 0x94
#define REG_DIG_P5 0x96
#define REG_DIG_P6 0x98
#define REG_DIG_P7 0x9A
#define REG_DIG_P8 0x9C
#define REG_DIG_P9 0x9E

#define REG_DIG_H1 0xA1
#define REG_DIG_H2 0xE1

#define BME280_CHIP_ID             0x60
#define BME280_STATUS_IM_UPDATE    0x01
#define BME280_STATUS_MEASURING    0x08

#define OSRS_1X      0x01
#define MODE_SLEEP   0x00
#define MODE_FORCED  0x01

#define BME_MEAS_CHECK_MS  2U
#define BME_MEAS_START_MS  20U
#define BME_XFER_MAX_DATA  32U
#define BME_PERIOD_MS      1000U

typedef enum {
    BME_IDLE = 0,
    BME_WAIT_MEAS
} bme_state_t;

typedef struct {
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

static bme_state_t bme_state = BME_IDLE;
static soft_timer_t bme_timer;
static uint32_t bme_last_start = 0U;

static bme280_calib_t calib;
static int32_t t_fine = 0;

static HAL_StatusTypeDef spi1_xfer_blocking(const uint8_t *tx, uint8_t *rx, uint16_t len)
{
    if ((tx == NULL) || (rx == NULL) || (len == 0U)) {
        return HAL_ERROR;
    }

    return HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)tx, rx, len, 100U);
}

static void bme280_read_registers(uint8_t reg, uint8_t *data, uint32_t size)
{
    uint32_t len;
    uint8_t txbuf[1U + BME_XFER_MAX_DATA];
    uint8_t rxbuf[1U + BME_XFER_MAX_DATA];

    if ((data == NULL) || (size == 0U) || (size > BME_XFER_MAX_DATA)) {
        bme280_error_flag = 1U;
        return;
    }

    len = size + 1U;
    txbuf[0] = reg | 0x80U;
    for (uint32_t i = 1U; i < len; i++) {
        txbuf[i] = 0x00U;
    }

    spi1_cs1_low();
    if (spi1_xfer_blocking(txbuf, rxbuf, (uint16_t)len) != HAL_OK) {
        bme280_error_flag = 1U;
    }
    spi1_cs1_high();

    for (uint32_t i = 0U; i < size; i++) {
        data[i] = rxbuf[i + 1U];
    }
}

static void bme280_write_register(uint8_t reg, uint8_t value)
{
    uint8_t txbuf[2];
    uint8_t rxbuf[2];

    txbuf[0] = reg & 0x7FU;
    txbuf[1] = value;

    spi1_cs1_low();
    if (spi1_xfer_blocking(txbuf, rxbuf, 2U) != HAL_OK) {
        bme280_error_flag = 1U;
    }
    spi1_cs1_high();
}

static uint8_t bme280_read_status(void)
{
    uint8_t st = 0U;
    bme280_read_registers(REG_STATUS, &st, 1U);
    return st;
}

static int16_t sign_extend_12(int16_t v)
{
    if ((v & 0x0800) != 0) {
        v |= (int16_t)0xF000;
    }
    return v;
}

static void bme280_read_calibration_data(void)
{
    uint8_t data[26];

    bme280_read_registers(REG_DIG_T1, data, 24U);
    calib.dig_T1 = (uint16_t)(data[0] | (data[1] << 8));
    calib.dig_T2 = (int16_t)(data[2] | (data[3] << 8));
    calib.dig_T3 = (int16_t)(data[4] | (data[5] << 8));
    calib.dig_P1 = (uint16_t)(data[6] | (data[7] << 8));
    calib.dig_P2 = (int16_t)(data[8] | (data[9] << 8));
    calib.dig_P3 = (int16_t)(data[10] | (data[11] << 8));
    calib.dig_P4 = (int16_t)(data[12] | (data[13] << 8));
    calib.dig_P5 = (int16_t)(data[14] | (data[15] << 8));
    calib.dig_P6 = (int16_t)(data[16] | (data[17] << 8));
    calib.dig_P7 = (int16_t)(data[18] | (data[19] << 8));
    calib.dig_P8 = (int16_t)(data[20] | (data[21] << 8));
    calib.dig_P9 = (int16_t)(data[22] | (data[23] << 8));

    bme280_read_registers(REG_DIG_H1, &data[0], 1U);
    calib.dig_H1 = data[0];

    bme280_read_registers(REG_DIG_H2, &data[0], 7U);
    calib.dig_H2 = (int16_t)(data[0] | (data[1] << 8));
    calib.dig_H3 = data[2];

    {
        int16_t h4 = (int16_t)((((int16_t)data[3]) << 4) | (data[4] & 0x0F));
        int16_t h5 = (int16_t)((((int16_t)data[5]) << 4) | (data[4] >> 4));

        calib.dig_H4 = sign_extend_12(h4);
        calib.dig_H5 = sign_extend_12(h5);
        calib.dig_H6 = (int8_t)data[6];
    }
}

uint8_t bme280_read_id(void)
{
    uint8_t id = 0U;
    bme280_read_registers(REG_ID, &id, 1U);
    return id;
}

void bme280_init(void)
{
    bme280_present = 0U;
    bme280_error_flag = 1U;
    bme_state = BME_IDLE;
    timer_stop(&bme_timer);

    bme280_write_register(REG_RESET, 0xB6);
    HAL_Delay(10U);

    if (bme280_read_id() != BME280_CHIP_ID) {
        return;
    }

    {
        uint32_t start = HAL_GetTick();
        while ((bme280_read_status() & BME280_STATUS_IM_UPDATE) != 0U) {
            if ((HAL_GetTick() - start) > 100U) {
                return;
            }
        }
    }

    bme280_read_calibration_data();

    bme280_write_register(REG_CTRL_HUM, OSRS_1X);
    bme280_write_register(REG_CONFIG, (0x05U << 5) | (0x00U << 2) | 0x00U);
    bme280_write_register(REG_CTRL_MEAS,
                          (OSRS_1X << 5) | (OSRS_1X << 2) | MODE_SLEEP);

    bme280_present = 1U;
    bme280_error_flag = 0U;
    bme_last_start = HAL_GetTick();
}

void bme280_trigger_forced(void)
{
    bme280_write_register(REG_CTRL_HUM, OSRS_1X);
    bme280_write_register(REG_CTRL_MEAS,
                          (OSRS_1X << 5) | (OSRS_1X << 2) | MODE_FORCED);
}

static uint8_t bme280_read_raw(int32_t *adc_P, int32_t *adc_T, int32_t *adc_H)
{
    uint8_t data[8];

    if ((adc_P == NULL) || (adc_T == NULL) || (adc_H == NULL)) return 1U;

    bme280_read_registers(REG_PRESS_MSB, data, 8U);

    *adc_P = (int32_t)((((uint32_t)data[0]) << 12) |
                       (((uint32_t)data[1]) << 4) |
                       (((uint32_t)data[2]) >> 4));

    *adc_T = (int32_t)((((uint32_t)data[3]) << 12) |
                       (((uint32_t)data[4]) << 4) |
                       (((uint32_t)data[5]) >> 4));

    *adc_H = (int32_t)((((uint32_t)data[6]) << 8) |
                       ((uint32_t)data[7]));
    return 0U;
}

static int32_t bme280_compensate_T_x100(int32_t adc_T)
{
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) * ((int32_t)calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) *
            ((int32_t)calib.dig_T3)) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

static uint32_t bme280_compensate_H_x1024(int32_t adc_H)
{
    int32_t v_x1_u32r;

    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)calib.dig_H4) << 20) - (((int32_t)calib.dig_H5) * v_x1_u32r)) +
                  ((int32_t)16384)) >> 15) *
                (((((((v_x1_u32r * ((int32_t)calib.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)calib.dig_H3)) >> 11) +
                   ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)calib.dig_H2) + 8192) >> 14));

    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)calib.dig_H1)) >> 4));

    if (v_x1_u32r < 0) v_x1_u32r = 0;
    if (v_x1_u32r > 419430400) v_x1_u32r = 419430400;

    return (uint32_t)(v_x1_u32r >> 12);
}

static uint32_t bme280_compensate_P(int32_t adc_P)
{
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) + ((var1 * (int64_t)calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_P1) >> 33;

    if (var1 == 0) return 0U;

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
    return (uint32_t)p;
}

uint8_t bme280_read_data(int32_t *temp_x100, uint32_t *hum_x100, uint32_t *press_pa)
{
    int32_t adc_P, adc_T, adc_H;
    uint32_t hum_x1024;
    uint32_t press_q24_8;

    if ((temp_x100 == NULL) || (hum_x100 == NULL) || (press_pa == NULL)) return 1U;
    if (bme280_read_raw(&adc_P, &adc_T, &adc_H) != 0U) return 1U;

    *temp_x100 = bme280_compensate_T_x100(adc_T);

    hum_x1024 = bme280_compensate_H_x1024(adc_H);
    if (hum_x1024 > 102400U) hum_x1024 = 102400U;
    *hum_x100 = (hum_x1024 * 100U) / 1024U;

    press_q24_8 = bme280_compensate_P(adc_P);
    *press_pa = press_q24_8 / 256U;

    return 0U;
}

void bme280_task(void)
{
    static int32_t  temp_x100;
    static uint32_t hum_x100;
    static uint32_t press_pa;
    uint32_t now;

    if (!bme280_present) {
        bme280_error_flag = 1U;
        bme_state = BME_IDLE;
        timer_stop(&bme_timer);
        return;
    }

    now = HAL_GetTick();

    switch (bme_state) {
    case BME_IDLE:
        if ((uint32_t)(now - bme_last_start) < BME_PERIOD_MS) {
            return;
        }

        bme280_trigger_forced();
        timer_start(&bme_timer, BME_MEAS_START_MS);
        bme_last_start = now;
        bme_state = BME_WAIT_MEAS;
        break;

    case BME_WAIT_MEAS:
    {
        uint8_t st;

        if (!timer_expired(&bme_timer)) {
            return;
        }

        st = bme280_read_status();
        if ((st & (BME280_STATUS_MEASURING | BME280_STATUS_IM_UPDATE)) != 0U) {
            timer_start(&bme_timer, BME_MEAS_CHECK_MS);
            return;
        }

        if (bme280_read_data(&temp_x100, &hum_x100, &press_pa) == 0U) {
            if (!(temp_x100 < -4000 || temp_x100 > 8500 ||
                  hum_x100 > 10000U || press_pa == 0U)) {
                measurement_bme280.temperature = temp_x100;
                measurement_bme280.humidity    = hum_x100;
                measurement_bme280.pressure    = press_pa;
                bme280_error_flag = 0U;
            } else {
                bme280_error_flag = 1U;
            }
        } else {
            bme280_error_flag = 1U;
        }

        bme_state = BME_IDLE;
        break;
    }

    default:
        bme_state = BME_IDLE;
        break;
    }
}