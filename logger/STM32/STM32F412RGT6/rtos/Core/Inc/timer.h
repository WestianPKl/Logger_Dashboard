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

/*
    * @brief  HAL callback used to configure timer GPIO alternate functions after init.
    * @param  htim: Pointer to the timer handle.
    * @retval None
*/
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/*
    * @brief  HAL timer period-elapsed callback. Increments HAL tick (TIM6) and triggers LCD refresh (TIM13).
    * @param  htim: Pointer to the timer handle that triggered the interrupt.
    * @retval None
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

/*
    * @brief  Initialize TIM1 for PWM output on channel 1 (1 kHz, 84 MHz / 84 / 1000).
    * @retval None
*/
void MX_TIM1_Init(void);

/*
    * @brief  Initialize TIM2 for PWM output on channel 3 (1 kHz).
    * @retval None
*/
void MX_TIM2_Init(void);

/*
    * @brief  Initialize TIM3 for RGB LED PWM output on channels 1, 2, and 3 (1 kHz).
    * @retval None
*/
void MX_TIM3_Init(void);

/*
    * @brief  Initialize TIM4 for PWM output on channels 3 and 4 (1 kHz).
    * @retval None
*/
void MX_TIM4_Init(void);

/*
    * @brief  Initialize TIM8 for buzzer PWM output on channel 4 (1 kHz default).
    * @retval None
*/
void MX_TIM8_Init(void);

/*
    * @brief  Initialize TIM13 as a 100 ms periodic interrupt timer (10 Hz) for LCD refresh.
    * @retval None
*/
void MX_TIM13_Init(void);

/*
    * @brief  Set TIM1 channel 1 PWM duty cycle.
    * @param  duty: Duty cycle in percent (0..100).
    * @retval None
*/
void timer1_pwm_ch1_set_duty(uint8_t duty);

/*
    * @brief  Set TIM2 channel 3 PWM duty cycle.
    * @param  duty: Duty cycle in percent (0..100).
    * @retval None
*/
void timer2_pwm_ch3_set_duty(uint8_t duty);

/*
    * @brief  Set TIM4 channel 3 PWM duty cycle.
    * @param  duty: Duty cycle in percent (0..100).
    * @retval None
*/
void timer4_pwm_ch3_set_duty(uint8_t duty);

/*
    * @brief  Set TIM4 channel 4 PWM duty cycle.
    * @param  duty: Duty cycle in percent (0..100).
    * @retval None
*/
void timer4_pwm_ch4_set_duty(uint8_t duty);

/*
    * @brief  Set the RGB LED color and brightness using TIM3 channels 1..3.
    * @param  r: Red intensity (0..255).
    * @param  g: Green intensity (0..255).
    * @param  b: Blue intensity (0..255).
    * @param  brightness: Overall brightness in percent (0..100).
    * @retval None
*/
void timer3_pwm_set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);

/*
    * @brief  Configure TIM8 channel 4 to output a PWM tone at the specified frequency and volume.
    *         Setting freq or volume to 0 silences the buzzer.
    * @param  freq: Desired tone frequency in Hz.
    * @param  volume: Volume in percent (0..100).
    * @retval None
*/
void timer8_pwm_set_buzzer(uint32_t freq, uint32_t volume);


#endif // TIMER_H