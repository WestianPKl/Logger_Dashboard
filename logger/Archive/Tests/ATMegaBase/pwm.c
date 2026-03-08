#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

static void pwm0a_init(void) {
    DDRD |= (1 << PD6); // OC0A output (D6)

    // Fast PWM, TOP=0xFF, non-inverting on OC0A
    TCCR0A = (1 << WGM01) | (1 << WGM00) | (1 << COM0A1);
    // Prescaler = 64
    TCCR0B = (1 << CS01) | (1 << CS00);

    OCR0A = 0; // start 0%
}

static void pwm0a_set(uint8_t duty) {
    OCR0A = duty;
}

int main(void) {
    pwm0a_init();

    while (1) {
        pwm0a_set(0);
        _delay_ms(1000);
        pwm0a_set(128);
        _delay_ms(1000);
        pwm0a_set(255);
        _delay_ms(1000);
    }
}