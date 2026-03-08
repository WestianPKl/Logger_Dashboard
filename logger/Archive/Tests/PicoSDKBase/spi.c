#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

int main() {
    const uint CS   = 17;
    const uint SCK  = 18;
    const uint MOSI = 19;
    const uint MISO = 16;

    spi_init(spi0, 1 * 1000 * 1000); // 1 MHz
    gpio_set_function(SCK,  GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);
    gpio_set_function(MISO, GPIO_FUNC_SPI);

    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);
    gpio_put(CS, 1);

    // Mode0 jest domyślny, ale pokażmy jawnie:
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    uint8_t tx = 0x9F;
    uint8_t rx = 0;

    while (true) {
        gpio_put(CS, 0);
        spi_write_read_blocking(spi0, &tx, &rx, 1);
        gpio_put(CS, 1);

        sleep_ms(1000);
    }
}