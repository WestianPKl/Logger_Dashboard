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

static void adc1_init_pa0(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR2 |= RCC_APBENR2_ADCEN;

    // PA0 analog mode
    GPIOA->MODER |= (3u << (0*2));

    // Wybudź ADC i włącz regulator
    ADC1->CR &= ~ADC_CR_DEEPPWD;
    ADC1->CR |= ADC_CR_ADVREGEN;
    delay_ms(2);

    // kalibracja
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL) {}

    // sample time
    ADC1->SMPR = 3u; // dłuższy sampling (dobierz)

    // enable
    ADC1->ISR |= ADC_ISR_ADRDY; // clear
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {}

    // select channel 0
    ADC1->CHSELR = ADC_CHSELR_CHSEL0;
}

static uint16_t adc1_read_once(void) {
    ADC1->CR |= ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC)) {}
    return (uint16_t)ADC1->DR;
}
   

int main(void) {
    systick_init_1ms();

    while (1) {
        // Your main loop code here
    }
}