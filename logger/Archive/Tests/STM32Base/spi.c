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

static void spi1_init_master_mode0_8bit(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;
    RCC->APBENR2 |= RCC_APBENR2_SPI1EN;

    // CS PB2 jako output
    GPIOB->MODER &= ~(3u << (2*2));
    GPIOB->MODER |=  (1u << (2*2));
    GPIOB->BSRR = (1u << 2); // CS high

    // PA5/6/7 AF0 (SPI1) - często AF0
    for (uint8_t pin = 5; pin <= 7; pin++) {
        GPIOA->MODER &= ~(3u << (pin*2));
        GPIOA->MODER |=  (2u << (pin*2));
        GPIOA->AFR[0] &= ~(0xFu << (pin*4));
        GPIOA->AFR[0] |=  (0x0u << (pin*4)); // AF0
    }

    SPI1->CR1 = 0;
    SPI1->CR2 = 0;

    // 8-bit data size (DS=7 -> 8-bit) w SPIv2
    SPI1->CR2 |= (7u << SPI_CR2_DS_Pos);
    SPI1->CR2 |= SPI_CR2_FRXTH; // RXNE when 8-bit

    // Master, SSM+SSI, BR prescaler, mode0
    SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
    SPI1->CR1 |= (3u << SPI_CR1_BR_Pos); // fPCLK/16 (dobierz)
    SPI1->CR1 |= SPI_CR1_SPE;
}

static inline void spi_cs_low(void)  { GPIOB->BSRR = (1u << (2 + 16)); }
static inline void spi_cs_high(void) { GPIOB->BSRR = (1u << 2); }

static uint8_t spi1_xfer(uint8_t b) {
    while (!(SPI1->SR & SPI_SR_TXE)) {}
    *((volatile uint8_t *)&SPI1->DR) = b;
    while (!(SPI1->SR & SPI_SR_RXNE)) {}
    return *((volatile uint8_t *)&SPI1->DR);
}

int main(void) {
    systick_init_1ms();

    while (1) {
        // Your main loop code here
    }
}