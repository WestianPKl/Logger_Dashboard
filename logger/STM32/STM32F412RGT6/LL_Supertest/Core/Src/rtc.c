#include "rtc.h"
#include "app_flags.h"

static uint8_t bcd2bin(uint8_t bcd)
{
    return (uint8_t)(((bcd >> 4) * 10U) + (bcd & 0x0FU));
}

void rtc_set_datetime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday,
                      uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    LL_RTC_TimeTypeDef RTC_TimeStruct = {0};
    LL_RTC_DateTypeDef RTC_DateStruct = {0};

    RTC_TimeStruct.Hours = hours;
    RTC_TimeStruct.Minutes = minutes;
    RTC_TimeStruct.Seconds = seconds;
    LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BIN, &RTC_TimeStruct);

    RTC_DateStruct.WeekDay = weekday;
    RTC_DateStruct.Month = month;
    RTC_DateStruct.Day = day;
    RTC_DateStruct.Year = year;
    LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BIN, &RTC_DateStruct);
}

void rtc_get_datetime(uint8_t *year, uint8_t *month, uint8_t *day, uint8_t *weekday,
                      uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    if (hours)   *hours   = bcd2bin((uint8_t)__LL_RTC_GET_HOUR(LL_RTC_TIME_Get(RTC)));
    if (minutes) *minutes = bcd2bin((uint8_t)__LL_RTC_GET_MINUTE(LL_RTC_TIME_Get(RTC)));
    if (seconds) *seconds = bcd2bin((uint8_t)__LL_RTC_GET_SECOND(LL_RTC_TIME_Get(RTC)));

    uint32_t date = LL_RTC_DATE_Get(RTC);
    if (year)    *year    = bcd2bin((uint8_t)__LL_RTC_GET_YEAR(date));
    if (month)   *month   = bcd2bin((uint8_t)__LL_RTC_GET_MONTH(date));
    if (day)     *day     = bcd2bin((uint8_t)__LL_RTC_GET_DAY(date));
    if (weekday) *weekday = bcd2bin((uint8_t)__LL_RTC_GET_WEEKDAY(date));
}