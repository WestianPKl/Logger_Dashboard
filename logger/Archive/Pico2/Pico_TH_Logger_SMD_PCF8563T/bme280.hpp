/**
 * @file
 * @brief Minimal BME280 driver interface for the Raspberry Pi Pico (RP2040) SDK.
 *
 * Provides a C++ wrapper to read temperature, pressure, and humidity from a Bosch BME280
 * over I2C, and computes altitude using a configurable sea-level pressure reference.
 */

/**
 * @class BME280
 * @brief High-level driver for the Bosch BME280 environmental sensor (I2C).
 *
 * Responsibilities:
 * - Configure measurement mode and oversampling for the sensor.
 * - Read raw ADC values and apply Bosch compensation formulas.
 * - Return compensated temperature (°C), relative humidity (%RH), and pressure (hPa).
 * - Compute altitude (m) from pressure using a barometric formula and sea-level reference.
 *
 * Notes:
 * - The default I2C address is 0x76. Some modules use 0x77.
 * - The implementation assumes the Pico SDK I2C APIs are initialized externally.
 * - This class is not inherently thread-safe.
 */

/**
 * @enum BME280::MODE
 * @brief Sensor operating mode.
 * - MODE_SLEEP: Low-power sleep mode; no ongoing measurements.
 * - MODE_FORCED: Perform a single measurement cycle, then return to sleep.
 * - MODE_NORMAL: Continuous measurement with standby timing configured internally.
 */

/**
 * @brief Standard sea-level pressure in hPa used to compute altitude.
 *
 * You may adjust this constant to your local sea-level pressure to improve altitude accuracy.
 */
 
/**
 * @brief Internal I2C read bit mask (OR-ed with register address to perform a read).
 */

/**
 * @brief Current I2C 7-bit device address (default: 0x76).
 */

/**
 * @brief Fine-resolution temperature value used by Bosch compensation formulas.
 *
 * This is an intermediate state updated by temperature compensation and reused for pressure/humidity.
 */

/**
 * @name Temperature calibration coefficients (from sensor NVM)
 * @brief Raw calibration parameters required for temperature compensation.
 * @{
 */
/** @brief dig_T1 (unsigned). */
/** @brief dig_T2 (signed). */
/** @brief dig_T3 (signed). */
/** @} */

/**
 * @name Pressure calibration coefficients (from sensor NVM)
 * @brief Raw calibration parameters required for pressure compensation.
 * @{
 */
/** @brief dig_P1 (unsigned). */
/** @brief dig_P2..dig_P9 (signed). Coefficients for Bosch pressure formula. */
/** @} */

/**
 * @name Humidity calibration coefficients (from sensor NVM)
 * @brief Raw calibration parameters required for humidity compensation.
 * @{
 */
/** @brief dig_H1, dig_H3, dig_H6 (mixed signedness per datasheet). */
/** @brief dig_H2, dig_H4, dig_H5 (signed). */
/** @} */

/**
 * @brief Latest raw ADC readings captured from the sensor (temperature, pressure, humidity).
 *
 * These hold the un-compensated 20-bit (T, P) and 16-bit (H) values as read from the device.
 */

/**
 * @brief Small scratch buffer used for multi-byte register transactions.
 */

/**
 * @brief Cached chip identifier read from the sensor (expected 0x60 for BME280).
 */

/**
 * @brief Currently configured operating mode.
 */

/**
 * @struct BME280::MeasurementControl_t
 * @brief Encodes oversampling and mode fields for the CTRL_MEAS register.
 *
 * Fields:
 * - osrs_t: Temperature oversampling (0–5), see BME280 datasheet.
 * - osrs_p: Pressure oversampling (0–5), see BME280 datasheet.
 * - mode: Operating mode (sleep/forced/normal).
 *
 * Use get() to obtain the packed register value.
 */
/**
 * @brief Pack the osrs_t, osrs_p, and mode fields into a CTRL_MEAS register byte.
 * @return Bit-packed register value suitable for writing to CTRL_MEAS.
 */

/**
 * @struct BME280::Measurement_t
 * @brief Container for a single set of compensated environmental readings.
 *
 * - temperature: Degrees Celsius.
 * - humidity: Relative humidity in percent (%RH).
 * - pressure: Pressure in hectopascals (hPa).
 * - altitude: Altitude above sea level in meters, derived from pressure and SEA_LEVEL_HPA.
 */

/**
 * @brief Construct a BME280 driver with the requested operating mode.
 *
 * Does not initialize the I2C peripheral; expects it to be configured by the caller.
 * May read and cache calibration data from the sensor during initialization.
 *
 * @param mode Sensor operating mode (sleep, forced, or normal). Defaults to MODE_NORMAL.
 */

/**
 * @brief Perform a measurement and return compensated environmental values.
 *
 * - Reads raw ADC registers from the device.
 * - Applies Bosch compensation algorithms using stored calibration coefficients.
 * - Converts pressure to hPa and computes altitude (m) using SEA_LEVEL_HPA.
 *
 * In MODE_FORCED, this will trigger a single measurement before reading results.
 * In MODE_NORMAL, it reads the most recent measurement.
 *
 * @return Populated Measurement_t with temperature (°C), humidity (%RH), pressure (hPa), and altitude (m).
 */

/**
 * @brief Read and return the BME280 chip ID.
 * @return 8-bit chip identifier (typically 0x60 for BME280).
 */

/**
 * @brief Apply Bosch temperature compensation to a raw ADC sample.
 *
 * Updates the internal t_fine state used by pressure/humidity compensation.
 * @param adc_T Raw 20-bit temperature ADC value.
 * @return Fixed-point compensated temperature per the datasheet (internal units).
 */

/**
 * @brief Apply Bosch pressure compensation to a raw ADC sample.
 * @param adc_P Raw 20-bit pressure ADC value.
 * @return Compensated pressure in integer form (typically Pa as per datasheet).
 */

/**
 * @brief Apply Bosch humidity compensation to a raw ADC sample.
 * @param adc_H Raw 16-bit humidity ADC value.
 * @return Compensated humidity in integer form (scaled as per datasheet).
 */

/**
 * @brief Read raw humidity, pressure, and temperature ADC values from the sensor.
 * @param[out] humidity Pointer to receive raw humidity ADC value.
 * @param[out] pressure Pointer to receive raw pressure ADC value.
 * @param[out] temperature Pointer to receive raw temperature ADC value.
 */

/**
 * @brief Write a single byte to a device register over I2C.
 * @param reg Register address.
 * @param data Value to write.
 */

/**
 * @brief Read multiple bytes starting at a device register over I2C.
 * @param reg Starting register address.
 * @param buf Destination buffer.
 * @param len Number of bytes to read.
 */

/**
 * @brief Read and cache all sensor compensation parameters from NVM.
 *
 * Must be called before performing compensation to ensure valid coefficients are loaded.
 */
#ifndef __BME280_HPP__
#define __BME280_HPP__

#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

class BME280 {
public:
    enum MODE {
        MODE_SLEEP  = 0b00,
        MODE_FORCED = 0b01,
        MODE_NORMAL = 0b11
    };

    static constexpr float SEA_LEVEL_HPA = 1013.25f;

private:
    const uint READ_BIT = 0x80;
    uint8_t     addr = 0x76;
    int32_t     t_fine = 0;
    uint16_t    dig_T1 = 0;
    int16_t     dig_T2 = 0, dig_T3 = 0;
    uint16_t    dig_P1 = 0;
    int16_t     dig_P2 = 0, dig_P3 = 0, dig_P4 = 0, dig_P5 = 0, dig_P6 = 0, dig_P7 = 0, dig_P8 = 0, dig_P9 = 0;
    uint8_t     dig_H1 = 0, dig_H3 = 0;
    int8_t      dig_H6 = 0;
    int16_t     dig_H2 = 0, dig_H4 = 0, dig_H5 = 0;
    int32_t     adc_T = 0, adc_P = 0, adc_H = 0;
    uint8_t     buffer[26]{};
    uint8_t     chip_id = 0;
    MODE        mode;

    struct MeasurementControl_t {
        unsigned int osrs_t : 3;
        unsigned int osrs_p : 3;
        unsigned int mode   : 2;
        unsigned int get() const { return (osrs_t << 5) | (osrs_p << 2) | mode; }
    } measurement_reg{};

public:
    struct Measurement_t {
        float temperature;
        float humidity;
        float pressure;
        float altitude;
    } measurement{};

    explicit BME280(MODE mode = MODE_NORMAL);

    Measurement_t measure();
    uint8_t get_chipID();

private:
    int32_t  compensate_temp(int32_t adc_T);
    uint32_t compensate_pressure(int32_t adc_P);
    uint32_t compensate_humidity(int32_t adc_H);

    void     bme280_read_raw(int32_t *humidity, int32_t *pressure, int32_t *temperature);
    void     write_register(uint8_t reg, uint8_t data);
    void     read_registers(uint8_t reg, uint8_t *buf, uint16_t len);
    void     read_compensation_parameters();
};

#endif /* __BME280_HPP__ */