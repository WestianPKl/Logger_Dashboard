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
        static volatile uint8_t g_btn_fired = 0;

        static void button_exti_init_pb1_falling(void) {
            RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
            RCC->APBENR2 |= RCC_APBENR2_SYSCFGEN;

            // PB1 input + pull-up
            GPIOB->MODER &= ~(3u << (1 * 2));
            GPIOB->PUPDR &= ~(3u << (1 * 2));
            GPIOB->PUPDR |=  (1u << (1 * 2)); // pull-up

            // Map EXTI1 to port B (EXTICR1: EXTI1[7:4])
            // 0=PA, 1=PB, 2=PC...
            SYSCFG->EXTICR[0] &= ~(0xFu << 4);
            SYSCFG->EXTICR[0] |=  (0x1u << 4); // PB

            // Falling edge trigger on line 1
            EXTI->IMR1  |= (1u << 1);
            EXTI->FTSR1 |= (1u << 1);
            EXTI->RTSR1 &= ~(1u << 1);

            // Clear pending flags (na G0 są osobne RPR1/FPR1)
            EXTI->FPR1 = (1u << 1);
            EXTI->RPR1 = (1u << 1);

            NVIC_EnableIRQ(EXTI0_1_IRQn);
        }

        void EXTI0_1_IRQHandler(void) {
            if (EXTI->FPR1 & (1u << 1)) {
                EXTI->FPR1 = (1u << 1); // clear
                g_btn_fired = 1;
            }
        }
    }
}