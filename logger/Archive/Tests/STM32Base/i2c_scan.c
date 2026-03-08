#include "stm32g0xx.h"
#include <stdint.h>

#define F_CPU_HZ 16000000UL

static volatile uint32_t g_ms = 0;

void SysTick_Handler(void) {
    g_ms++;
}

static void delay_ms(uint32_t ms) {
    uint32_t start = g_ms;
    while ((g_ms - start) < ms) { __NOP(); }
}

static void systick_init_1ms(void) {
    SysTick->LOAD = (F_CPU_HZ / 1000UL) - 1UL;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}

static int i2c1_probe_7bit(uint8_t addr7) {
    i2c1_clear_flags();

    // START z NBYTES=0 i AUTOEND: tylko sprawdzenie ACK adresu
    I2C1->CR2 = 0;
    I2C1->CR2 |= ((uint32_t)addr7 << 1);
    I2C1->CR2 |= (0u << I2C_CR2_NBYTES_Pos);
    I2C1->CR2 |= I2C_CR2_AUTOEND;
    I2C1->CR2 |= I2C_CR2_START;

    // Czekaj na STOPF albo NACKF
    while (!(I2C1->ISR & (I2C_ISR_STOPF | I2C_ISR_NACKF))) {}

    int ok = (I2C1->ISR & I2C_ISR_NACKF) ? 0 : 1;
    i2c1_clear_flags();
    return ok;
}

static void i2c1_scan_uart(void) {
    uart2_puts("I2C scan:\r\n");
    for (uint8_t a = 1; a < 127; a++) {
        if (i2c1_probe_7bit(a)) {
            uart2_puts("  found 0x");
            const char hex[] = "0123456789ABCDEF";
            uart2_putc(hex[(a >> 4) & 0xF]);
            uart2_putc(hex[a & 0xF]);
            uart2_puts("\r\n");
        }
    }
}

int main(void) {
    systick_init_1ms();

    while (1) {
        // Your main loop code here
    }
}