HAL_ADC_Start(&hadc1);
HAL_ADC_PollForConversion(&hadc1, 10);
uint32_t raw = HAL_ADC_GetValue(&hadc1);
HAL_ADC_Stop(&hadc1);