#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

extern I2C_HandleTypeDef hi2c1;
extern SemaphoreHandle_t i2cMutex;
extern TaskHandle_t i2cOwnerTask;
extern volatile uint8_t i2cError;

/*
    * @brief  Initialize the I2C1 peripheral in master mode at 100 kHz (standard mode).
    * @retval None
*/
void MX_I2C1_Init(void);

/*
    * @brief  Block the calling FreeRTOS task until the current I2C transfer completes or a timeout occurs.
    *         On timeout, performs a soft reset of the I2C peripheral to recover from bus errors.
    * @param  timeoutTicks: Maximum number of RTOS ticks to wait.
    * @retval 1 on success, -1 on timeout or I2C error.
*/
int8_t wait_i2c_done(TickType_t timeoutTicks);

/*
    * @brief  Perform a full de-init/re-init cycle on I2C1 to recover from bus errors.
    * @retval 1 always.
*/
int8_t i2c_restart(void);

/*
    * @brief  HAL callback invoked when an I2C memory-write transfer completes.
    * @param  hi2c: Pointer to the I2C handle.
    * @retval None
*/
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c);

/*
    * @brief  HAL callback invoked when an I2C memory-read transfer completes.
    * @param  hi2c: Pointer to the I2C handle.
    * @retval None
*/
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);

/*
    * @brief  HAL callback invoked when an I2C master-transmit transfer completes.
    * @param  hi2c: Pointer to the I2C handle.
    * @retval None
*/
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);

/*
    * @brief  HAL callback invoked when an I2C master-receive transfer completes.
    * @param  hi2c: Pointer to the I2C handle.
    * @retval None
*/
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);

/*
    * @brief  HAL callback invoked when an I2C error occurs during transfer.
    *         Sets the i2cError flag and notifies the owner task.
    * @param  hi2c: Pointer to the I2C handle.
    * @retval None
*/
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);

#endif // I2C_H