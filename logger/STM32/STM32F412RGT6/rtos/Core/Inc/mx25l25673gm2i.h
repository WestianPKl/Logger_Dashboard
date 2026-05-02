#ifndef MX25L25673GM2I_H
#define MX25L25673GM2I_H

#include <stdint.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

#define MX25_PAGE_SIZE        256u
#define MX25_SECTOR_SIZE      4096u
#define MX25_SIZE_BYTES       33554432u
#define MX25_MAX_ADDR         0x01FFFFFFu

#define MX25_CMD_WREN         0x06u
#define MX25_CMD_RDSR         0x05u
#define MX25_CMD_READ         0x03u
#define MX25_CMD_PP           0x02u
#define MX25_CMD_SE           0x20u
#define MX25_CMD_RDID         0x9Fu
#define MX25_CMD_EN4B         0xB7u
#define MX25_CMD_RSTEN        0x66u
#define MX25_CMD_RST          0x99u
#define MX25_CMD_CE           0xC7u

#define MX25_SR_WIP           0x01u
#define MX25_SR_WEL           0x02u

/*
    * @brief  Initialize the MX25L25673G flash: reset, verify ID, and enable 4-byte addressing.
    * @retval 1 on success, -1 on failure.
*/
int mx25_init(void);

/*
    * @brief  Read data from the flash at the given 32-bit address.
    * @param  addr: Start address (0 .. MX25_MAX_ADDR).
    * @param  dst: Pointer to the destination buffer.
    * @param  len: Number of bytes to read.
    * @retval 1 on success, -1 on failure.
*/
int mx25_read(uint32_t addr, uint8_t *dst, uint32_t len);

/*
    * @brief  Write data to the flash, splitting across page boundaries automatically.
    * @param  addr: Start address (0 .. MX25_MAX_ADDR).
    * @param  src: Pointer to the source data.
    * @param  len: Number of bytes to write.
    * @retval 1 on success, -1 on failure.
*/
int mx25_write(uint32_t addr, const uint8_t *src, uint32_t len);

/*
    * @brief  Erase a 4 KB sector that contains the given address. Blocks until complete.
    * @param  addr: Any address within the target sector.
    * @retval 1 on success, -1 on failure.
*/
int mx25_sector_erase_4k(uint32_t addr);

/*
    * @brief  Start a full chip erase. Returns immediately; use mx25_is_ready() to poll completion.
    * @retval 1 on success, -1 on failure.
*/
int mx25_chip_erase_start(void);

/*
    * @brief  Check if the flash is ready (no write/erase operation in progress).
    * @retval 1 if ready, 0 if busy, -1 on communication error.
*/
int mx25_is_ready(void);

#endif