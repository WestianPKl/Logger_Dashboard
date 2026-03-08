#include "systick.h"

#define SYSTICK_CLK_HZ  4000000U
#define CLK_ENABLE      (1U << 0)
#define CLK_CLKSRC      (1U << 2)
#define CLK_COUNTFLAG   (1U << 16)
#define CLK_TICKINT     (1U << 1)

void systick_delay_ms(uint32_t ms)
{
    uint32_t ticks_per_ms = SystemCoreClock / 1000u;

    while (ms--)
    {
        SysTick->LOAD = ticks_per_ms - 1u;
        SysTick->VAL  = 0u;
        SysTick->CTRL = CLK_ENABLE | CLK_CLKSRC;
        while (!(SysTick->CTRL & CLK_COUNTFLAG)) {}
        SysTick->CTRL = 0u;
    }
}
void systick_irq(void)
{
    SysTick->LOAD = (SYSTICK_CLK_HZ / 1U) - 1U;    
    SysTick->VAL = 0U;
    SysTick->CTRL = CLK_ENABLE | CLK_CLKSRC | CLK_TICKINT;
}