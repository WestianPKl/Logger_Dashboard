#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define SHT30_ADDR 0x44

// wklej tu twi_init(), twi_start(), twi_stop(), twi_write(), twi_read_ack(), twi_read_nack()

static uint8_t sht30_read_raw(uint16_t *t_raw, uint16_t *rh_raw) {
    // Write command
    if (!twi_start((SHT30_ADDR << 1) | 0)) { twi_stop(); return 0; }
    if (!twi_write(0x2C)) { twi_stop(); return 0; }
    if (!twi_write(0x06)) { twi_stop(); return 0; }
    twi_stop();

    _delay_ms(15);

    // Read 6 bytes
    if (!twi_start((SHT30_ADDR << 1) | 1)) { twi_stop(); return 0; }

    uint8_t t_msb = twi_read_ack();
    uint8_t t_lsb = twi_read_ack();
    (void)twi_read_ack(); // CRC T (ignore)

    uint8_t rh_msb = twi_read_ack();
    uint8_t rh_lsb = twi_read_ack();
    (void)twi_read_nack(); // CRC RH (ignore)
    twi_stop();

    *t_raw  = ((uint16_t)t_msb << 8) | t_lsb;
    *rh_raw = ((uint16_t)rh_msb << 8) | rh_lsb;
    return 1;
}

// (opcjonalnie) konwersja do fixed-point bez float:
// T_x100 = -4500 + 17500*raw/65535
// RH_x100 = 10000*raw/65535
static int16_t sht30_temp_c_x100(uint16_t raw) {
    int32_t v = -4500L + (17500L * (int32_t)raw) / 65535L;
    return (int16_t)v;
}
static uint16_t sht30_rh_x100(uint16_t raw) {
    uint32_t v = (10000UL * (uint32_t)raw) / 65535UL;
    return (uint16_t)v;
}

int main(void) {
    twi_init();

    // (opcjonalnie) dołóż uart0 i wypisuj wyniki
    while (1) {
        uint16_t tr, rr;
        if (sht30_read_raw(&tr, &rr)) {
            int16_t  t100  = sht30_temp_c_x100(tr);
            uint16_t rh100 = sht30_rh_x100(rr);

            // tu np. wyślij po UART: t100/100, t100%100, rh100/100...
            (void)t100;
            (void)rh100;
        }
        _delay_ms(1000);
    }
}