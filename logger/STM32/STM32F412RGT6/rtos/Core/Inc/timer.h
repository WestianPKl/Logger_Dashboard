#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "main.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim13;

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void MX_TIM1_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);
void MX_TIM8_Init(void);
void MX_TIM13_Init(void);

void timer1_pwm_ch1_set_duty(uint8_t duty);
void timer2_pwm_ch3_set_duty(uint8_t duty);
void timer4_pwm_ch3_set_duty(uint8_t duty);
void timer4_pwm_ch4_set_duty(uint8_t duty);

void timer3_pwm_set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
void timer8_pwm_set_buzzer(uint32_t freq, uint32_t volume);


#endif // TIMER_H