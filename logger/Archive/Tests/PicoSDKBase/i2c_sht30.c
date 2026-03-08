#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

static void i2c0_init_100k(void) {
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);
}

int main() {
    stdio_init_all();
    sleep_ms(1000);

    i2c0_init_100k();

    while (true) {
        printf("I2C scan:\n");
        for (int addr = 1; addr < 127; addr++) {
            uint8_t dummy = 0;
            // i2c_read_blocking bez rejestru: próba ACK
            int ret = i2c_read_blocking(i2c0, addr, &dummy, 1, false);
            if (ret >= 0) {
                printf("  found: 0x%02X\n", addr);
            }
        }
        sleep_ms(3000);
    }
}