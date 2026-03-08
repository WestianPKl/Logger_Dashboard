#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define SPI_HOST  SPI2_HOST
#define SPI_SCLK  18
#define SPI_MOSI  23
#define SPI_MISO  19
#define SPI_CS    5

void app_main(void) {
    spi_bus_config_t buscfg = {
        .sclk_io_num = SPI_SCLK,
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = SPI_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 64
    };
    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = SPI_CS,
        .queue_size = 1
    };

    spi_device_handle_t dev;
    spi_bus_add_device(SPI_HOST, &devcfg, &dev);

    uint8_t tx = 0x9F;
    uint8_t rx = 0x00;

    while (1) {
        spi_transaction_t t = {0};
        t.length = 8;
        t.tx_buffer = &tx;
        t.rx_buffer = &rx;

        spi_device_transmit(dev, &t);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}