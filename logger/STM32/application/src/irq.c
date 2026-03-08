#include "irq.h"

void gpio_irq_init(void)
{
    __disable_irq();
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
    (void)RCC->AHB2ENR;

    GPIOC->MODER &= ~(3U << (13U * 2U));
    GPIOC->MODER |=  (0U << (13U * 2U));

    GPIOC->PUPDR &= ~(3U << (13U * 2U));
    GPIOC->PUPDR |=  (1U << (13U * 2U));

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    (void)RCC->APB2ENR;

    SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13_Msk;
    SYSCFG->EXTICR[3] |=  (2U << SYSCFG_EXTICR4_EXTI13_Pos);

    EXTI->IMR1 |= EXTI_IMR1_IM13;

    EXTI->FTSR1 |= EXTI_FTSR1_FT13;
    EXTI->RTSR1 &= ~EXTI_RTSR1_RT13;

    NVIC_EnableIRQ(EXTI15_10_IRQn);
    __enable_irq();
}