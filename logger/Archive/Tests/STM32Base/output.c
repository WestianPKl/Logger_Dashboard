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

int main(void) {
    systick_init_1ms();

    while (1) {
        // Your main loop code here

        static void led_init_pb0(void) {
            RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

            // PB0 output
            GPIOB->MODER &= ~(3u << (0 * 2));
            GPIOB->MODER |=  (1u << (0 * 2));
        }

        static inline void led_on(void)  { GPIOB->BSRR = (1u << 0); }
        static inline void led_off(void) { GPIOB->BSRR = (1u << (0 + 16)); }
        static inline void led_toggle(void){ GPIOB->ODR ^= (1u << 0); }
    }
}