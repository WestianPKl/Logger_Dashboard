#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include "main.h"
#include <stdint.h>

#define ERROR_RESPONSE 0x7F

/*
    * @brief  Structure holding a received CAN message: header and data payload.
*/
typedef struct
{
    CAN_RxHeaderTypeDef header;
    uint8_t data[8];
} can_rx_msg_t;

extern uint32_t tx_mailbox;
extern volatile uint8_t counter;

/*
    * @brief  Transmit the next cyclic CAN frame in a round-robin sequence.
    *         Each call sends one frame from the set (0x10..0x60), advancing the slot counter.
    * @retval None
*/
void send_cyclic_frames(void);

#endif // CAN_PROTOCOL_H