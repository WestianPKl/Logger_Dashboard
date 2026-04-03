#include "timer.h"

void timer1_pwm_ch1_set_duty(uint8_t duty)
{
    if (duty > 100) duty = 100;
    LL_TIM_OC_SetCompareCH1(TIM1, (LL_TIM_GetAutoReload(TIM1) + 1U) * duty / 100U);
}

void timer2_pwm_ch3_set_duty(uint8_t duty)
{
    if (duty > 100) duty = 100;
    LL_TIM_OC_SetCompareCH3(TIM2, (LL_TIM_GetAutoReload(TIM2) + 1U) * duty / 100U);
}

void timer4_pwm_ch3_set_duty(uint8_t duty)
{
    if (duty > 100) duty = 100;
    LL_TIM_OC_SetCompareCH3(TIM4, (LL_TIM_GetAutoReload(TIM4) + 1U) * duty / 100U);
}

void timer4_pwm_ch4_set_duty(uint8_t duty)
{
    if (duty > 100) duty = 100;
    LL_TIM_OC_SetCompareCH4(TIM4, (LL_TIM_GetAutoReload(TIM4) + 1U) * duty / 100U);
}

void timer3_pwm_set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
    if (brightness > 100) brightness = 100;
    uint32_t duty_r = (uint32_t)r * brightness / 100U;
    uint32_t duty_g = (uint32_t)g * brightness / 100U;
    uint32_t duty_b = (uint32_t)b * brightness / 100U;

    LL_TIM_OC_SetCompareCH3(TIM3, (LL_TIM_GetAutoReload(TIM3) + 1U) * duty_r / 255U);
    LL_TIM_OC_SetCompareCH2(TIM3, (LL_TIM_GetAutoReload(TIM3) + 1U) * duty_g / 255U);
    LL_TIM_OC_SetCompareCH1(TIM3, (LL_TIM_GetAutoReload(TIM3) + 1U) * duty_b / 255U);
}

void timer8_pwm_set_buzzer(uint32_t freq, uint32_t volume)
{
    if (freq == 0 || volume == 0) {
        LL_TIM_OC_SetCompareCH4(TIM8, 0);
        return;
    }

    uint32_t timer_clk = SystemCoreClock / (LL_TIM_GetPrescaler(TIM8) + 1U);
    uint32_t arr = timer_clk / freq - 1U;
    if (arr > 0xFFFFU) arr = 0xFFFFU;

    LL_TIM_SetAutoReload(TIM8, arr);
    LL_TIM_OC_SetCompareCH4(TIM8, (arr + 1U) * volume / 100U);
}