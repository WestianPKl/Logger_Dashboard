#ifndef FLASH_LOG_H
#define FLASH_LOG_H

#include <stdint.h>
#include "main.h"
#include "bme280.h"
#include "sht40.h"
#include "app_flags.h"

typedef struct __attribute__((packed)) {
    uint32_t sequence;
    uint32_t timestamp;

    int32_t  temp_x100;
    uint32_t hum_x100;
    uint32_t press_pa;

    uint32_t crc32;
} flash_log_record_t;

int flash_log_init(void);
int flash_log_clear(void);

int flash_log_append(uint32_t timestamp,
                     const bme280_data_t *bme,
                     const sht40_data_t *sht);

int flash_log_read_oldest(uint32_t index, flash_log_record_t *rec);
int flash_log_read_latest(uint32_t index, flash_log_record_t *rec);

uint32_t flash_log_count(void);
uint32_t flash_log_capacity(void);

#endif // FLASH_LOG_H