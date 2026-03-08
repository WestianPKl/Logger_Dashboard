#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

// wklej tu twi_init(), twi_start(), twi_stop() z poprzedniej sekcji
// + (opcjonalnie) uart0_puts/uarto0_puthex jeśli chcesz log

static uint8_t i2c_probe(uint8_t addr7) {
    uint8_t ok = twi_start((addr7 << 1) | 0); // write
    twi_stop();
    return ok;
}

int main(void) {
    twi_init();

    // jeśli chcesz, dołóż uart0 i wypisuj znalezione adresy
    while (1) {
        for (uint8_t a = 1; a < 127; a++) {
            if (i2c_probe(a)) {
                // FOUND: a
                // np. mrugnięcie LED, albo wysłanie po UART
            }
            _delay_ms(2);
        }
        _delay_ms(3000);
    }
}