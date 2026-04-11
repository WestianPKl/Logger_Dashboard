#include "can.h"
#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "can_protocol.h"

extern QueueHandle_t canQueue;

#include "can.h"
#include "main.h"

uint8_t can_start(void)
{
    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        return (uint8_t)-1;
    }

    return 1;
}

uint8_t can_add_tx_message(CAN_TxHeaderTypeDef *pHeader, uint8_t aData[], uint32_t *pTxMailbox)
{
    uint32_t start;

    if ((pHeader == NULL) || (aData == NULL) || (pTxMailbox == NULL))
    {
        return (uint8_t)-1;
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U)
    {
        return (uint8_t)-2;
    }

    if (HAL_CAN_AddTxMessage(&hcan1, pHeader, aData, pTxMailbox) != HAL_OK)
    {
        return (uint8_t)-3;
    }

    start = HAL_GetTick();

    while (HAL_CAN_IsTxMessagePending(&hcan1, *pTxMailbox))
    {
        if ((HAL_GetTick() - start) > 50U)
        {
            return (uint8_t)-4;
        }
    }

    return 1;
}

uint8_t can_filter_accept_all(void)
{
    CAN_FilterTypeDef can_filter;

    can_filter.FilterActivation = ENABLE;
    can_filter.FilterBank = 0;
    can_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    can_filter.FilterIdHigh = 0x0000;
    can_filter.FilterIdLow = 0x0000;
    can_filter.FilterMaskIdHigh = 0x0000;
    can_filter.FilterMaskIdLow = 0x0000;
    can_filter.FilterMode = CAN_FILTERMODE_IDMASK;
    can_filter.FilterScale = CAN_FILTERSCALE_32BIT;
    can_filter.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &can_filter) != HAL_OK)
    {
        return (uint8_t)-1;
    }

    return 1;
}

uint8_t can_filter_config(uint16_t std_id)
{
    CAN_FilterTypeDef can_filter;

    can_filter.FilterActivation = CAN_FILTER_ENABLE;
    can_filter.FilterBank = 0;
    can_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    can_filter.FilterIdHigh = (uint16_t)(std_id << 5);
    can_filter.FilterIdLow = 0x0000;
    can_filter.FilterMaskIdHigh = (uint16_t)(0x7FF << 5);
    can_filter.FilterMaskIdLow = 0x0000;
    can_filter.FilterMode = CAN_FILTERMODE_IDMASK;
    can_filter.FilterScale = CAN_FILTERSCALE_32BIT;
#if defined(CAN2)
    can_filter.SlaveStartFilterBank = 14;
#endif

    if (HAL_CAN_ConfigFilter(&hcan1, &can_filter) != HAL_OK)
    {
        return (uint8_t)-1;
    }

    return 1;
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    can_rx_msg_t msg;

    if (hcan != &hcan1) {
        return;
    }

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &msg.header, msg.data) == HAL_OK)
    {
        if (canQueue != NULL)
        {
            (void)xQueueSendFromISR(canQueue, &msg, &xHigherPriorityTaskWoken);
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
