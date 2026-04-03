#include "mx25l25673gm2i.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "support.h"
#include "spi.h"

#define SPI_POLL_TIMEOUT 100000U

static uint8_t spi1_xfer8(uint8_t tx)
{
    uint32_t t;

    t = SPI_POLL_TIMEOUT;
    while ((SPI1->SR & SPI_SR_TXE) == 0U) { if (--t == 0U) break; }
    *((__IO uint8_t *)&SPI1->DR) = tx;

    t = SPI_POLL_TIMEOUT;
    while ((SPI1->SR & SPI_SR_RXNE) == 0U) { if (--t == 0U) break; }
    return *((__IO uint8_t *)&SPI1->DR);
}

static void spi1_flush_rx(void)
{
    volatile uint8_t dummy;
    while (SPI1->SR & SPI_SR_RXNE) {
        dummy = *((__IO uint8_t *)&SPI1->DR);
        (void)dummy;
    }
}

static void mx25_select(void)
{
    spi1_flush_rx();
    spi1_cs2_low();
}

static void mx25_deselect(void)
{
    uint32_t t = SPI_POLL_TIMEOUT;
    while ((SPI1->SR & SPI_SR_BSY) && --t) {}
    spi1_flush_rx();
    spi1_cs2_high();
}

static void mx25_send_addr32(uint32_t addr)
{
    spi1_xfer8((uint8_t)(addr >> 24));
    spi1_xfer8((uint8_t)(addr >> 16));
    spi1_xfer8((uint8_t)(addr >> 8));
    spi1_xfer8((uint8_t)(addr));
}

static int mx25_read_status(uint8_t *sr)
{
    if (!sr) return -1;

    mx25_select();
    spi1_xfer8(MX25_CMD_RDSR);
    *sr = spi1_xfer8(0xFF);
    mx25_deselect();

    return 1;
}

static int mx25_write_enable(void)
{
    uint8_t sr;

    mx25_select();
    spi1_xfer8(MX25_CMD_WREN);
    mx25_deselect();

    if (mx25_read_status(&sr) != 1) return -1;
    return ((sr & MX25_SR_WEL) != 0U) ? 1 : -1;
}

static int mx25_wait_ready(uint32_t timeout_ms)
{
    uint8_t sr;
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
    spi1_xfer8(MX25_CMD_RSTEN);
    mx25_deselect();

    mx25_select();
    spi1_xfer8(MX25_CMD_RST);
    mx25_deselect();

    delay_ms(1);
    return 1;
}

static int mx25_read_id(uint8_t id[3])
{
    if (!id) return -1;

    mx25_select();
    spi1_xfer8(MX25_CMD_RDID);
    id[0] = spi1_xfer8(0xFF);
    id[1] = spi1_xfer8(0xFF);
    id[2] = spi1_xfer8(0xFF);
    mx25_deselect();

    return 1;
}

static int mx25_page_program(uint32_t addr, const uint8_t *src, uint16_t len)
{
    if (!src || !len) return -1;
    if (len > MX25_PAGE_SIZE) return -1;
    if (addr > MX25_MAX_ADDR) return -1;
    if ((uint32_t)(addr & 0xFFU) + len > MX25_PAGE_SIZE) return -1;
    if ((addr + len) > MX25_SIZE_BYTES) return -1;

    if (mx25_write_enable() != 1) return -1;

    mx25_select();
    spi1_xfer8(MX25_CMD_PP);
    mx25_send_addr32(addr);

    for (uint16_t i = 0; i < len; i++) {
        spi1_xfer8(src[i]);
    }

    mx25_deselect();

    return mx25_wait_ready(10U);
}

int mx25_init(void)
{
    uint8_t id[3];

    LL_GPIO_SetOutputPin(FLASH_Hold_GPIO_Port, FLASH_Hold_Pin);

    if (mx25_reset() != 1) return -1;
    if (mx25_read_id(id) != 1) return -1;

    if (id[0] != 0xC2U) return -1;

    mx25_select();
    spi1_xfer8(MX25_CMD_EN4B);
    mx25_deselect();

    return 1;
}

int mx25_read(uint32_t addr, uint8_t *dst, uint32_t len)
{
    if (!dst || !len) return -1;
    if (addr > MX25_MAX_ADDR) return -1;
    if ((addr + len) > MX25_SIZE_BYTES) return -1;

    mx25_select();
    spi1_xfer8(MX25_CMD_READ);
    mx25_send_addr32(addr);

    for (uint32_t i = 0; i < len; i++) {
        dst[i] = spi1_xfer8(0xFF);
    }

    mx25_deselect();
    return 1;
}

int mx25_write(uint32_t addr, const uint8_t *src, uint32_t len)
{
    if (!src || !len) return -1;
    if (addr > MX25_MAX_ADDR) return -1;
    if ((addr + len) > MX25_SIZE_BYTES) return -1;

    while (len) {
        uint16_t chunk = (uint16_t)(MX25_PAGE_SIZE - (addr & 0xFFU));
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
    spi1_xfer8(MX25_CMD_SE);
    mx25_send_addr32(addr);
    mx25_deselect();

    return mx25_wait_ready(500U);
}