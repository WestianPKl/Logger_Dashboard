#include "timer.h"

void timer1_pwm_ch1_set_duty(uint8_t duty)
{
    uint32_t compare;

    if (duty > 100U) duty = 100U;

    compare = ((htim1.Init.Period + 1U) * duty) / 100U;
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, compare);
}

void timer2_pwm_ch3_set_duty(uint8_t duty)
{
    uint32_t compare;

    if (duty > 100U) duty = 100U;

    compare = ((htim2.Init.Period + 1U) * duty) / 100U;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, compare);
}

void timer4_pwm_ch3_set_duty(uint8_t duty)
{
    uint32_t compare;

    if (duty > 100U) duty = 100U;

    compare = ((htim4.Init.Period + 1U) * duty) / 100U;
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, compare);
}

void timer4_pwm_ch4_set_duty(uint8_t duty)
{
    uint32_t compare;

    if (duty > 100U) duty = 100U;

    compare = ((htim4.Init.Period + 1U) * duty) / 100U;
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, compare);
}

void timer3_pwm_set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
    uint32_t duty_r;
    uint32_t duty_g;
    uint32_t duty_b;

    if (brightness > 100U) brightness = 100U;

    duty_r = ((uint32_t)r * brightness) / 100U;
    duty_g = ((uint32_t)g * brightness) / 100U;
    duty_b = ((uint32_t)b * brightness) / 100U;

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, ((htim3.Init.Period + 1U) * duty_r) / 255U);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, ((htim3.Init.Period + 1U) * duty_g) / 255U);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ((htim3.Init.Period + 1U) * duty_b) / 255U);
}

void timer8_pwm_set_buzzer(uint32_t freq, uint32_t volume)
{
    uint32_t timer_clk;
    uint32_t arr;
    uint32_t compare;

    if ((freq == 0U) || (volume == 0U)) {
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, 0U);
        return;
    }

    if (volume > 100U) volume = 100U;

    timer_clk = HAL_RCC_GetPCLK2Freq();

    if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_HCLK_DIV1) {
        timer_clk *= 2U;
    }

    timer_clk /= (htim8.Init.Prescaler + 1U);

    arr = (timer_clk / freq);
    if (arr == 0U) {
        arr = 1U;
    }

    arr -= 1U;

    if (arr > 0xFFFFU) {
        arr = 0xFFFFU;
    }

    __HAL_TIM_SET_AUTORELOAD(&htim8, arr);
    __HAL_TIM_SET_COUNTER(&htim8, 0U);

    compare = ((arr + 1U) * volume) / 100U;
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, compare);
}