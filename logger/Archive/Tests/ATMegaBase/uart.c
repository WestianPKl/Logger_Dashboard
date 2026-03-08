#define F_CPU 16000000UL
#include <avr/io.h>

#define BAUD 115200UL
#define UBRR_VAL ((F_CPU / (16UL * BAUD)) - 1)

static void uart0_init(void) {
    UBRR0H = (uint8_t)(UBRR_VAL >> 8);
    UBRR0L = (uint8_t)(UBRR_VAL);
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
}

static void uart0_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0))) {}
    UDR0 = c;
}

static void uart0_puts(const char *s) {
    while (*s) uart0_putc(*s++);
}

static uint8_t uart0_available(void) {
    return (UCSR0A & (1 << RXC0));
}

static char uart0_getc(void) {
    while (!uart0_available()) {}
    return UDR0;
}

int main(void) {
    uart0_init();
    uart0_puts("UART ready\r\n");

    while (1) {
        if (uart0_available()) {
            char c = uart0_getc();
            uart0_putc(c); // echo
        }
    }
}