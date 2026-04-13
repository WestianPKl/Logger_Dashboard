#include "rtc.h"
#include "app_flags.h"
#include "mcp7940n.h"
#include "i2c.h"

RTC_HandleTypeDef hrtc;

SemaphoreHandle_t rtcDataMutex;
QueueHandle_t rtcCmdQueue;

TaskHandle_t RTCTaskHandle;

void MX_RTC_Init(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_RTC_Init(&hrtc) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0xA5A5U) {
        sTime.Hours = 0x00;
        sTime.Minutes = 0x00;
        sTime.Seconds = 0x00;
        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sTime.StoreOperation = RTC_STOREOPERATION_RESET;

        if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK) {
            Error_Handler();
        }

        sDate.WeekDay = RTC_WEEKDAY_MONDAY;
        sDate.Month   = RTC_MONTH_JANUARY;
        sDate.Date    = 0x01;
        sDate.Year    = 0x00;

        if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK) {
            Error_Handler();
        }

        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0xA5A5U);
    }
}

int8_t rtc_set_datetime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday,
                      uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours = hours;
    sTime.Minutes = minutes;
    sTime.Seconds = seconds;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if( HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        return -1;
    }

    sDate.WeekDay = weekday;
    sDate.Month = month;
    sDate.Date = day;
    sDate.Year = year;
    if( HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        return -1;
    }
    return 1;
}

int8_t rtc_get_datetime(uint8_t *year, uint8_t *month, uint8_t *day, uint8_t *weekday,
                      uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    if(HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        return -1;
    }
    
    if(HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        return -1;
    }

    if (hours)   *hours   = sTime.Hours;
    if (minutes) *minutes = sTime.Minutes;
    if (seconds) *seconds = sTime.Seconds;

    if (year)    *year    = sDate.Year;
    if (month)   *month   = sDate.Month;
    if (day)     *day     = sDate.Date;
    if (weekday) *weekday = sDate.WeekDay;

    return 1;
}

void RTCTask(void *argument)
{
    rtc_date_time_t local_date_time = {0};
    rtc_msg_t msg;
    EventBits_t bits;

    bits = xEventGroupGetBits(appEvents);

    if (bits & EVT_EXT_RTC_PRESENT)
    {
        if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
        {
            int rc1 = mcp7940n_init(0);
            int rc2 = mcp7940n_mfp_sqw_1hz();

            if ((rc1 != 1) || (rc2 != 1)) {
                xEventGroupClearBits(appEvents, EVT_EXT_RTC_PRESENT);
            }
            measure_trigger_update();

            xSemaphoreGive(i2cMutex);
        }
    }

    while (1)
    {
        if (xQueueReceive(rtcCmdQueue, &msg, 0) == pdTRUE)
        {
            if (msg.cmd == RTC_CMD_SET_DATETIME)
            {
                if (xEventGroupGetBits(appEvents) & EVT_EXT_RTC_PRESENT)
                {
                    if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
                    {
                        (void)mcp7940n_set_datetime(&msg.ext_datetime);
                        xSemaphoreGive(i2cMutex);
                    }
                }
                else
                {
                    rtc_set_datetime(msg.datetime.year,
                                     msg.datetime.month,
                                     msg.datetime.day,
                                     msg.datetime.weekday,
                                     msg.datetime.hours,
                                     msg.datetime.minutes,
                                     msg.datetime.seconds);
                }
            }
        }

        bits = xEventGroupGetBits(appEvents);
        if (bits & EVT_RTC_REINIT)
        {
            if (bits & EVT_EXT_RTC_PRESENT)
            {
                if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
                {
                    int rc1 = mcp7940n_init(0);
                    int rc2 = mcp7940n_mfp_sqw_1hz();

                    if ((rc1 != 1) || (rc2 != 1)) {
                        xEventGroupClearBits(appEvents, EVT_EXT_RTC_PRESENT);
                    }
                    measure_trigger_update();

                    xSemaphoreGive(i2cMutex);
                }
            }

            xEventGroupClearBits(appEvents, EVT_RTC_REINIT);
        }

        if (xSemaphoreTake(rtcDataMutex, portMAX_DELAY) == pdTRUE)
        {
            bits = xEventGroupGetBits(appEvents);

            if (bits & EVT_EXT_RTC_PRESENT)
            {
                if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
                {
                    mcp7940n_datetime_t mcp_dt;
                    if (mcp7940n_get_datetime(&mcp_dt) == 1)
                    {
                        local_date_time.year     = mcp_dt.year;
                        local_date_time.month    = mcp_dt.month;
                        local_date_time.day      = mcp_dt.mday;
                        local_date_time.weekday  = mcp_dt.wday;
                        local_date_time.hours    = mcp_dt.hour;
                        local_date_time.minutes  = mcp_dt.min;
                        local_date_time.seconds  = mcp_dt.sec;
                    }
                    xSemaphoreGive(i2cMutex);
                }
            }
            else
            {
                rtc_get_datetime(&local_date_time.year,
                                 &local_date_time.month,
                                 &local_date_time.day,
                                 &local_date_time.weekday,
                                 &local_date_time.hours,
                                 &local_date_time.minutes,
                                 &local_date_time.seconds);
            }

            rtc_date_time = local_date_time;
            xSemaphoreGive(rtcDataMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}