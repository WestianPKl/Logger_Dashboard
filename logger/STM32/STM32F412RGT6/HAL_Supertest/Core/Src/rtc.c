#include "rtc.h"
#include "app_flags.h"

void rtc_set_datetime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday,
                      uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours = hours;
    sTime.Minutes = minutes;
    sTime.Seconds = seconds;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    sDate.WeekDay = weekday;
    sDate.Month = month;
    sDate.Date = day;
    sDate.Year = year;
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void rtc_get_datetime(uint8_t *year, uint8_t *month, uint8_t *day, uint8_t *weekday,
                      uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    if (hours)   *hours   = sTime.Hours;
    if (minutes) *minutes = sTime.Minutes;
    if (seconds) *seconds = sTime.Seconds;

    if (year)    *year    = sDate.Year;
    if (month)   *month   = sDate.Month;
    if (day)     *day     = sDate.Date;
    if (weekday) *weekday = sDate.WeekDay;
}