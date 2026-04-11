#include "mx25l25673gm2i.h"
#include "spi.h"
#include "FreeRTOS.h"
#include "task.h"

#define SPI_XFER_TIMEOUT_MS 100U

static int spi1_xfer8(uint8_t tx, uint8_t *rx)
{
    uint8_t rx_local = 0U;

    spiOwnerTask = xTaskGetCurrentTaskHandle();
    spiError = 0U;
    xTaskNotifyStateClear(NULL);
    (void)ulTaskNotifyTake(pdTRUE, 0);

    if (HAL_SPI_TransmitReceive_DMA(&hspi1, &tx, &rx_local, 1U) != HAL_OK) {
        spiOwnerTask = NULL;
        return -1;
    }

    if (wait_spi_done(pdMS_TO_TICKS(SPI_XFER_TIMEOUT_MS)) != 1) {
        (void)HAL_SPI_Abort(&hspi1);
        spiOwnerTask = NULL;
        return -1;
    }

    spiOwnerTask = NULL;

    if (rx != NULL) {
        *rx = rx_local;
    }

    return 1;
}

static inline void mx25_select(void)
{
    spi1_cs2_low();
}

static inline void mx25_deselect(void)
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
    uint8_t tmp = 0U;

    if (sr == NULL) return -1;

    mx25_select();

    if (spi1_xfer8(MX25_CMD_RDSR, NULL) != 1) {
        mx25_deselect();
        return -1;
    }

    if (spi1_xfer8(0xFFU, &tmp) != 1) {
        mx25_deselect();
        return -1;
    }

    mx25_deselect();
    *sr = tmp;
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
    TickType_t start = xTaskGetTickCount();

    do {
        if (mx25_read_status(&sr) != 1) return -1;
        if ((sr & MX25_SR_WIP) == 0U) return 1;
        vTaskDelay(pdMS_TO_TICKS(1U));
    } while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(timeout_ms));

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

    vTaskDelay(pdMS_TO_TICKS(1U));
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

    if (spi1_xfer8(0xFFU, &id[0]) != 1) {
        mx25_deselect();
        return -1;
    }
    if (spi1_xfer8(0xFFU, &id[1]) != 1) {
        mx25_deselect();
        return -1;
    }
    if (spi1_xfer8(0xFFU, &id[2]) != 1) {
        mx25_deselect();
        return -1;
    }

    mx25_deselect();
    return 1;
}

static int mx25_page_program(uint32_t addr, const uint8_t *src, uint16_t len)
{
    if ((src == NULL) || (len == 0U)) return -1;
    if (len > MX25_PAGE_SIZE) return -1;
    if (addr > MX25_MAX_ADDR) return -1;
    if ((uint32_t)(addr & 0xFFU) + len > MX25_PAGE_SIZE) return -1;
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

    for (uint16_t i = 0; i < len; i++) {
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

    for (uint32_t i = 0; i < len; i++) {
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
    if ((src == NULL) || (len == 0U)) return -1;
    if (addr > MX25_MAX_ADDR) return -1;
    if ((addr + len) > MX25_SIZE_BYTES) return -1;

    while (len > 0U) {
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

int mx25_chip_erase_start(void)
{
    if (mx25_write_enable() != 1) return -1;

    mx25_select();

    if (spi1_xfer8(MX25_CMD_CE, NULL) != 1) {
        mx25_deselect();
        return -1;
    }

    mx25_deselect();
    return 1;
}

int mx25_is_ready(void)
{
    uint8_t sr = 0U;

    if (mx25_read_status(&sr) != 1) return -1;
    return ((sr & MX25_SR_WIP) == 0U) ? 1 : 0;
}