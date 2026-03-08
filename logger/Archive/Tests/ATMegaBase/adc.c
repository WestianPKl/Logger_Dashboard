#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

static void adc_init_avcc(void) {
    ADMUX  = (1 << REFS0); // AVcc ref, channel=0
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // /128
}

static uint16_t adc_read(uint8_t ch) {
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F); // select channel 0..7
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC)) {}
    return ADC; // 10-bit
}

int main(void) {
    adc_init_avcc();

    DDRB |= (1 << PB0);

    while (1) {
        uint16_t v = adc_read(0); // ADC0
        if (v > 512) PORTB |=  (1 << PB0);
        else         PORTB &= ~(1 << PB0);
        _delay_ms(10);
    }
}