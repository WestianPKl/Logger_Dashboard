#include "support.h"
#include "stm32f4xx_hal.h"

void timer_start(soft_timer_t *t, uint32_t period_ms)
{
    if (t == NULL) return;
    t->start = HAL_GetTick();
    t->period = period_ms;
    t->active = 1U;
}

void timer_stop(soft_timer_t *t)
{
    if (t == NULL) return;
    t->active = 0U;
}

uint8_t timer_running(const soft_timer_t *t)
{
    if (t == NULL) return 0U;
    return t->active ? 1U : 0U;
}

uint8_t timer_expired(soft_timer_t *t)
{
    if ((t == NULL) || (t->active == 0U)) return 0U;

    if ((uint32_t)(HAL_GetTick() - t->start) >= t->period) {
        t->active = 0U;
        return 1U;
    }
    return 0U;
}

void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

uint8_t crc8_atm(const uint8_t *data, uint32_t len)
{
    uint8_t crc = 0x00U;

    for (uint32_t i = 0U; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0U; b < 8U; b++) {
            if ((crc & 0x80U) != 0U)
                crc = (uint8_t)((crc << 1) ^ 0x07U);
            else
                crc <<= 1;
        }
    }
    return crc;
}

uint16_t crc16_ccitt_false(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFU;

    while (len--) {
        crc ^= (uint16_t)(*data++) << 8;
        for (uint8_t i = 0U; i < 8U; i++) {
            if ((crc & 0x8000U) != 0U) crc = (uint16_t)((crc << 1) ^ 0x1021U);
            else                       crc <<= 1;
        }
    }

    return crc;
}

uint32_t crc32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFu;

    for (uint32_t i = 0U; i < len; ++i) {
        crc ^= data[i];
        for (uint32_t b = 0U; b < 8U; ++b) {
            if ((crc & 1U) != 0U) crc = (crc >> 1) ^ 0xEDB88320u;
            else                  crc >>= 1;
        }
    }

    return crc;
}