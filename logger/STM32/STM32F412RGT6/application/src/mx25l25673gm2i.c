#include "mx25l25673gm2i.h"
#include "spi.h"
#include "systick.h"

// FLASH CS = PC12
// FLASH HOLD = PC13

#define MX25_CS_PORT        GPIOC
#define MX25_CS_PIN         12U
#define MX25_HOLD_PORT      GPIOC
#define MX25_HOLD_PIN       13U

static void mx25_cs_high(void)
{
    MX25_CS_PORT->BSRR = (1U << MX25_CS_PIN);
}

static void mx25_cs_low(void)
{
    MX25_CS_PORT->BSRR = (1U << (MX25_CS_PIN + 16U));
}

static void mx25_hold_high(void)
{
    MX25_HOLD_PORT->BSRR = (1U << MX25_HOLD_PIN);
}

static uint8_t spi1_xfer8(uint8_t tx)
{
    while ((SPI1->SR & SPI_SR_TXE) == 0U) {}
    *((__IO uint8_t *)&SPI1->DR) = tx;

    while ((SPI1->SR & SPI_SR_RXNE) == 0U) {}
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

static void mx25_send_addr32(uint32_t addr)
{
    spi1_xfer8((uint8_t)(addr >> 24));
    spi1_xfer8((uint8_t)(addr >> 16));
    spi1_xfer8((uint8_t)(addr >> 8));
    spi1_xfer8((uint8_t)(addr));
}

void mx25_gpio_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    (void)RCC->AHB1ENR;

    MX25_CS_PORT->MODER &= ~(3U << (MX25_CS_PIN * 2U));
    MX25_CS_PORT->MODER |=  (1U << (MX25_CS_PIN * 2U));
    MX25_CS_PORT->OTYPER &= ~(1U << MX25_CS_PIN);
    MX25_CS_PORT->OSPEEDR |= (3U << (MX25_CS_PIN * 2U));
    MX25_CS_PORT->PUPDR &= ~(3U << (MX25_CS_PIN * 2U));
    mx25_cs_high();

    MX25_HOLD_PORT->MODER &= ~(3U << (MX25_HOLD_PIN * 2U));
    MX25_HOLD_PORT->MODER |=  (1U << (MX25_HOLD_PIN * 2U));
    MX25_HOLD_PORT->OTYPER &= ~(1U << MX25_HOLD_PIN);
    MX25_HOLD_PORT->OSPEEDR |= (3U << (MX25_HOLD_PIN * 2U));
    MX25_HOLD_PORT->PUPDR &= ~(3U << (MX25_HOLD_PIN * 2U));
    mx25_hold_high();
}

int mx25_read_status(uint8_t *sr)
{
    if (!sr) return -1;

    spi1_flush_rx();
    mx25_cs_low();
    spi1_xfer8(MX25_CMD_RDSR);
    *sr = spi1_xfer8(0x00);
    mx25_cs_high();

    return 1;
}

int mx25_read_config(uint8_t *cr)
{
    if (!cr) return -1;

    spi1_flush_rx();
    mx25_cs_low();
    spi1_xfer8(MX25_CMD_RDCR);
    *cr = spi1_xfer8(0x00);
    mx25_cs_high();

    return 1;
}

int mx25_read_id(uint8_t id[3])
{
    if (!id) return -1;

    spi1_flush_rx();
    mx25_cs_low();
    spi1_xfer8(MX25_CMD_RDID);
    id[0] = spi1_xfer8(0x00);
    id[1] = spi1_xfer8(0x00);
    id[2] = spi1_xfer8(0x00);
    mx25_cs_high();

    return 1;
}

int mx25_write_enable(void)
{
    uint8_t sr = 0;

    spi1_flush_rx();
    mx25_cs_low();
    spi1_xfer8(MX25_CMD_WREN);
    mx25_cs_high();

    if (mx25_read_status(&sr) != 1) return -1;
    return (sr & MX25_SR_WEL) ? 1 : -1;
}

int mx25_wait_ready(uint32_t timeout)
{
    uint8_t sr = 0;

    while (timeout--) {
        if (mx25_read_status(&sr) != 1) return -1;
        if ((sr & MX25_SR_WIP) == 0U) return 1;
    }

    return -1;
}

int mx25_reset(void)
{
    spi1_flush_rx();
    mx25_cs_low();
    spi1_xfer8(MX25_CMD_RSTEN);
    mx25_cs_high();

    spi1_flush_rx();
    mx25_cs_low();
    spi1_xfer8(MX25_CMD_RST);
    mx25_cs_high();

    systick_delay_ms(1);
    return 1;
}

int mx25_init(void)
{
    uint8_t id[3];

    mx25_gpio_init();

    if (mx25_reset() != 1) return -1;

    if (mx25_read_id(id) != 1) return -1;

    if (id[0] != 0xC2U) return -1;

    spi1_flush_rx();
    mx25_cs_low();
    spi1_xfer8(MX25_CMD_EN4B);
    mx25_cs_high();

    return 1;
}

int mx25_read(uint32_t addr, uint8_t *dst, uint32_t len)
{
    if (!dst || !len) return -1;
    if (addr > MX25L25673GM2I_MAX_ADDR) return -1;
    if ((addr + len) > MX25L25673GM2I_SIZE_BYTES) return -1;

    spi1_flush_rx();
    mx25_cs_low();

    spi1_xfer8(MX25_CMD_READ);
    mx25_send_addr32(addr);

    for (uint32_t i = 0; i < len; i++) {
        dst[i] = spi1_xfer8(0x00);
    }

    mx25_cs_high();
    return 1;
}

int mx25_page_program(uint32_t addr, const uint8_t *src, uint16_t len)
{
    if (!src || !len) return -1;
    if (len > MX25L25673GM2I_PAGE_SIZE) return -1;
    if (addr > MX25L25673GM2I_MAX_ADDR) return -1;
    if ((uint32_t)(addr & 0xFFU) + len > MX25L25673GM2I_PAGE_SIZE) return -1;
    if ((addr + len) > MX25L25673GM2I_SIZE_BYTES) return -1;

    if (mx25_write_enable() != 1) return -1;

    spi1_flush_rx();
    mx25_cs_low();

    spi1_xfer8(MX25_CMD_PP);
    mx25_send_addr32(addr);

    for (uint16_t i = 0; i < len; i++) {
        spi1_xfer8(src[i]);
    }

    mx25_cs_high();

    return mx25_wait_ready(2000000U);
}

int mx25_write(uint32_t addr, const uint8_t *src, uint32_t len)
{
    if (!src || !len) return -1;
    if (addr > MX25L25673GM2I_MAX_ADDR) return -1;
    if ((addr + len) > MX25L25673GM2I_SIZE_BYTES) return -1;

    while (len) {
        uint16_t chunk = (uint16_t)(MX25L25673GM2I_PAGE_SIZE - (addr & 0xFFU));
        if (chunk > len) chunk = (uint16_t)len;

        if (mx25_page_program(addr, src, chunk) != 1) return -1;

        addr += chunk;
        src  += chunk;
        len  -= chunk;
    }

    return 1;
}

static int mx25_erase_common(uint8_t cmd, uint32_t addr, uint32_t timeout)
{
    if (addr > MX25L25673GM2I_MAX_ADDR) return -1;

    if (mx25_write_enable() != 1) return -1;

    spi1_flush_rx();
    mx25_cs_low();

    spi1_xfer8(cmd);
    mx25_send_addr32(addr);

    mx25_cs_high();

    return mx25_wait_ready(timeout);
}

int mx25_sector_erase_4k(uint32_t addr)
{
    addr &= ~(uint32_t)(MX25L25673GM2I_SECTOR_SIZE - 1U);
    return mx25_erase_common(MX25_CMD_SE, addr, 50000000U);
}

int mx25_block_erase_32k(uint32_t addr)
{
    addr &= ~(uint32_t)(MX25L25673GM2I_BLOCK32_SIZE - 1U);
    return mx25_erase_common(MX25_CMD_BE32, addr, 150000000U);
}

int mx25_block_erase_64k(uint32_t addr)
{
    addr &= ~(uint32_t)(MX25L25673GM2I_BLOCK64_SIZE - 1U);
    return mx25_erase_common(MX25_CMD_BE64, addr, 300000000U);
}

int mx25_chip_erase(void)
{
    if (mx25_write_enable() != 1) return -1;

    spi1_flush_rx();
    mx25_cs_low();
    spi1_xfer8(MX25_CMD_CE);
    mx25_cs_high();

    return mx25_wait_ready(2000000000U);
}

int mx25_log_read(uint32_t addr, mx25_log_record_t *rec)
{
    if (!rec) return -1;
    return mx25_read(addr, (uint8_t *)rec, sizeof(mx25_log_record_t));
}

int mx25_log_write(uint32_t addr, const mx25_log_record_t *rec)
{
    if (!rec) return -1;
    return mx25_write(addr, (const uint8_t *)rec, sizeof(mx25_log_record_t));
}