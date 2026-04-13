#include "i2c.h"

I2C_HandleTypeDef hi2c1;

TaskHandle_t i2cOwnerTask = NULL;
volatile uint8_t i2cError = 0U;

SemaphoreHandle_t i2cMutex;

void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

int8_t wait_i2c_done(TickType_t timeoutTicks)
{
    uint32_t notified;

    notified = ulTaskNotifyTake(pdTRUE, timeoutTicks);
    if (notified == 0U) {
        __HAL_I2C_DISABLE(&hi2c1);
        hi2c1.State = HAL_I2C_STATE_READY;
        hi2c1.Mode  = HAL_I2C_MODE_NONE;
        __HAL_UNLOCK(&hi2c1);
        __HAL_I2C_ENABLE(&hi2c1);
        return -1;
    }

    if (i2cError != 0U) {
        i2cError = 0U;
        return -1;
    }

    return 1;
}

int8_t i2c_restart(void)
{
    HAL_I2C_DeInit(&hi2c1);
    HAL_Delay(2);
    HAL_I2C_Init(&hi2c1);
    return 1;
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hi2c->Instance == I2C1 && i2cOwnerTask != NULL) {
        vTaskNotifyGiveFromISR(i2cOwnerTask, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hi2c->Instance == I2C1 && i2cOwnerTask != NULL) {
        vTaskNotifyGiveFromISR(i2cOwnerTask, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hi2c->Instance == I2C1 && i2cOwnerTask != NULL) {
        vTaskNotifyGiveFromISR(i2cOwnerTask, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hi2c->Instance == I2C1 && i2cOwnerTask != NULL) {
        vTaskNotifyGiveFromISR(i2cOwnerTask, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hi2c->Instance == I2C1 && i2cOwnerTask != NULL) {
        i2cError = 1U;
        vTaskNotifyGiveFromISR(i2cOwnerTask, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}