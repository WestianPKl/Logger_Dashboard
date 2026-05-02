
#ifndef RTC_LOCALE_H
#define RTC_LOCALE_H

#include <stdint.h>

/*
    * @brief  Convert a UTC date/time to Warsaw local time (CET/CEST), applying EU DST rules.
    *         All parameters are modified in place. Year is offset from 2000.
    * @param  yy: Pointer to year (0..99).
    * @param  mo: Pointer to month (1..12).
    * @param  dd: Pointer to day (1..31).
    * @param  wd: Pointer to weekday (1=Monday..7=Sunday).
    * @param  hh: Pointer to hours (0..23).
    * @param  mi: Pointer to minutes (unused, passed for API consistency).
    * @param  ss: Pointer to seconds (unused, passed for API consistency).
    * @retval None
*/
void rtc_utc_to_warsaw(uint8_t *yy, uint8_t *mo, uint8_t *dd, uint8_t *wd,
                       uint8_t *hh, uint8_t *mi, uint8_t *ss);

#endif // RTC_LOCALE_H
