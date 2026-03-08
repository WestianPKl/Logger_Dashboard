#define F_CPU 16000000UL
#include <avr/io.h>

#define TWI_FREQ 100000UL

static void twi_init(void) {
    // SCL = F_CPU / (16 + 2*TWBR*prescaler)
    // prescaler = 1 (TWPS=0)
    TWSR = 0x00;
    TWBR = (uint8_t)((F_CPU / TWI_FREQ - 16) / 2);
    TWCR = (1 << TWEN);
}

static uint8_t twi_start(uint8_t addr_rw) {
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT))) {}

    uint8_t st = TWSR & 0xF8;
    if (st != 0x08 && st != 0x10) return 0; // START / REP START

    TWDR = addr_rw;
    TWCR = (1<<TWINT) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT))) {}

    st = TWSR & 0xF8;
    // SLA+W ACK: 0x18, SLA+R ACK: 0x40
    if (st != 0x18 && st != 0x40) return 0;

    return 1;
}

static void twi_stop(void) {
    TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}

static uint8_t twi_write(uint8_t data) {
    TWDR = data;
    TWCR = (1<<TWINT) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT))) {}

    uint8_t st = TWSR & 0xF8;
    return (st == 0x28); // DATA ACK
}

static uint8_t twi_read_ack(void) {
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    while (!(TWCR & (1<<TWINT))) {}
    return TWDR;
}

static uint8_t twi_read_nack(void) {
    TWCR = (1<<TWINT) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT))) {}
    return TWDR;
}