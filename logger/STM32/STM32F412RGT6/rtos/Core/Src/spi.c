#include "spi.h"

TaskHandle_t spiOwnerTask = NULL;
volatile uint8_t spiError = 0U;

void spi1_cs1_high(void)
{
    HAL_GPIO_WritePin(SPI1_CS1_GPIO_Port, SPI1_CS1_Pin, GPIO_PIN_SET);
}

void spi1_cs1_low(void)
{
    HAL_GPIO_WritePin(SPI1_CS1_GPIO_Port, SPI1_CS1_Pin, GPIO_PIN_RESET);
}

void spi1_cs2_high(void)
{
    HAL_GPIO_WritePin(SPI1_CS2_GPIO_Port, SPI1_CS2_Pin, GPIO_PIN_SET);
}

void spi1_cs2_low(void)
{
    HAL_GPIO_WritePin(SPI1_CS2_GPIO_Port, SPI1_CS2_Pin, GPIO_PIN_RESET);
}

int8_t wait_spi_done(TickType_t timeoutTicks)
{
    uint32_t notified;

    notified = ulTaskNotifyTake(pdTRUE, timeoutTicks);
    if (notified == 0U) {
        return -1;
    }

    if (spiError != 0U) {
        spiError = 0U;
        return -1;
    }

    return 1;
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hspi->Instance == SPI1 && spiOwnerTask != NULL) {
        vTaskNotifyGiveFromISR(spiOwnerTask, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hspi->Instance == SPI1 && spiOwnerTask != NULL) {
        vTaskNotifyGiveFromISR(spiOwnerTask, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hspi->Instance == SPI1 && spiOwnerTask != NULL) {
        vTaskNotifyGiveFromISR(spiOwnerTask, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hspi->Instance == SPI1 && spiOwnerTask != NULL) {
        spiError = 1U;
        vTaskNotifyGiveFromISR(spiOwnerTask, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}