#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t btn_fired = 0;

ISR(INT0_vect) {
    btn_fired = 1; // ISR krótki: tylko flaga
}

static void int0_init_falling_edge_pullup(void) {
    DDRD &= ~(1 << PD2);      // PD2 input
    PORTD |= (1 << PD2);      // pull-up

    EICRA |= (1 << ISC01);    // falling edge: ISC01=1, ISC00=0
    EICRA &= ~(1 << ISC00);
    EIFR  |= (1 << INTF0);    // skasuj ewentualny pending
    EIMSK |= (1 << INT0);     // enable INT0
}

int main(void) {
    DDRB |= (1 << PB0);       // LED PB0
    int0_init_falling_edge_pullup();
    sei();

    while (1) {
        if (btn_fired) {
            btn_fired = 0;
            PORTB ^= (1 << PB0); // toggle LED
        }
    }
}