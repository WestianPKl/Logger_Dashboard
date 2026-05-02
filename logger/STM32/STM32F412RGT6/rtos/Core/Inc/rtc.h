#ifndef RTC_H
#define RTC_H

#include <stdint.h>

/*
    * @brief  Structure holding a date and time snapshot from the RTC.
*/
typedef struct {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} rtc_date_time_t;

#include "main.h"

extern RTC_HandleTypeDef hrtc;

extern QueueHandle_t rtcCmdQueue;
extern SemaphoreHandle_t rtcDataMutex;

extern TaskHandle_t RTCTaskHandle;

/*
    * @brief  Initialize the internal RTC peripheral.
    *         On first boot (no backup register marker), sets the time to 00:00:00 on 2000-01-01.
    * @retval None
*/
void MX_RTC_Init(void);

/*
    * @brief  FreeRTOS task that periodically reads the RTC (internal or external MCP7940N) and updates the shared rtc_date_time variable.
    *         Also handles RTC_CMD_SET_DATETIME commands from the command queue.
    * @param  argument: Unused task parameter.
    * @retval None
*/
void RTCTask(void *argument);

/*
    * @brief  Set the internal RTC date and time.
    * @param  year: Year offset from 2000 (0..99).
    * @param  month: Month (1..12).
    * @param  day: Day of month (1..31).
    * @param  weekday: Day of week (1=Monday..7=Sunday).
    * @param  hours: Hours (0..23).
    * @param  minutes: Minutes (0..59).
    * @param  seconds: Seconds (0..59).
    * @retval 1 on success, -1 on failure.
*/
int8_t rtc_set_datetime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday,
                      uint8_t hours, uint8_t minutes, uint8_t seconds);

/*
    * @brief  Get the current date and time from the internal RTC.
    * @param  year: Pointer for year (0..99), or NULL.
    * @param  month: Pointer for month, or NULL.
    * @param  day: Pointer for day, or NULL.
    * @param  weekday: Pointer for weekday, or NULL.
    * @param  hours: Pointer for hours, or NULL.
    * @param  minutes: Pointer for minutes, or NULL.
    * @param  seconds: Pointer for seconds, or NULL.
    * @retval 1 on success, -1 on failure.
*/
int8_t rtc_get_datetime(uint8_t *year, uint8_t *month, uint8_t *day, uint8_t *weekday,
                      uint8_t *hours, uint8_t *minutes, uint8_t *seconds);                      


#endif // RTC_H