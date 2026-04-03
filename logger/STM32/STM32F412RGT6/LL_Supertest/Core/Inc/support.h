#ifndef SUPPORT_H
#define SUPPORT_H

#include <stdint.h>

typedef struct {
    uint32_t start;
    uint32_t period;
    uint8_t active;
} soft_timer_t;

void timer_start(soft_timer_t *t, uint32_t period_ms);
void timer_stop(soft_timer_t *t);
uint8_t timer_expired(soft_timer_t *t);
uint8_t timer_running(const soft_timer_t *t);
void delay_ms(uint32_t ms);
uint8_t crc8_atm(const uint8_t *data, uint32_t len);
uint16_t crc16_ccitt_false(const uint8_t *data, uint16_t len);
uint32_t crc32(const uint8_t *data, uint32_t len);

#endif // SUPPORT_H
