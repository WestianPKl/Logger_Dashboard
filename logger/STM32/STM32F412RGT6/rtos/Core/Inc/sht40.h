#ifndef SHT40_H
#define SHT40_H

#include <stdint.h>

typedef struct {
  int16_t temperature;
  uint16_t humidity;
} sht40_data_t;

#include "main.h"

uint32_t sht40_read_serial_number(void);
int8_t sht40_data_read_int(int16_t *temp_c, uint16_t *rh);
int8_t sht40_data_read_int_it(int16_t *temp_c, uint16_t *rh);

#endif // SHT40_H