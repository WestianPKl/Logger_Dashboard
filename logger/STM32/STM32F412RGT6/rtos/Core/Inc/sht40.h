#ifndef SHT40_H
#define SHT40_H

#include <stdint.h>

/*
    * @brief  Structure holding compensated SHT40 sensor data.
    * @field  temperature: Temperature in hundredths of degrees Celsius (e.g., 2534 = 25.34 C).
    * @field  humidity: Relative humidity in hundredths of percent (e.g., 4532 = 45.32% RH).
*/
typedef struct {
  int16_t temperature;
  uint16_t humidity;
} sht40_data_t;

#include "main.h"

/*
    * @brief  Read the unique serial number from the SHT40 sensor.
    * @retval 32-bit serial number on success, (uint32_t)-1 on failure.
*/
uint32_t sht40_read_serial_number(void);

/*
    * @brief  Perform a single high-precision measurement on the SHT40 sensor and return compensated values.
    * @param  temp_c: Pointer where the temperature in hundredths of degrees Celsius will be stored.
    * @param  rh: Pointer where the relative humidity in hundredths of percent will be stored.
    * @retval 1 on success, -1 on failure.
*/
int8_t sht40_data_read_int(int16_t *temp_c, uint16_t *rh);

#endif // SHT40_H