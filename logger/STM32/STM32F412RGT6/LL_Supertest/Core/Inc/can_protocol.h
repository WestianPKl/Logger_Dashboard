#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include <stdint.h>
#include "can.h"

extern volatile can_rx_header_typedef can_rx_header_last;
extern volatile uint8_t can_rx_data_last[8];
extern volatile uint8_t can_rx_ready;

extern volatile uint32_t can_slot_ok[8];
extern volatile uint32_t can_slot_fail[8];
extern volatile uint32_t can_slot_last_err[8];

void send_cyclic_frames(void);

#endif // CAN_PROTOCOL_H