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

#define SHT30_ADDR 0x44

static int sht30_read(float *t_c, float *rh) {
    uint8_t cmd[2] = {0x2C, 0x06};
    if (!i2c1_write(SHT30_ADDR, cmd, 2)) return 0;

    delay_ms(15);

    uint8_t d[6];
    if (!i2c1_read(SHT30_ADDR, d, 6)) return 0;

    uint16_t t_raw  = ((uint16_t)d[0] << 8) | d[1];
    uint16_t rh_raw = ((uint16_t)d[3] << 8) | d[4];

    *t_c = -45.0f + 175.0f * ((float)t_raw / 65535.0f);
    *rh  = 100.0f * ((float)rh_raw / 65535.0f);
    return 1;
}

int main(void) {
    systick_init_1ms();

    while (1) {
        // Your main loop code here
    }
}