void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_1) {   // np. PB1
        // ISR krótki
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    }
}