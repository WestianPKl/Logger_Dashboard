const char *msg = "Hello\r\n";
HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

uint8_t b;
if (HAL_UART_Receive(&huart2, &b, 1, 10) == HAL_OK) {
    HAL_UART_Transmit(&huart2, &b, 1, 100); // echo
}