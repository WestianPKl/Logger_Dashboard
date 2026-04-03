#ifndef CAN_H
#define CAN_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "main.h"

typedef struct
{
	uint32_t std_id;
	uint32_t ext_id;
	uint32_t ide;
	uint32_t rtr;
	uint32_t dlc;
	uint8_t transmit_global_time;

}can_tx_header_typedef;


typedef struct
{
  uint32_t std_id;
  uint32_t ext_id;
  uint32_t ide;
  uint32_t rtr;
  uint32_t dlc;
  uint32_t timestamp;
  uint32_t filter_match_index;

} can_rx_header_typedef;

extern volatile uint8_t can_last_error;
extern volatile uint32_t can_diag;
extern volatile uint32_t can_last_status;
extern volatile uint32_t can_tx_no_mailbox;
extern volatile uint32_t can_tx_hal_fail;
extern volatile uint32_t can_tx_ok;

uint8_t can_add_tx_message(can_tx_header_typedef *pHeader, uint8_t aData[], uint32_t *pTxMailbox);
uint8_t can_get_rx_message(uint32_t RxFifo, can_rx_header_typedef *pHeader, uint8_t aData[]);
uint8_t can_filter_config(uint16_t std_id);
uint8_t can_filter_accept_all(void);
uint8_t can_start(void);

#endif // CAN_H