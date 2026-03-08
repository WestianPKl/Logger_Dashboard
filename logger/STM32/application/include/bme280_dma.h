#ifndef BME280_DMA_H
#define BME280_DMA_H

#include <stdint.h>

void bme280_init(void);
void bme280_trigger_forced(void);
uint8_t bme280_read_id(void);
uint8_t bme280_read_data(int32_t *temp_c, uint32_t *hum_pct, uint32_t *press_hPa);

#endif // BME280_DMA_H