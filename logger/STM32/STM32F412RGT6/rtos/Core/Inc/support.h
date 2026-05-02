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
    * @brief  Compute CRC-32 (polynomial 0xEDB88320, reflected) over the given data.
    * @param  data: Pointer to the input buffer.
    * @param  len: Number of bytes.
    * @retval Computed CRC-32 value, 0 if data is NULL.
*/
uint32_t crc32(const uint8_t *data, uint32_t len);

#endif // SUPPORT_H
