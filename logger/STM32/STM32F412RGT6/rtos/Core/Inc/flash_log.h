#ifndef FLASH_LOG_H
#define FLASH_LOG_H

#include <stdint.h>
#include "main.h"
#include "bme280.h"
#include "sht40.h"
#include "app_flags.h"

/*
    * @brief  Structure representing a single flash log record with sensor data, sequence number, and CRC.
*/
typedef struct __attribute__((packed)) {
    uint32_t sequence;
    uint32_t timestamp;

    int32_t  temp_x100;
    uint32_t hum_x100;
    uint32_t press_pa;

    uint32_t crc32;
} flash_log_record_t;

/*
    * @brief  Initialize the flash log subsystem.
    *         Initializes the external SPI flash, loads the log metadata from FRAM, and sets defaults if no valid metadata is found.
    * @retval 1 on success, -1 on failure.
*/
int flash_log_init(void);

/*
    * @brief  Erase all log records from external flash and reset the log metadata.
    * @retval 1 on success, -1 on failure.
*/
int flash_log_clear(void);

/*
    * @brief  Append a new sensor data record to the flash log.
    *         Writes a new record at the current write position and advances the index in a circular fashion.
    *         Automatically erases the flash sector when crossing a sector boundary.
    * @param  timestamp: Unix-style timestamp for the record.
    * @param  bme: Pointer to BME280 sensor data, or NULL if unavailable.
    * @param  sht: Pointer to SHT40 sensor data, used only if bme is NULL.
    * @retval 1 on success, -1 on failure.
*/
int flash_log_append(uint32_t timestamp,
                     const bme280_data_t *bme,
                     const sht40_data_t *sht);

/*
    * @brief  Read a log record by index relative to the oldest stored record.
    * @param  index: Zero-based offset from the oldest record.
    * @param  rec: Pointer to the structure where the record will be stored.
    * @retval 1 if a valid record was read, -1 on failure or invalid CRC.
*/
int flash_log_read_oldest(uint32_t index, flash_log_record_t *rec);

/*
    * @brief  Read a log record by index relative to the most recent record.
    * @param  index: Zero-based offset from the latest record (0 = newest).
    * @param  rec: Pointer to the structure where the record will be stored.
    * @retval 1 if a valid record was read, -1 on failure or invalid CRC.
*/
int flash_log_read_latest(uint32_t index, flash_log_record_t *rec);

/*
    * @brief  Return the number of log records currently stored.
    * @retval The record count, or 0 if the mutex is unavailable.
*/
uint32_t flash_log_count(void);

/*
    * @brief  Return the maximum number of log records that can be stored.
    * @retval The total capacity of the flash log area.
*/
uint32_t flash_log_capacity(void);

#endif // FLASH_LOG_H