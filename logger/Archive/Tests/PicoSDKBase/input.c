#include "pico/stdlib.h"

static volatile bool fired = false;

static void gpio_irq_cb(uint gpio, uint32_t events) {
    if (gpio == 15 && (events & GPIO_IRQ_EDGE_FALL)) {
        fired = true;
    }
}

int main() {
    stdio_init_all();

    const uint LED = 25;
    const uint BTN = 15;

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);

    gpio_init(BTN);
    gpio_set_dir(BTN, GPIO_IN);
    gpio_pull_up(BTN); // przycisk do GND

    gpio_set_irq_enabled_with_callback(BTN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_cb);

    while (true) {
        if (fired) {
            fired = false;
            gpio_put(LED, !gpio_get(LED));
        }
        tight_loop_contents();
    }
}