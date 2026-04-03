#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "main.h"

void timer1_pwm_ch1_set_duty(uint8_t duty);
void timer2_pwm_ch3_set_duty(uint8_t duty);
void timer4_pwm_ch3_set_duty(uint8_t duty);
void timer4_pwm_ch4_set_duty(uint8_t duty);

void timer3_pwm_set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
void timer8_pwm_set_buzzer(uint32_t freq, uint32_t volume);


#endif // TIMER_H