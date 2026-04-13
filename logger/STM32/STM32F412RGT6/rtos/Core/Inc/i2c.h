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

void MX_I2C1_Init(void);
int8_t wait_i2c_done(TickType_t timeoutTicks);
int8_t i2c_restart(void);
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);

#endif // I2C_H