#ifndef SUPPORT_H
#define SUPPORT_H

#include <stdint.h>

/*
    * @brief  Compute CRC-8/ATM (polynomial 0x07) over the given data.
    * @param  data: Pointer to the input buffer.
    * @param  len: Number of bytes.
    * @retval Computed CRC-8 value, 0 if data is NULL.
*/
uint8_t crc8_atm(const uint8_t *data, uint32_t len);

/*
    * @brief  Compute CRC-16/CCITT-FALSE (polynomial 0x1021, init 0xFFFF) over the given data.
    * @param  data: Pointer to the input buffer.
    * @param  len: Number of bytes.
    * @retval Computed CRC-16 value, 0 if data is NULL.
*/
uint16_t crc16_ccitt_false(const uint8_t *data, uint16_t len);

/*
    * @brief  Compute CRC-32 (polynomial 0xEDB88320, reflected) over the given data.
    * @param  data: Pointer to the input buffer.
    * @param  len: Number of bytes.
    * @retval Computed CRC-32 value, 0 if data is NULL.
*/
uint32_t crc32(const uint8_t *data, uint32_t len);

/*
    * @brief  Look up a digital output channel ID by name.
    * @param  name: Channel name (e.g. "LED1", "PB12", "PC0").
    * @param  channel_id: Pointer where the channel ID will be stored.
    * @retval 1 if found, 0 if not found or invalid arguments.
*/
int8_t find_output_channel_id(const char *name, uint8_t *channel_id);

/*
    * @brief  Look up a PWM channel ID by name.
    * @param  name: Channel name (e.g. "TIM1_CH1", "TIM2_CH3").
    * @param  channel_id: Pointer where the channel ID will be stored.
    * @retval 1 if found, 0 if not found or invalid arguments.
*/
int8_t find_pwm_channel_id(const char *name, uint8_t *channel_id);

/*
    * @brief  Look up a digital input channel ID by name.
    * @param  name: Channel name (e.g. "BTN1", "BTN2").
    * @param  channel_id: Pointer where the channel ID will be stored.
    * @retval 1 if found, 0 if not found or invalid arguments.
*/
int8_t find_input_channel_id(const char *name, uint8_t *channel_id);

#endif // SUPPORT_H
