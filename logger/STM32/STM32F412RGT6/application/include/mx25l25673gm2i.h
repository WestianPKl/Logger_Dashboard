#ifndef MX25L25673GM2I_H
#define MX25L25673GM2I_H

#include "stm32f4xx.h"
#include <stdint.h>

#define MX25L25673GM2I_PAGE_SIZE        256u
#define MX25L25673GM2I_SECTOR_SIZE      4096u
#define MX25L25673GM2I_BLOCK32_SIZE     32768u
#define MX25L25673GM2I_BLOCK64_SIZE     65536u
#define MX25L25673GM2I_SIZE_BYTES       33554432u
#define MX25L25673GM2I_MAX_ADDR         0x01FFFFFFu

#define MX25_CMD_WREN                0x06u
#define MX25_CMD_WRDI                0x04u
#define MX25_CMD_RDSR                0x05u
#define MX25_CMD_WRSR                0x01u
#define MX25_CMD_READ                0x03u
#define MX25_CMD_FAST_READ           0x0Bu
#define MX25_CMD_PP                  0x02u
#define MX25_CMD_SE                  0x20u
#define MX25_CMD_BE32                0x52u
#define MX25_CMD_BE64                0xD8u
#define MX25_CMD_CE                  0xC7u
#define MX25_CMD_RDID                0x9Fu
#define MX25_CMD_RDCR                0x15u
#define MX25_CMD_RDSCUR              0x2Bu
#define MX25_CMD_EN4B                0xB7u
#define MX25_CMD_EX4B                0xE9u
#define MX25_CMD_RSTEN               0x66u
#define MX25_CMD_RST                 0x99u
#define MX25_CMD_DP                  0xB9u
#define MX25_CMD_RDP                 0xABu

#define MX25_SR_WIP                  0x01u
#define MX25_SR_WEL                  0x02u

typedef struct __attribute__((packed)) {
    int32_t  temp_c;
    uint32_t hum_pct;
    uint32_t press_hPa;
    uint8_t  year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
} mx25_log_record_t;

void mx25_gpio_init(void);

int mx25_reset(void);
int mx25_init(void);

int mx25_read_id(uint8_t id[3]);
int mx25_read_status(uint8_t *sr);
int mx25_read_config(uint8_t *cr);

int mx25_write_enable(void);
int mx25_wait_ready(uint32_t timeout);

int mx25_read(uint32_t addr, uint8_t *dst, uint32_t len);
int mx25_page_program(uint32_t addr, const uint8_t *src, uint16_t len);
int mx25_write(uint32_t addr, const uint8_t *src, uint32_t len);

int mx25_sector_erase_4k(uint32_t addr);
int mx25_block_erase_32k(uint32_t addr);
int mx25_block_erase_64k(uint32_t addr);
int mx25_chip_erase(void);

int mx25_log_write(uint32_t addr, const mx25_log_record_t *rec);
int mx25_log_read(uint32_t addr, mx25_log_record_t *rec);

#endif // MX25L25673GM2I_H