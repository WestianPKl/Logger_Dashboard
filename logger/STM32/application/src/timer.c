#include "timer.h"

#define TIM2_CH1_PIN   0U
#define TIM2_CH2_PIN   1U

void tim1_init(uint32_t prescaler, uint32_t period)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    (void)RCC->APB2ENR;

    TIM1->CR1 = 0;
    TIM1->CR2 = 0;

    TIM1->PSC = (prescaler > 0) ? (prescaler - 1U) : 0U;
    TIM1->ARR = (period > 0) ? (period - 1U) : 0U;

    TIM1->CR2 &= ~TIM_CR2_MMS_Msk;
    TIM1->CR2 |=  (2U << TIM_CR2_MMS_Pos);

    TIM1->EGR = TIM_EGR_UG;
    TIM1->CR1 |= TIM_CR1_CEN;
}

void tim2_pa0_pa1_pwm_init(uint32_t prescaler, uint32_t period)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    (void)RCC->APB1ENR1;

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    (void)RCC->AHB2ENR;

    GPIOA->MODER &= ~((3U << (TIM2_CH1_PIN * 2U)) | (3U << (TIM2_CH2_PIN * 2U))); 
    GPIOA->MODER |=  ((2U << (TIM2_CH1_PIN * 2U)) | (2U << (TIM2_CH2_PIN * 2U)));

    GPIOA->OTYPER &= ~((1U << TIM2_CH1_PIN) | (1U << TIM2_CH2_PIN));
    GPIOA->OSPEEDR |= ((3U << (TIM2_CH1_PIN * 2U)) | (3U << (TIM2_CH2_PIN * 2U)));

    GPIOA->PUPDR &= ~((3U << (TIM2_CH1_PIN * 2U)) | (3U << (TIM2_CH2_PIN * 2U))); 

    GPIOA->AFR[0] &= ~((0xFU << (TIM2_CH1_PIN * 4U)) | (0xFU << (TIM2_CH2_PIN * 4U)));
    GPIOA->AFR[0] |=  ((1U << (TIM2_CH1_PIN * 4U)) | (1U << (TIM2_CH2_PIN * 4U)));

    TIM2->CCMR1 &= ~((TIM_CCMR1_OC1M_Msk | TIM_CCMR1_OC1PE_Msk) | 
                     (TIM_CCMR1_OC2M_Msk | TIM_CCMR1_OC2PE_Msk));

    TIM2->CCMR1 |=  ((6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE) |
                    ((6U << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE);

    
    TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;

    TIM2->PSC = (prescaler > 0U) ? (prescaler - 1U) : 0U;
    TIM2->ARR = (period > 0U) ? (period - 1U) : 0U;

    TIM2->CCR1 = 0;
    TIM2->CCR2 = 0;
    TIM2->CNT = 0;

    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG;

    TIM2->CR1 |= TIM_CR1_CEN;
}

static void tim2_ch_force_gpio_low(uint8_t channel)
{
    switch (channel) {
        case 1U:
            TIM2->CCER &= ~TIM_CCER_CC1E;    
            GPIOA->MODER &= ~(3U << (TIM2_CH1_PIN * 2U)); 
            GPIOA->MODER |=  (1U << (TIM2_CH1_PIN * 2U));
            GPIOA->BSRR = (1U << (TIM2_CH1_PIN + 16U));
            break;
        case 2U:
            TIM2->CCER &= ~TIM_CCER_CC2E;    
            GPIOA->MODER &= ~(3U << (TIM2_CH2_PIN * 2U)); 
            GPIOA->MODER |=  (1U << (TIM2_CH2_PIN * 2U));
            GPIOA->BSRR = (1U << (TIM2_CH2_PIN + 16U));
            break;
        default:
            return;
    }
}

static void tim2_ch_restore_pwm(uint8_t channel)
{
    switch (channel) {
        case 1U:
            GPIOA->MODER &= ~(3U << (TIM2_CH1_PIN * 2U)); 
            GPIOA->MODER |=  (2U << (TIM2_CH1_PIN * 2U));
            GPIOA->AFR[0] &= ~(0xFU << (TIM2_CH1_PIN * 4U));
            GPIOA->AFR[0] |=  (1U << (TIM2_CH1_PIN * 4U));
            TIM2->CCER |= TIM_CCER_CC1E;    
            break;
        case 2U:
            GPIOA->MODER &= ~(3U << (TIM2_CH2_PIN * 2U)); 
            GPIOA->MODER |=  (2U << (TIM2_CH2_PIN * 2U));
            GPIOA->AFR[0] &= ~(0xFU << (TIM2_CH2_PIN * 4U));
            GPIOA->AFR[0] |=  (1U << (TIM2_CH2_PIN * 4U));
            TIM2->CCER |= TIM_CCER_CC2E;    
            break;
        default:
            return;
    }
}

void tim2_pa0_pa1_pwm_set_duty(uint8_t channel, int32_t duty)
{
    if (duty <= 0) {
        tim2_ch_force_gpio_low(channel);
        return;
    }

    tim2_ch_restore_pwm(channel);

    uint32_t arr = TIM2->ARR;
    uint32_t d = (uint32_t)duty;
    if (d > arr) d = arr;

    if (channel == 1U)
    {
        TIM2->CCR1 = d;
    }
    else if (channel == 2U)
    {
        TIM2->CCR2 = d;
    }
}
