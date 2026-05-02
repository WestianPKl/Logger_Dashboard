#ifndef BME280_H
#define BME280_H

#include <stdint.h>

/*
    * @brief  Structure holding compensated BME280 sensor data.
    * @field  temperature: Compensated temperature in hundredths of degrees Celsius (e.g., 2534 = 25.34 C).
    * @field  humidity: Compensated relative humidity in hundredths of percent (e.g., 4532 = 45.32% RH).
    * @field  pressure: Compensated pressure in Pascals (e.g., 101325 = 1013.25 hPa).
*/
typedef struct {
  int32_t temperature;
  uint32_t humidity;
  uint32_t pressure;
} bme280_data_t;

/*
    * @brief  Initialize the BME280 sensor with the desired settings.
    *         This function performs a software reset of the BME280 sensor, waits for it to complete its internal NVM copy,
    *         reads the calibration data, and configures the sensor's control registers for humidity and measurement settings.
    * @retval 1 on successful initialization, -1 on failure (e.g., SPI communication error or sensor not responding).
*/
int8_t bme280_init(void);

/*
    * @brief  Read the chip ID from the BME280 sensor to verify communication.
    *         This function reads the ID register of the BME280 sensor.
    *         It is typically used during initialization to confirm that the sensor is responding correctly over SPI.
    * @retval The chip ID on success (expected 0x60), -1 on failure (e.g., SPI read error).
*/
int8_t bme280_read_id(void);

/*
    * @brief  Trigger a forced measurement on the BME280 sensor.
    *         This function configures the BME280 sensor to perform a single forced measurement by setting the appropriate control registers.
    *         After calling this function, the sensor will take a measurement and then return to sleep mode.
    * @retval 1 on success, -1 on failure (e.g., SPI communication error).
*/
int8_t bme280_trigger_forced(void);

/*
    * @brief  Read the compensated temperature, humidity, and pressure data from the BME280 sensor.
    *         This function waits for the current measurement to complete, reads the raw ADC values,
    *         and applies the compensation algorithms to obtain accurate readings. bme280_trigger_forced() must be called before this function.
    * @param  temp_x100: Pointer to an integer where the compensated temperature in hundredths of degrees Celsius will be stored.
    * @param  hum_x100: Pointer to an integer where the compensated relative humidity in hundredths of percent will be stored.
    * @param  press_pa: Pointer to an integer where the compensated pressure in Pascals will be stored.
    * @retval 1 on success, -1 on failure (e.g., SPI communication error or invalid parameters).
*/
int8_t bme280_read_data(int32_t *temp_x100, uint32_t *hum_x100, uint32_t *press_pa);

#endif // BME280_H