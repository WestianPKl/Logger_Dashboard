#ifndef CAN_H
#define CAN_H
#include "main.h"
#include <stdint.h>

uint8_t can_start(void);
uint8_t can_add_tx_message(CAN_TxHeaderTypeDef *pHeader, uint8_t aData[], uint32_t *pTxMailbox);
uint8_t can_filter_config(uint16_t std_id);
uint8_t can_filter_accept_all(void);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

#endif // CAN_H