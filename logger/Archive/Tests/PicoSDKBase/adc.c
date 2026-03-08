#include "pico/stdlib.h"
#include "hardware/adc.h"

int main() {
    stdio_init_all();

    adc_init();
    adc_gpio_init(26);      // GPIO26 -> ADC0
    adc_select_input(0);    // kanał ADC0

    while (true) {
        uint16_t raw = adc_read(); // 12-bit: 0..4095
        // np. prosto: mrugaj LED zależnie od progu
        sleep_ms(200);
        (void)raw;
    }
}