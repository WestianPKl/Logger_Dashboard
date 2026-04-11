#include <stdint.h>
#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "can.h"
#include "can_protocol.h"
#include "sht40.h"
#include "bme280.h"
#include "rtc.h"
#include "support.h"
#include "app_flags.h"
#include "version.h"

extern SemaphoreHandle_t sensorDataMutex;
extern SemaphoreHandle_t rtcDataMutex;
extern SemaphoreHandle_t canMutex;

extern volatile uint16_t adc_data_buffer[ADC_BUFFER_SIZE];

uint32_t tx_mailbox;
volatile uint8_t counter = 0U;

static int send_frame_0x10(void)
{
    const device_info_t *info = device_info_get();
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[5] = {0};
    uint32_t serial = 0U;

    if (info->magic == INFO_MAGIC) {
        serial = info->serial;
    }

    tx_header.StdId = 0x10;
    tx_header.ExtId = 0x00;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 5U;
    tx_header.TransmitGlobalTime = DISABLE;

    tx_data[0] = (uint8_t)((serial >> 24) & 0xFFU);
    tx_data[1] = (uint8_t)((serial >> 16) & 0xFFU);
    tx_data[2] = (uint8_t)((serial >> 8) & 0xFFU);
    tx_data[3] = (uint8_t)(serial & 0xFFU);
    tx_data[4] = counter++;

    if (counter > 250U) {
        counter = 0U;
    }

    return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
}

static int send_frame_0x11(void)
{
    const device_info_t *info = device_info_get();
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[5] = {0};
    uint8_t hwmaj = 0U;
    uint8_t hwmin = 0U;

    if (info->magic == INFO_MAGIC) {
        hwmaj = info->hw_major;
        hwmin = info->hw_minor;
    }

    tx_header.StdId = 0x11;
    tx_header.ExtId = 0x00;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 5U;
    tx_header.TransmitGlobalTime = DISABLE;

    tx_data[0] = FW_VERSION_MAJOR;
    tx_data[1] = FW_VERSION_MINOR;
    tx_data[2] = FW_VERSION_PATCH;
    tx_data[3] = hwmaj;
    tx_data[4] = hwmin;

    return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
}

static int send_frame_0x12(void)
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[6] = {0};
    uint8_t build_date[10] = {0};

    tx_header.StdId = 0x12;
    tx_header.ExtId = 0x00;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 6U;
    tx_header.TransmitGlobalTime = DISABLE;

    memcpy(build_date, FW_BUILD_DATE, 10);

    // tx_data[0] = build_date[0];
    // tx_data[1] = build_date[1];
    // tx_data[2] = build_date[2];
    // tx_data[3] = build_date[3];
    // tx_data[4] = build_date[5];
    // tx_data[5] = build_date[6];
    // tx_data[6] = build_date[8];
    // tx_data[7] = build_date[9];

    tx_data[0] = build_date[2];
    tx_data[1] = build_date[3];
    tx_data[2] = build_date[5];
    tx_data[3] = build_date[6];
    tx_data[4] = build_date[8];
    tx_data[5] = build_date[9];

    return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
}

static int send_frame_0x13(void)
{
    const device_info_t *info = device_info_get();
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[6] = {0};
    uint8_t production_date[10] = {0};

    tx_header.StdId = 0x13;
    tx_header.ExtId = 0x00;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 6U;
    tx_header.TransmitGlobalTime = DISABLE;

    if (info->magic == INFO_MAGIC) {
        memcpy(production_date, info->prod_date, 10);
    }

    // tx_data[0] = production_date[0];
    // tx_data[1] = production_date[1];
    // tx_data[2] = production_date[2];
    // tx_data[3] = production_date[3];
    // tx_data[4] = production_date[5];
    // tx_data[5] = production_date[6];
    // tx_data[6] = production_date[8];
    // tx_data[7] = production_date[9];

    tx_data[0] = production_date[2];
    tx_data[1] = production_date[3];
    tx_data[2] = production_date[5];
    tx_data[3] = production_date[6];
    tx_data[4] = production_date[8];
    tx_data[5] = production_date[9];

    return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
}

static int send_frame_0x20(void)
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[4] = {0};
    uint16_t v0;
    uint16_t v1;

    tx_header.StdId = 0x20;
    tx_header.ExtId = 0x00;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 4U;
    tx_header.TransmitGlobalTime = DISABLE;

    if ((xEventGroupGetBits(appEvents) & EVT_ADC_PRESENT) == 0U) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 1U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    if (ADC_BUFFER_SIZE < 2U) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 2U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    v0 = adc_data_buffer[0];
    v1 = adc_data_buffer[1];

    tx_data[0] = (uint8_t)(v0 >> 8);
    tx_data[1] = (uint8_t)(v0);
    tx_data[2] = (uint8_t)(v1 >> 8);
    tx_data[3] = (uint8_t)(v1);

    return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
}

static int send_frame_0x30(void)
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[4] = {0};
    int16_t temp_c_x100;
    uint16_t rh_x100;

    tx_header.StdId = 0x30;
    tx_header.ExtId = 0x00;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 4U;
    tx_header.TransmitGlobalTime = DISABLE;

    if ((xEventGroupGetBits(appEvents) & EVT_SHT40_PRESENT) == 0U) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 1U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    if (xEventGroupGetBits(appEvents) & EVT_SHT40_ERROR) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 2U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 3U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    temp_c_x100 = sht40_data.temperature;
    rh_x100 = sht40_data.humidity;

    xSemaphoreGive(sensorDataMutex);

    tx_data[0] = (uint8_t)((temp_c_x100 >> 8) & 0xFFU);
    tx_data[1] = (uint8_t)(temp_c_x100 & 0xFFU);
    tx_data[2] = (uint8_t)((rh_x100 >> 8) & 0xFFU);
    tx_data[3] = (uint8_t)(rh_x100 & 0xFFU);

    return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
}

static int send_frame_0x31(void)
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[4] = {0};
    int16_t temp_x100;
    uint16_t hum_x100;

    tx_header.StdId = 0x31;
    tx_header.ExtId = 0x00;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 4U;
    tx_header.TransmitGlobalTime = DISABLE;

    if ((xEventGroupGetBits(appEvents) & EVT_BME280_PRESENT) == 0U) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 1U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    if (xEventGroupGetBits(appEvents) & EVT_BME280_ERROR) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 2U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 3U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    temp_x100 = (int16_t)(bme280_data.temperature);
    hum_x100  = (uint16_t)(bme280_data.humidity);

    xSemaphoreGive(sensorDataMutex);

    tx_data[0] = (uint8_t)((temp_x100 >> 8) & 0xFFU);
    tx_data[1] = (uint8_t)(temp_x100 & 0xFFU);
    tx_data[2] = (uint8_t)((hum_x100 >> 8) & 0xFFU);
    tx_data[3] = (uint8_t)(hum_x100 & 0xFFU);

    return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
}

static int send_frame_0x32(void)
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[4] = {0};
    uint32_t press_pa;

    tx_header.StdId = 0x32;
    tx_header.ExtId = 0x00;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 4U;
    tx_header.TransmitGlobalTime = DISABLE;

    if ((xEventGroupGetBits(appEvents) & EVT_BME280_PRESENT) == 0U) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 1U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    if (xEventGroupGetBits(appEvents) & EVT_BME280_ERROR) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 2U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 3U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    press_pa  = bme280_data.pressure;

    xSemaphoreGive(sensorDataMutex);

    tx_data[0] = (uint8_t)((press_pa >> 24) & 0xFFU);
    tx_data[1] = (uint8_t)((press_pa >> 16) & 0xFFU);
    tx_data[2] = (uint8_t)((press_pa >> 8) & 0xFFU);
    tx_data[3] = (uint8_t)(press_pa & 0xFFU);

    return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
}

static int send_frame_0x60(void)
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[6] = {0};

    tx_header.StdId = 0x60;
    tx_header.ExtId = 0x00;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 6U;
    tx_header.TransmitGlobalTime = DISABLE;

    if (xSemaphoreTake(rtcDataMutex, pdMS_TO_TICKS(50)) != pdTRUE) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 1U;
        tx_header.DLC = 2U;
        return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
    }

    tx_data[0] = rtc_date_time.year;
    tx_data[1] = rtc_date_time.month;
    tx_data[2] = rtc_date_time.day;
    tx_data[3] = rtc_date_time.hours;
    tx_data[4] = rtc_date_time.minutes;
    tx_data[5] = rtc_date_time.seconds;

    xSemaphoreGive(rtcDataMutex);

    return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
}

void send_cyclic_frames(void)
{
    static uint8_t slot = 0U;

    switch (slot) {
        case 0U:
            (void)send_frame_0x10();
            break;
        case 1U:
            (void)send_frame_0x11();
            break;
        case 2U:
            (void)send_frame_0x12();
            break;
        case 3U:
            (void)send_frame_0x13();
            break;
        case 4U:
            (void)send_frame_0x20();
            break;
        case 5U:
            (void)send_frame_0x30();
            break;
        case 6U:
            (void)send_frame_0x31();
            break;
        case 7U:
            (void)send_frame_0x32();
            break;
        case 8U:
            (void)send_frame_0x60();
            break;
        default:
            break;
    }

    slot++;
    if (slot >= 9U) {
        slot = 0U;
    }
}