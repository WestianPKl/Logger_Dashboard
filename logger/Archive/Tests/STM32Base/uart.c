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

static void pwm_tim1_ch1_pa8_init(uint16_t arr, uint16_t presc) {
    // GPIOA clock
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // PA8 -> AF (TIM1_CH1)
    GPIOA->MODER &= ~(3u << (8 * 2));
    GPIOA->MODER |=  (2u << (8 * 2)); // AF
    GPIOA->AFR[1] &= ~(0xFu << ((8 - 8) * 4));
    GPIOA->AFR[1] |=  (0x2u << ((8 - 8) * 4)); // AF2 = TIM1 na wielu G0

    // TIM1 clock
    RCC->APBENR2 |= RCC_APBENR2_TIM1EN;

    TIM1->PSC = presc;
    TIM1->ARR = arr;

    // PWM mode 1 on CH1, preload enable
    TIM1->CCMR1 &= ~(TIM_CCMR1_OC1M_Msk | TIM_CCMR1_CC1S_Msk);
    TIM1->CCMR1 |=  (6u << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;

    TIM1->CCR1 = 0;          // duty
    TIM1->CCER |= TIM_CCER_CC1E;

    // Advanced timer needs MOE
    TIM1->BDTR |= TIM_BDTR_MOE;

    TIM1->CR1 |= TIM_CR1_ARPE;
    TIM1->EGR  |= TIM_EGR_UG;
    TIM1->CR1 |= TIM_CR1_CEN;
}

static void pwm_set_duty_u16(uint16_t duty) {
    // duty 0..ARR
    TIM1->CCR1 = duty;
}

int main(void) {
    systick_init_1ms();

    while (1) {
        // Your main loop code here
    }
}