#include "mx25l25673gm2i.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "support.h"
#include "spi.h"

#define SPI_POLL_TIMEOUT 100U

extern SPI_HandleTypeDef hspi1;

static int spi1_xfer8(uint8_t tx, uint8_t *rx)
{
    uint8_t txb = tx;
    uint8_t rxb = 0U;

    if (HAL_SPI_TransmitReceive(&hspi1, &txb, &rxb, 1U, SPI_POLL_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (rx != NULL) {
        *rx = rxb;
    }

    return 1;
}

static void mx25_select(void)
{
    spi1_cs2_low();
}

static void mx25_deselect(void)
{
    spi1_cs2_high();
}

static int mx25_send_addr32(uint32_t addr)
{
    if (spi1_xfer8((uint8_t)(addr >> 24), NULL) != 1) return -1;
    if (spi1_xfer8((uint8_t)(addr >> 16), NULL) != 1) return -1;
    if (spi1_xfer8((uint8_t)(addr >> 8),  NULL) != 1) return -1;
    if (spi1_xfer8((uint8_t)(addr),       NULL) != 1) return -1;
    return 1;
}

static int mx25_read_status(uint8_t *sr)
{
    if (sr == NULL) return -1;

    mx25_select();

    if (spi1_xfer8(MX25_CMD_RDSR, NULL) != 1) {
        mx25_deselect();
        return -1;
    }

    if (spi1_xfer8(0xFFU, sr) != 1) {
        mx25_deselect();
        return -1;
    }

    mx25_deselect();
    return 1;
}

static int mx25_write_enable(void)
{
    uint8_t sr = 0U;

    mx25_select();
    if (spi1_xfer8(MX25_CMD_WREN, NULL) != 1) {
        mx25_deselect();
        return -1;
    }
    mx25_deselect();

    if (mx25_read_status(&sr) != 1) return -1;
    return ((sr & MX25_SR_WEL) != 0U) ? 1 : -1;
}

static int mx25_wait_ready(uint32_t timeout_ms)
{
    uint8_t sr = 0U;
    uint32_t start = HAL_GetTick();

    do {
        if (mx25_read_status(&sr) != 1) return -1;
        if ((sr & MX25_SR_WIP) == 0U) return 1;
    } while ((HAL_GetTick() - start) < timeout_ms);

    return -1;
}

static int mx25_reset(void)
{
    mx25_select();
    if (spi1_xfer8(MX25_CMD_RSTEN, NULL) != 1) {
        mx25_deselect();
        return -1;
    }
    mx25_deselect();

    mx25_select();
    if (spi1_xfer8(MX25_CMD_RST, NULL) != 1) {
        mx25_deselect();
        return -1;
    }
    mx25_deselect();

    HAL_Delay(1U);
    return 1;
}

static int mx25_read_id(uint8_t id[3])
{
    if (id == NULL) return -1;

    mx25_select();

    if (spi1_xfer8(MX25_CMD_RDID, NULL) != 1) {
        mx25_deselect();
        return -1;
    }

    if (spi1_xfer8(0xFFU, &id[0]) != 1) { mx25_deselect(); return -1; }
    if (spi1_xfer8(0xFFU, &id[1]) != 1) { mx25_deselect(); return -1; }
    if (spi1_xfer8(0xFFU, &id[2]) != 1) { mx25_deselect(); return -1; }

    mx25_deselect();
    return 1;
}

static int mx25_page_program(uint32_t addr, const uint8_t *src, uint16_t len)
{
    if ((src == NULL) || (len == 0U)) return -1;
    if (len > MX25_PAGE_SIZE) return -1;
    if (addr > MX25_MAX_ADDR) return -1;
    if (((uint32_t)(addr & 0xFFU) + len) > MX25_PAGE_SIZE) return -1;
    if ((addr + len) > MX25_SIZE_BYTES) return -1;

    if (mx25_write_enable() != 1) return -1;

    mx25_select();

    if (spi1_xfer8(MX25_CMD_PP, NULL) != 1) {
        mx25_deselect();
        return -1;
    }

    if (mx25_send_addr32(addr) != 1) {
        mx25_deselect();
        return -1;
    }

    for (uint16_t i = 0U; i < len; i++) {
        if (spi1_xfer8(src[i], NULL) != 1) {
            mx25_deselect();
            return -1;
        }
    }

    mx25_deselect();
    return mx25_wait_ready(10U);
}

int mx25_init(void)
{
    uint8_t id[3] = {0};

    HAL_GPIO_WritePin(FLASH_Hold_GPIO_Port, FLASH_Hold_Pin, GPIO_PIN_SET);

    if (mx25_reset() != 1) return -1;
    if (mx25_read_id(id) != 1) return -1;

    if (id[0] != 0xC2U) return -1;

    mx25_select();
    if (spi1_xfer8(MX25_CMD_EN4B, NULL) != 1) {
        mx25_deselect();
        return -1;
    }
    mx25_deselect();

    return 1;
}

int mx25_read(uint32_t addr, uint8_t *dst, uint32_t len)
{
    if ((dst == NULL) || (len == 0U)) return -1;
    if (addr > MX25_MAX_ADDR) return -1;
    if ((addr + len) > MX25_SIZE_BYTES) return -1;

    mx25_select();

    if (spi1_xfer8(MX25_CMD_READ, NULL) != 1) {
        mx25_deselect();
        return -1;
    }

    if (mx25_send_addr32(addr) != 1) {
        mx25_deselect();
        return -1;
    }

    for (uint32_t i = 0U; i < len; i++) {
        if (spi1_xfer8(0xFFU, &dst[i]) != 1) {
            mx25_deselect();
            return -1;
        }
    }

    mx25_deselect();
    return 1;
}

int mx25_write(uint32_t addr, const uint8_t *src, uint32_t len)
{
    while (len != 0U) {
        uint16_t chunk;

        if ((src == NULL) || (addr > MX25_MAX_ADDR) || ((addr + len) > MX25_SIZE_BYTES)) {
            return -1;
        }

        chunk = (uint16_t)(MX25_PAGE_SIZE - (addr & 0xFFU));
        if (chunk > len) {
            chunk = (uint16_t)len;
        }

        if (mx25_page_program(addr, src, chunk) != 1) {
            return -1;
        }

        addr += chunk;
        src  += chunk;
        len  -= chunk;
    }

    return 1;
}

int mx25_sector_erase_4k(uint32_t addr)
{
    addr &= ~(uint32_t)(MX25_SECTOR_SIZE - 1U);

    if (addr > MX25_MAX_ADDR) return -1;
    if (mx25_write_enable() != 1) return -1;

    mx25_select();

    if (spi1_xfer8(MX25_CMD_SE, NULL) != 1) {
        mx25_deselect();
        return -1;
    }

    if (mx25_send_addr32(addr) != 1) {
        mx25_deselect();
        return -1;
    }

    mx25_deselect();
    return mx25_wait_ready(500U);
}