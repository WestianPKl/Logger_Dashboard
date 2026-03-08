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

static void i2c1_init_100k(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    RCC->APBENR1 |= RCC_APBENR1_I2C1EN;

    // PB6 SCL, PB7 SDA -> AF, open-drain, pull-up
    for (uint8_t pin = 6; pin <= 7; pin++) {
        GPIOB->MODER &= ~(3u << (pin*2));
        GPIOB->MODER |=  (2u << (pin*2)); // AF
        GPIOB->OTYPER |= (1u << pin);      // OD
        GPIOB->PUPDR  &= ~(3u << (pin*2));
        GPIOB->PUPDR  |=  (1u << (pin*2)); // PU
    }
    // AF6 dla I2C na wielu G0
    GPIOB->AFR[0] &= ~((0xFu << (6*4)) | (0xFu << (7*4)));
    GPIOB->AFR[0] |=  ((0x6u << (6*4)) | (0x6u << (7*4)));

    // Reset I2C
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->CR1 = 0;

    // TIMINGR: zależy od F_CPU_HZ i target freq.
    // WARTOŚĆ STARTOWA (często działa dla ~16MHz, 100kHz):
    I2C1->TIMINGR = 0x00303D5B;

    I2C1->CR1 |= I2C_CR1_PE;
}

static void i2c1_clear_flags(void) {
    // Wyczyść typowe flagi w ICR
    I2C1->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF | I2C_ICR_BERRCF | I2C_ICR_ARLOCF;
}

static int i2c1_write(uint8_t addr7, const uint8_t *buf, uint8_t len) {
    i2c1_clear_flags();

    I2C1->CR2 = 0;
    I2C1->CR2 |= ((uint32_t)addr7 << 1);
    I2C1->CR2 |= ((uint32_t)len << I2C_CR2_NBYTES_Pos);
    I2C1->CR2 |= I2C_CR2_AUTOEND;
    I2C1->CR2 |= I2C_CR2_START;

    for (uint8_t i = 0; i < len; i++) {
        while (!(I2C1->ISR & I2C_ISR_TXIS)) {
            if (I2C1->ISR & I2C_ISR_NACKF) { i2c1_clear_flags(); return 0; }
        }
        I2C1->TXDR = buf[i];
    }

    while (!(I2C1->ISR & I2C_ISR_STOPF)) {}
    int ok = (I2C1->ISR & I2C_ISR_NACKF) ? 0 : 1;
    i2c1_clear_flags();
    return ok;
}

static int i2c1_read(uint8_t addr7, uint8_t *buf, uint8_t len) {
    i2c1_clear_flags();

    I2C1->CR2 = 0;
    I2C1->CR2 |= ((uint32_t)addr7 << 1);
    I2C1->CR2 |= I2C_CR2_RD_WRN;
    I2C1->CR2 |= ((uint32_t)len << I2C_CR2_NBYTES_Pos);
    I2C1->CR2 |= I2C_CR2_AUTOEND;
    I2C1->CR2 |= I2C_CR2_START;

    for (uint8_t i = 0; i < len; i++) {
        while (!(I2C1->ISR & I2C_ISR_RXNE)) {
            if (I2C1->ISR & I2C_ISR_NACKF) { i2c1_clear_flags(); return 0; }
        }
        buf[i] = (uint8_t)I2C1->RXDR;
    }

    while (!(I2C1->ISR & I2C_ISR_STOPF)) {}
    int ok = (I2C1->ISR & I2C_ISR_NACKF) ? 0 : 1;
    i2c1_clear_flags();
    return ok;
}

int main(void) {
    systick_init_1ms();

    while (1) {
        // Your main loop code here
    }
}