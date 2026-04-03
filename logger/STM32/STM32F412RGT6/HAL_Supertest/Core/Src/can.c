#include "can.h"
#include "can_protocol.h"
#include "main.h"

volatile uint8_t can_last_error = 0U;
volatile uint32_t can_diag = 0U;

volatile uint32_t can_tx_no_mailbox = 0U;
volatile uint32_t can_tx_hal_fail = 0U;
volatile uint32_t can_tx_ok = 0U;

volatile uint32_t can_last_std_id = 0U;
volatile uint32_t can_last_dlc = 0U;
volatile uint32_t can_last_status = 0U;

volatile uint32_t can_hal_error = 0U;
volatile uint32_t can_esr = 0U;
volatile uint32_t can_tsr = 0U;
volatile uint32_t can_free_level = 0U;
volatile uint32_t can_tx_pending = 0U;

static void can_set_error(HAL_StatusTypeDef status)
{
    can_last_error = (uint8_t)status;
    can_diag++;
}

uint8_t can_add_tx_message(can_tx_header_typedef *pHeader, uint8_t aData[], uint32_t *pTxMailbox)
{
    CAN_TxHeaderTypeDef hal_header;
    HAL_StatusTypeDef status;

    if ((pHeader == NULL) || (aData == NULL) || (pTxMailbox == NULL)) {
        can_set_error(HAL_ERROR);
        can_last_status = HAL_ERROR;
        return 1U;
    }

    can_last_std_id = pHeader->std_id;
    can_last_dlc = pHeader->dlc;

    can_free_level = HAL_CAN_GetTxMailboxesFreeLevel(&hcan1);
    if (can_free_level == 0U) {
        can_tx_no_mailbox++;
        can_set_error(HAL_BUSY);
        can_last_status = HAL_BUSY;
        return 1U;
    }

    hal_header.StdId = pHeader->std_id;
    hal_header.ExtId = pHeader->ext_id;
    hal_header.IDE   = pHeader->ide;
    hal_header.RTR   = pHeader->rtr;
    hal_header.DLC   = (pHeader->dlc > 8U) ? 8U : pHeader->dlc;
    hal_header.TransmitGlobalTime = pHeader->transmit_global_time ? ENABLE : DISABLE;

    status = HAL_CAN_AddTxMessage(&hcan1, &hal_header, aData, pTxMailbox);
    can_last_status = (uint32_t)status;

    can_hal_error = HAL_CAN_GetError(&hcan1);
    can_esr = hcan1.Instance->ESR;
    can_tsr = hcan1.Instance->TSR;
    can_free_level = HAL_CAN_GetTxMailboxesFreeLevel(&hcan1);

    if (status != HAL_OK) {
        can_tx_hal_fail++;
        can_set_error(status);
        can_tx_pending = 0U;
        return 1U;
    }

    if ((*pTxMailbox == CAN_TX_MAILBOX0) ||
        (*pTxMailbox == CAN_TX_MAILBOX1) ||
        (*pTxMailbox == CAN_TX_MAILBOX2)) {
        can_tx_pending = HAL_CAN_IsTxMessagePending(&hcan1, *pTxMailbox);
    } else {
        can_tx_pending = 0U;
    }

    can_tx_ok++;
    return 0U;
}

uint8_t can_get_rx_message(uint32_t RxFifo, can_rx_header_typedef *pHeader, uint8_t aData[])
{
    CAN_RxHeaderTypeDef hal_header;
    HAL_StatusTypeDef status;

    if ((pHeader == NULL) || (aData == NULL)) {
        can_set_error(HAL_ERROR);
        can_last_status = HAL_ERROR;
        return 1U;
    }

    status = HAL_CAN_GetRxMessage(&hcan1, RxFifo, &hal_header, aData);
    if (status != HAL_OK) {
        can_set_error(status);
        can_last_status = (uint32_t)status;
        return 1U;
    }

    pHeader->std_id             = hal_header.StdId;
    pHeader->ext_id             = hal_header.ExtId;
    pHeader->ide                = hal_header.IDE;
    pHeader->rtr                = hal_header.RTR;
    pHeader->dlc                = hal_header.DLC;
    pHeader->timestamp          = hal_header.Timestamp;
    pHeader->filter_match_index = hal_header.FilterMatchIndex;

    return 0U;
}

uint8_t can_filter_config(uint16_t std_id)
{
    CAN_FilterTypeDef filter = {0};
    HAL_StatusTypeDef status;

    filter.FilterBank           = 0;
    filter.FilterMode           = CAN_FILTERMODE_IDMASK;
    filter.FilterScale          = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh         = (uint16_t)(std_id << 5);
    filter.FilterIdLow          = 0x0000;
    filter.FilterMaskIdHigh     = (uint16_t)(0x7FF << 5);
    filter.FilterMaskIdLow      = 0x0000;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterActivation     = CAN_FILTER_ENABLE;
    filter.SlaveStartFilterBank = 14;

    status = HAL_CAN_ConfigFilter(&hcan1, &filter);
    if (status != HAL_OK) {
        can_set_error(status);
        can_last_status = (uint32_t)status;
        return 1U;
    }

    return 0U;
}

uint8_t can_filter_accept_all(void)
{
    CAN_FilterTypeDef filter = {0};
    HAL_StatusTypeDef status;

    filter.FilterBank           = 0;
    filter.FilterMode           = CAN_FILTERMODE_IDMASK;
    filter.FilterScale          = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh         = 0x0000;
    filter.FilterIdLow          = 0x0000;
    filter.FilterMaskIdHigh     = 0x0000;
    filter.FilterMaskIdLow      = 0x0000;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterActivation     = CAN_FILTER_ENABLE;
    filter.SlaveStartFilterBank = 14;

    status = HAL_CAN_ConfigFilter(&hcan1, &filter);
    if (status != HAL_OK) {
        can_set_error(status);
        can_last_status = (uint32_t)status;
        return 1U;
    }

    return 0U;
}

uint8_t can_start(void)
{
    HAL_StatusTypeDef status;

    if (can_filter_accept_all() != 0U) {
        return 1U;
    }

    status = HAL_CAN_Start(&hcan1);
    if (status != HAL_OK) {
        return 1U;
    }

    status = HAL_CAN_ActivateNotification(&hcan1,
                                          CAN_IT_RX_FIFO0_MSG_PENDING |
                                          CAN_IT_BUSOFF |
                                          CAN_IT_ERROR_WARNING |
                                          CAN_IT_ERROR_PASSIVE |
                                          CAN_IT_LAST_ERROR_CODE);
    if (status != HAL_OK) {
        return 1U;
    }

    return 0U;
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    if ((hcan == NULL) || (hcan->Instance != CAN1)) {
        return;
    }

    can_diag++;
    can_hal_error = HAL_CAN_GetError(hcan);
    can_esr = hcan->Instance->ESR;
    can_tsr = hcan->Instance->TSR;
    can_free_level = HAL_CAN_GetTxMailboxesFreeLevel(hcan);
    can_last_status = HAL_ERROR;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    can_rx_header_typedef rx_header;
    uint8_t rx_data[8] = {0};
    uint8_t len;
    uint8_t i;

    if ((hcan == NULL) || (hcan->Instance != CAN1)) {
        return;
    }

    if (can_get_rx_message(CAN_RX_FIFO0, &rx_header, rx_data) == 0U) {
        can_rx_header_last.std_id             = rx_header.std_id;
        can_rx_header_last.ext_id             = rx_header.ext_id;
        can_rx_header_last.ide                = rx_header.ide;
        can_rx_header_last.rtr                = rx_header.rtr;
        can_rx_header_last.dlc                = rx_header.dlc;
        can_rx_header_last.timestamp          = rx_header.timestamp;
        can_rx_header_last.filter_match_index = rx_header.filter_match_index;

        len = rx_header.dlc;
        if (len > 8U) {
            len = 8U;
        }

        for (i = 0U; i < len; i++) {
            can_rx_data_last[i] = rx_data[i];
        }

        for (; i < 8U; i++) {
            can_rx_data_last[i] = 0U;
        }

        can_rx_ready = 1U;
    }
}

uint8_t can_tx_idle(void)
{
    return (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 3U) ? 1U : 0U;
}