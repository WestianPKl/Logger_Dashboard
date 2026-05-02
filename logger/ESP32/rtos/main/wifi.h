
#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define NTP_SERVER             "192.168.18.158"
#define WIFI_SSID              "TP-Link_0A7B"
#define WIFI_PASS              "12345678"

#define WIFI_CONNECTED_BIT     BIT0
#define WIFI_FAIL_BIT          BIT1
#define WIFI_MAXIMUM_RETRY     10


/*
    * @brief  Synchronize system time using SNTP. Blocks up to 15 s waiting for a valid time.
*/
void ntp_sync(void);

/*
    * @brief  Get the current Unix timestamp if the system time has been synchronized.
    * @param  ts_out: Pointer where the 32-bit timestamp will be stored.
    * @retval 1 on success (time valid), 0 if time is not yet synchronized or ts_out is NULL.
*/
int8_t get_unix_timestamp(uint32_t *ts_out);

/*
    * @brief  Initialize Wi-Fi in station mode and connect to the configured AP.
    *         Blocks until connected or maximum retries exhausted.
*/
void wifi_init_sta(void);

#endif // WIFI_H