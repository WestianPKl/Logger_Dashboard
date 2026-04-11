#ifndef FM24CL16B_H
#define FM24CL16B_H

#include <stdint.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

#define FM24CL16B_I2C_ADDR_BASE   0x50
#define FM24CL16B_SIZE_BYTES      2048u
#define FM24CL16B_ADDR_MASK       0x07FFu

#define FRAM_ADDR_FLAGS           0x0005u

#define FRAM_ADDR_DEVICE_ID       0x0008u
#define FRAM_ADDR_SENSOR_ID       0x0028u
#define FRAM_ADDR_WRITE_INDEX     0x0048u
#define FRAM_ADDR_COUNT           0x0068u
#define FRAM_ADDR_NEXT_SEQUENCE   0x0088u
#define FRAM_ADDR_MEASURE_INTERVAL 0x0128u
#define FRAM_ADDR_LOG_META_A      0x0180u
#define FRAM_ADDR_LOG_META_B      0x01C0u

#define FRAM_FLAG_EXT_RTC_PRESENT  (1U << 0)
#define FRAM_FLAG_FLASH_PRESENT    (1U << 1)
#define FRAM_FLAG_DISPLAY_PRESENT  (1U << 2)
#define FRAM_FLAG_SHT40_PRESENT    (1U << 3)
#define FRAM_FLAG_BME280_PRESENT   (1U << 4)
#define FRAM_FLAG_INA226_PRESENT   (1U << 5)
#define FRAM_FLAG_ADC_PRESENT      (1U << 6)
#define FRAM_FLAG_CAN_PRESENT      (1U << 7)

#define FM24CL16B_DEV_ADDR(a) (((0x50 | (((a) >> 8) & 0x07))) << 1)

#define FM24CL16B_WORD_ADDR(a) \
    (uint8_t)((a) & 0xFF)

#define FRAM_MAX_DATA_LEN  16u

typedef enum {
    FRAM_CMD_READ = 0,
    FRAM_CMD_WRITE,
    FRAM_CMD_READ_FLAGS,
    FRAM_CMD_WRITE_FLAGS
} fram_cmd_t;

typedef struct {
    fram_cmd_t cmd;
    uint16_t addr;
    uint16_t len;
    uint8_t data[FRAM_MAX_DATA_LEN];
    TaskHandle_t replyTask;
    int status;
} fram_msg_t;

int fm24cl16b_write(uint16_t mem_addr, const uint8_t *src, uint16_t len);
int fm24cl16b_read(uint16_t mem_addr, uint8_t *dst, uint16_t len);

int fm24cl16b_write_byte(uint16_t mem_addr, uint8_t value);
int fm24cl16b_read_byte(uint16_t mem_addr, uint8_t *value);

int fm24cl16b_write_u32(uint16_t mem_addr, uint32_t value);
int fm24cl16b_read_u32(uint16_t mem_addr, uint32_t *value);

#endif