#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include "main.h"
#include <stdint.h>

#define ERROR_RESPONSE 0x7F

typedef struct
{
    CAN_RxHeaderTypeDef header;
    uint8_t data[8];
} can_rx_msg_t;

extern uint32_t tx_mailbox;
extern volatile uint8_t counter;

void send_cyclic_frames(void);

#endif // CAN_PROTOCOL_H