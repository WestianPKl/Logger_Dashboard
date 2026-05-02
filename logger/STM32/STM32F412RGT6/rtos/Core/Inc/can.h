#ifndef CAN_H
#define CAN_H
#include "main.h"
#include <stdint.h>

extern CAN_HandleTypeDef hcan1;

extern QueueHandle_t canQueue;
extern SemaphoreHandle_t canMutex;
extern TaskHandle_t CANTaskHandle;

/*
    * @brief  Initialize the CAN1 peripheral with the desired settings.
    *         Configures CAN1 for normal mode at 500 kbit/s with auto bus-off recovery enabled.
    * @retval None
*/
void MX_CAN1_Init(void);

/*
    * @brief  FreeRTOS task that manages the CAN bus communication.
    *         Initializes CAN filters and starts the peripheral, then periodically sends cyclic CAN frames.
    * @param  argument: Unused task parameter.
    * @retval None
*/
void CANTask(void *argument);

/*
    * @brief  Start the CAN1 peripheral.
    * @retval 1 on success, (uint8_t)-1 on failure.
*/
uint8_t can_start(void);

/*
    * @brief  Transmit a CAN message and wait for the mailbox to become free.
    *         This function adds a message to the CAN transmit mailbox and polls until the transmission
    *         is complete or a 50 ms timeout occurs.
    * @param  pHeader: Pointer to the CAN TX header structure.
    * @param  aData: Pointer to the data payload array (up to 8 bytes).
    * @param  pTxMailbox: Pointer to the variable where the mailbox number will be stored.
    * @retval 1 on success, negative (cast to uint8_t) on failure.
*/
uint8_t can_add_tx_message(CAN_TxHeaderTypeDef *pHeader, uint8_t aData[], uint32_t *pTxMailbox);

/*
    * @brief  Configure a CAN hardware filter to accept only a specific standard ID.
    * @param  std_id: The 11-bit standard CAN identifier to accept.
    * @retval 1 on success, (uint8_t)-1 on failure.
*/
uint8_t can_filter_config(uint16_t std_id);

/*
    * @brief  Configure a CAN hardware filter to accept all incoming messages (no filtering).
    * @retval 1 on success, (uint8_t)-1 on failure.
*/
uint8_t can_filter_accept_all(void);

/*
    * @brief  HAL callback invoked when a CAN message is pending in RX FIFO0.
    *         Reads the message and enqueues it for processing by the CAN task.
    * @param  hcan: Pointer to the CAN handle that triggered the callback.
    * @retval None
*/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

#endif // CAN_H