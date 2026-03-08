#include "bme280.hpp"

/**
 * @brief Construct a BME280 driver and initialize sensor configuration.
 *
 * Performs device discovery and setup by:
 * - Reading the chip ID from register 0xD0 into the internal chip_id.
 * - Reading and caching factory compensation parameters for later measurements.
 * - Writing SLEEP to ctrl_meas (0xF4) to safely configure settings.
 * - Setting humidity oversampling (ctrl_hum, 0xF2) to x1.
 * - Setting temperature and pressure oversampling to x4 and applying the requested operating mode via ctrl_meas (0xF4).
 *   (The ctrl_hum setting is latched by the subsequent ctrl_meas write per the datasheet.)
 *
 * @param mode Desired operating mode to apply to ctrl_meas (e.g., SLEEP, FORCED, NORMAL).
 *
 * @pre I2C/SPI bus must be initialized and the sensor must be reachable at the configured address/CS.
 * @post Sensor is configured with osrs_t = x4, osrs_p = x4, osrs_h = x1, and placed in the requested mode.
 *
 * @note Registers accessed: 0xD0 (chip ID), 0xF2 (ctrl_hum), 0xF4 (ctrl_meas).
 * @warning Performs blocking hardware I/O; ensure safe calling context. Error signaling depends on the underlying read/write implementation.
 */
BME280::BME280(MODE mode) : mode(mode) {
    measurement_reg.mode   = mode;
    measurement_reg.osrs_p = 0b011;
    measurement_reg.osrs_t = 0b011;
    read_registers(0xD0, &chip_id, 1);
    read_compensation_parameters();
    write_register(0xF4, MODE_SLEEP); 
    write_register(0xF2, 0x01);
    write_register(0xF4, (uint8_t)measurement_reg.get());
}

/**
 * Perform a single sensor read and return a fully compensated measurement.
 *
 * Behavior:
 * - If the device is configured in MODE_FORCED, triggers a one-shot
 *   conversion by writing to ctrl_meas (0xF4) and busy-waits on the
 *   status register (0xF3, measuring bit) until completion or a ~200 ms
 *   timeout.
 * - Reads raw temperature, pressure, and humidity, applies Bosch
 *   compensation, and updates the internal Measurement_t.
 *
 * Returns:
 * - Measurement_t with fields populated as:
 *   - temperature: degrees Celsius.
 *   - pressure: hectopascals (hPa).
 *   - humidity: percent relative humidity (%RH), clamped to [0, 100].
 *   - altitude: meters, derived from pressure using the barometric
 *     formula relative to SEA_LEVEL_HPA.
 *
 * Notes:
 * - This call may block for up to ~200 ms in forced mode.
 * - On timeout, proceeds with whatever data are present in the sensor
 *   registers, which may be stale if conversion has not finished.
 * - Assumes calibration data have been loaded and compensation helpers
 *   are available.
 */
BME280::Measurement_t BME280::measure() {
    int32_t raw_p, raw_h, raw_t;
    if (measurement_reg.mode == MODE_FORCED) {
        write_register(0xF4, (uint8_t)measurement_reg.get());
        uint8_t status = 0;
        absolute_time_t deadline = make_timeout_time_ms(200);
        do {
            read_registers(0xF3, &status, 1);
            sleep_ms(1);
            if (time_reached(deadline)) break;
        } while (status & 0x08);
    }

    bme280_read_raw(&raw_h, &raw_p, &raw_t);
    int32_t  t100 = compensate_temp(raw_t);
    uint32_t pPa  = compensate_pressure(raw_p);
    uint32_t hQ12 = compensate_humidity(raw_h);
    measurement.temperature = t100 / 100.0f;
    measurement.pressure = (pPa >> 8) / 100.0f;
    measurement.humidity    = hQ12 / 1024.0f;
    if (measurement.humidity < 0.0f)   measurement.humidity = 0.0f;
    if (measurement.humidity > 100.0f) measurement.humidity = 100.0f;
    measurement.altitude = 44330.0f * (1.0f - powf(measurement.pressure / SEA_LEVEL_HPA, 0.19029495f));
    return measurement;
}

/**
 * @brief Returns the BME280 sensor chip identifier.
 *
 * Provides the cached chip ID read from the device (typically 0x60 for BME280).
 * Useful for verifying sensor presence and bus communication after initialization.
 *
 * @return uint8_t The last-read chip ID value of the connected sensor.
 */
uint8_t BME280::get_chipID() {
    return chip_id;
}

/**
 * @brief Compensate a raw BME280 temperature reading.
 *
 * Applies the Bosch BME280 fixed-point compensation formula using the device’s
 * calibration coefficients (dig_T1, dig_T2, dig_T3) to compute a calibrated
 * temperature and updates the internal fine temperature state.
 *
 * Side effects:
 * - Updates the member variable t_fine, which must be used for subsequent
 *   pressure and humidity compensation routines.
 *
 * @param adc_T Raw 20-bit uncompensated temperature value read from the sensor,
 *              combined as (msb << 12) | (lsb << 4) | (xlsb >> 4).
 * @return int32_t Compensated temperature in centi-degrees Celsius (0.01 °C units),
 *                 e.g., 5123 represents 51.23 °C.
 *
 * @pre Calibration parameters dig_T1, dig_T2, and dig_T3 must be read from the device.
 * @note This function performs only compensation; it does not trigger a measurement.
 * @warning Not thread-safe if the same instance is shared across threads, due to t_fine.
 */
int32_t BME280::compensate_temp(int32_t adc_T) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t) dig_T1 << 1))) * ((int32_t) dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t) dig_T1)) * ((adc_T >> 4) - ((int32_t) dig_T1))) >> 12) * ((int32_t) dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

/**
 * @brief Compensates a raw BME280 pressure reading using sensor calibration data.
 *
 * Implements Bosch’s integer pressure compensation routine with 64-bit
 * intermediates and the previously computed temperature fine value (t_fine).
 *
 * @param adc_P Raw uncompensated pressure sample from the sensor (20-bit range: 0..1048575).
 * @return Calibrated pressure in Q24.8 fixed-point Pascals (Pa).
 *         - To get pressure in Pa as a float: static_cast<float>(return_value) / 256.0f
 *         - To get pressure in hPa (mbar): static_cast<float>(return_value) / 25600.0f
 *
 * @pre Calibration coefficients dig_P1..dig_P9 must be valid and t_fine must have
 *      been computed from a prior temperature compensation.
 * @retval 0 Returned if an invalid calibration state would cause division by zero (e.g., dig_P1 == 0).
 * @note This function only compensates pressure; it relies on a valid t_fine from the temperature path.
 */
uint32_t BME280::compensate_pressure(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1) * (int64_t)dig_P1) >> 33;
    if (var1 == 0) {
        return 0;
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = (((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4));
    return (uint32_t)p;
}

/**
 * @brief Compensate raw humidity ADC reading using BME280 calibration data.
 *
 * Computes relative humidity from the raw (uncompensated) humidity value using
 * the fixed-point compensation algorithm described in the Bosch BME280 datasheet.
 * Requires valid temperature fine-resolution value (t_fine) and humidity calibration
 * coefficients (dig_H1..dig_H6) to be initialized beforehand.
 *
 * @param adc_H Raw uncompensated humidity reading from the sensor ADC (typically 16-bit).
 *
 * @return Relative humidity in fixed-point Q22.10 format (LSB = 1/1024 %RH).
 *         To obtain %RH as a floating-point value: rh_percent = return_value / 1024.0f.
 *         The result is saturated to the range [0, 100%] (i.e., [0, 102400] in Q22.10).
 *
 * @note Call the temperature compensation routine first to update t_fine.
 * @note Uses only integer arithmetic; no floating-point operations are performed.
 * @warning Not thread-safe if t_fine or calibration fields can change concurrently.
 * @see Bosch Sensortec BME280 datasheet, section "Compensation formulae".
 */
uint32_t BME280::compensate_humidity(int32_t adc_H) {
    int32_t v = t_fine - 76800;
    int32_t x = (((adc_H << 14) - ((int32_t)dig_H4 << 20) - ((int32_t)dig_H5 * v) + 16384) >> 15);
    int32_t y = (((((v * (int32_t)dig_H6) >> 10) * (((v * (int32_t)dig_H3) >> 11) + 32768)) >> 10) + 2097152);
    y = (y * (int32_t)dig_H2 + 8192) >> 14;
    int32_t r = x * y;
    int32_t r2 = (r >> 15);
    r -= (((r2 * r2) >> 7) * (int32_t)dig_H1) >> 4;
    if (r < 0) r = 0;
    if (r > 419430400) r = 419430400;
    return (uint32_t)(r >> 12);
}

/**
 * @brief Writes a single byte to a BME280 register over I2C.
 *
 * Constructs a two-byte payload {reg, data}, sends it via the default I2C
 * instance to the device address stored in 'addr' with a STOP condition, then
 * delays for 2 ms to allow the device to process the write.
 *
 * This function is synchronous and blocking due to both the I2C transfer and
 * the post-write delay.
 *
 * @param reg  8-bit register address to write to.
 * @param data 8-bit value to write into the register.
 *
 * @pre The I2C peripheral must be initialized and the BME280 device address
 *      (member 'addr') must be valid.
 *
 * @note Errors from the underlying transfer are not propagated to the caller.
 * @warning Includes a 2 ms sleep; avoid calling in time-critical paths.
 */
void BME280::write_register(uint8_t reg, uint8_t data) {
    uint8_t buf[2] = { reg, data };
    i2c_write_blocking(i2c_default, addr, buf, 2, false);
    sleep_ms(2);
}

/**
 * @brief Reads a sequence of registers from the BME280 over I2C into a buffer.
 *
 * Writes the starting register address without a STOP condition (repeated start),
 * then performs a blocking read of the requested number of bytes. Short delays
 * are inserted between bus operations to satisfy device timing.
 *
 * @param reg Starting register address to read from.
 * @param buf Pointer to the destination buffer where the data will be stored.
 *            Must reference at least @p len bytes of writable memory.
 * @param len Number of bytes to read.
 *
 * @pre The I2C peripheral must be initialized and configured, and the device
 *      address for this instance must be set.
 *
 * @note This function is blocking and introduces ~2 ms delays before and after
 *       the read transfer.
 *
 * @warning No bounds checking or error reporting is performed; the caller must
 *          ensure valid arguments and handle I2C errors at a higher level.
 */
void BME280::read_registers(uint8_t reg, uint8_t *buf, uint16_t len) {
    i2c_write_blocking(i2c_default, addr, &reg, 1, true);
    sleep_ms(2);
    i2c_read_blocking(i2c_default, addr, buf, len, false);
    sleep_ms(2);
}

/**
 * @brief Read and decode BME280 calibration (compensation) parameters.
 *
 * Populates the device's temperature (dig_T1..dig_T3), pressure (dig_P1..dig_P9),
 * and humidity (dig_H1..dig_H6) calibration coefficients by reading the BME280
 * register map:
 * - 0x88..0xA1 (26 bytes): temperature, pressure, and H1
 * - 0xE1..0xE7 (7 bytes): humidity H2..H6
 *
 * Decoding details:
 * - Little-endian 16-bit fields are combined from LSB/MSB bytes.
 * - Signedness per datasheet: T1 and P1 are unsigned; T2..T3 and P2..P9 are signed;
 *   H1 is unsigned; H2 is signed; H3 is unsigned; H6 is signed 8-bit.
 * - H4 and H5 are 12-bit signed values spanning nibbles across registers:
 *   H4 = (E4 << 4) | (E5 & 0x0F), H5 = (E6 << 4) | (E5 >> 4). Proper sign extension is applied.
 *
 * Preconditions:
 * - The BME280 is powered and responding on the bus.
 * - The underlying bus interface and read_registers() are initialized and functional.
 * - The temporary buffer is large enough for the requested bursts (>= 26 and >= 7 bytes).
 *
 * Notes:
 * - Must be called once after power-up/reset and before compensating raw measurements.
 * - This function does not perform explicit error handling; failures in read_registers()
 *   may leave calibration fields in an undefined state.
 *
 * Reference: Bosch BME280 datasheet (Calibration data registers).
 */
void BME280::read_compensation_parameters() {
    read_registers(0x88, buffer, 26);
    dig_T1 = (uint16_t)(buffer[0] | (buffer[1] << 8));
    dig_T2 = (int16_t)(buffer[2] | (buffer[3] << 8));
    dig_T3 = (int16_t)(buffer[4] | (buffer[5] << 8));
    dig_P1 = (uint16_t)(buffer[6] | (buffer[7] << 8));
    dig_P2 = (int16_t)(buffer[8] | (buffer[9] << 8));
    dig_P3 = (int16_t)(buffer[10] | (buffer[11] << 8));
    dig_P4 = (int16_t)(buffer[12] | (buffer[13] << 8));
    dig_P5 = (int16_t)(buffer[14] | (buffer[15] << 8));
    dig_P6 = (int16_t)(buffer[16] | (buffer[17] << 8));
    dig_P7 = (int16_t)(buffer[18] | (buffer[19] << 8));
    dig_P8 = (int16_t)(buffer[20] | (buffer[21] << 8));
    dig_P9 = (int16_t)(buffer[22] | (buffer[23] << 8));
    dig_H1 = (uint8_t)buffer[25];
    read_registers(0xE1, buffer, 7);
    dig_H2 = (int16_t)(buffer[0] | (buffer[1] << 8));
    dig_H3 = (uint8_t)buffer[2];
    int16_t h4 = (int16_t)(((int16_t)buffer[3] << 4) | (buffer[4] & 0x0F));
    if (h4 & 0x0800) { h4 |= 0xF000; }
    dig_H4 = h4;

    int16_t h5 = (int16_t)(((int16_t)buffer[5] << 4) | (buffer[4] >> 4));
    if (h5 & 0x0800) { h5 |= 0xF000; }
    dig_H5 = h5;
    dig_H6 = (int8_t)buffer[6];
}

/**
 * @brief Read raw (uncompensated) ADC measurements from the BME280 sensor.
 *
 * Reads 8 bytes starting at register 0xF7 and assembles:
 * - Pressure: 20-bit value from registers 0xF7..0xF9 (bits [19:0]).
 * - Temperature: 20-bit value from registers 0xFA..0xFC (bits [19:0]).
 * - Humidity: 16-bit value from registers 0xFD..0xFE.
 *
 * The results are written to the provided pointers as 32-bit integers; only the
 * lower 20 bits (pressure, temperature) or 16 bits (humidity) are significant.
 * These are raw ADC counts and must be compensated using the device’s calibration
 * parameters to obtain physical units (Pa, °C, %RH).
 *
 * @param[out] humidity    Pointer to receive the raw 16-bit humidity reading.
 * @param[out] pressure    Pointer to receive the raw 20-bit pressure reading.
 * @param[out] temperature Pointer to receive the raw 20-bit temperature reading.
 *
 * @pre The sensor must be initialized and configured; all pointers must be non-null.
 * @note This call performs synchronous I/O with the sensor and may block.
 */
void BME280::bme280_read_raw(int32_t *humidity, int32_t *pressure, int32_t *temperature) {
    uint8_t rb[8];
    read_registers(0xF7, rb, 8);
    *pressure    = ((uint32_t)rb[0] << 12) | ((uint32_t)rb[1] << 4) | (rb[2] >> 4);
    *temperature = ((uint32_t)rb[3] << 12) | ((uint32_t)rb[4] << 4) | (rb[5] >> 4);
    *humidity    = ((uint32_t)rb[6] << 8)  |  (uint32_t)rb[7];
}
