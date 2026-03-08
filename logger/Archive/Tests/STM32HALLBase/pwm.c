HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, duty); // duty 0..ARR