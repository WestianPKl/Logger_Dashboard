#include "can_protocol.h"
#include <stdint.h>
#include <string.h>
#include "uart.h"
#include "sht40.h"
#include "bme280.h"
#include "rtc.h"
#include "support.h"
#include "app_flags.h"
#include "version.h"
#include "uart_protocol.h"
#include "can.h"

#define STATUS_OK       0x40
#define ERROR_RESPONSE  0x7F
#define CAN_FRAME_GAP_MS 20U

uint32_t tx_mailbox;
volatile uint8_t counter;

volatile can_rx_header_typedef can_rx_header_last;
volatile uint8_t can_rx_data_last[8];
volatile uint8_t can_rx_ready;

volatile uint32_t can_slot_ok[8];
volatile uint32_t can_slot_fail[8];
volatile uint32_t can_slot_last_err[8];

static int can_send_std(uint16_t std_id, const uint8_t *data, uint8_t dlc)
{
    can_tx_header_typedef tx_header;
    uint8_t tx_data[8] = {0};

    if ((data == NULL) && (dlc != 0U)) {
        return -1;
    }

    if (dlc > 8U) {
        return -1;
    }

    tx_header.std_id = std_id;
    tx_header.ext_id = 0U;
    tx_header.ide = CAN_ID_STD;
    tx_header.rtr = 0U;
    tx_header.dlc = dlc;
    tx_header.transmit_global_time = 0U;

    if ((data != NULL) && (dlc != 0U)) {
        memcpy(tx_data, data, dlc);
    }

    return can_add_tx_message(&tx_header, tx_data, &tx_mailbox);
}

static int send_frame_0x10(void)
{
    const device_info_t *info = device_info_get();
    uint8_t tx_data[5] = {0};
    uint32_t serial = 0U;

    if (info->magic == INFO_MAGIC) {
        serial = info->serial;
    }

    tx_data[0] = (uint8_t)((serial >> 24) & 0xFF);
    tx_data[1] = (uint8_t)((serial >> 16) & 0xFF);
    tx_data[2] = (uint8_t)((serial >> 8) & 0xFF);
    tx_data[3] = (uint8_t)(serial & 0xFF);
    tx_data[4] = counter++;

    if (counter > 250U) {
        counter = 0U;
    }

    return can_send_std(0x10U, tx_data, 5U);
}

static int send_frame_0x11(void)
{
    const device_info_t *info = device_info_get();
    uint8_t tx_data[5] = {0};
    uint8_t hwmaj = 0U;
    uint8_t hwmin = 0U;

    if (info->magic == INFO_MAGIC) {
        hwmaj = info->hw_major;
        hwmin = info->hw_minor;
    }

    tx_data[0] = FW_VERSION_MAJOR;
    tx_data[1] = FW_VERSION_MINOR;
    tx_data[2] = FW_VERSION_PATCH;
    tx_data[3] = hwmaj;
    tx_data[4] = hwmin;

    return can_send_std(0x11U, tx_data, 5U);
}

static int send_frame_0x12(void)
{
    uint8_t tx_data[8] = {0};
    uint8_t build_date[10] = {0};

    memcpy(build_date, FW_BUILD_DATE, 10);

    tx_data[0] = build_date[0];
    tx_data[1] = build_date[1];
    tx_data[2] = build_date[2];
    tx_data[3] = build_date[3];
    tx_data[4] = build_date[5];
    tx_data[5] = build_date[6];
    tx_data[6] = build_date[8];
    tx_data[7] = build_date[9];

    return can_send_std(0x12U, tx_data, 8U);
}

static int send_frame_0x13(void)
{
    const device_info_t *info = device_info_get();
    uint8_t tx_data[8] = {0};
    uint8_t production_date[10] = {0};

    if (info->magic == INFO_MAGIC) {
        memcpy(production_date, info->prod_date, 10);
    }

    tx_data[0] = production_date[0];
    tx_data[1] = production_date[1];
    tx_data[2] = production_date[2];
    tx_data[3] = production_date[3];
    tx_data[4] = production_date[5];
    tx_data[5] = production_date[6];
    tx_data[6] = production_date[8];
    tx_data[7] = production_date[9];

    return can_send_std(0x13U, tx_data, 8U);
}

static int send_frame_0x20(void)
{
    uint8_t tx_data[8] = {0};

    if (!adc_present) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 1U;
        return can_send_std(0x20U, tx_data, 2U);
    }

    if (ADC_BUFFER_SIZE < 2) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 2U;
        return can_send_std(0x20U, tx_data, 2U);
    }

    tx_data[0] = (uint8_t)(adc_data_buffer[0] >> 8);
    tx_data[1] = (uint8_t)(adc_data_buffer[0]);
    tx_data[2] = (uint8_t)(adc_data_buffer[1] >> 8);
    tx_data[3] = (uint8_t)(adc_data_buffer[1]);

    return can_send_std(0x20U, tx_data, 4U);
}

static int send_frame_0x30(void)
{
    uint8_t tx_data[8] = {0};
    int16_t temp_c_x100;
    uint16_t rh_x100;

    if (!sht40_present) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 1U;
        return can_send_std(0x30U, tx_data, 2U);
    }

    if (sht40_error_flag) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 2U;
        return can_send_std(0x30U, tx_data, 2U);
    }

    temp_c_x100 = measurement_sht40.temperature;
    rh_x100 = measurement_sht40.humidity;

    tx_data[0] = (uint8_t)((temp_c_x100 >> 8) & 0xFF);
    tx_data[1] = (uint8_t)(temp_c_x100 & 0xFF);
    tx_data[2] = (uint8_t)((rh_x100 >> 8) & 0xFF);
    tx_data[3] = (uint8_t)(rh_x100 & 0xFF);

    return can_send_std(0x30U, tx_data, 4U);
}

static int send_frame_0x31(void)
{
    uint8_t tx_data[8] = {0};
    int16_t temp_x100;
    uint16_t hum_x100;
    uint32_t press_pa;

    if (!bme280_present) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 1U;
        return can_send_std(0x31U, tx_data, 2U);
    }

    if (bme280_error_flag) {
        tx_data[0] = ERROR_RESPONSE;
        tx_data[1] = 2U;
        return can_send_std(0x31U, tx_data, 2U);
    }

    temp_x100 = (int16_t)measurement_bme280.temperature;
    hum_x100  = (uint16_t)measurement_bme280.humidity;
    press_pa  = measurement_bme280.pressure;

    tx_data[0] = (uint8_t)((temp_x100 >> 8) & 0xFF);
    tx_data[1] = (uint8_t)(temp_x100 & 0xFF);
    tx_data[2] = (uint8_t)((hum_x100 >> 8) & 0xFF);
    tx_data[3] = (uint8_t)(hum_x100 & 0xFF);
    tx_data[4] = (uint8_t)((press_pa >> 24) & 0xFF);
    tx_data[5] = (uint8_t)((press_pa >> 16) & 0xFF);
    tx_data[6] = (uint8_t)((press_pa >> 8) & 0xFF);
    tx_data[7] = (uint8_t)(press_pa & 0xFF);

    return can_send_std(0x31U, tx_data, 8U);
}

static int send_frame_0x60(void)
{
    uint8_t tx_data[7];

    tx_data[0] = datetime.year;
    tx_data[1] = datetime.month;
    tx_data[2] = datetime.day;
    tx_data[3] = datetime.weekday;
    tx_data[4] = datetime.hours;
    tx_data[5] = datetime.minutes;
    tx_data[6] = datetime.seconds;

    return can_send_std(0x60U, tx_data, 7U);
}

void send_cyclic_frames(void)
{
    static uint8_t slot = 0U;
    static uint32_t last_send_tick = 0U;
    int result = -1;
    uint32_t now = HAL_GetTick();

    if ((uint32_t)(now - last_send_tick) < CAN_FRAME_GAP_MS) {
        return;
    }

    if (!can_tx_idle()) {
        return;
    }

    switch (slot) {
        case 0: result = send_frame_0x10(); break;
        case 1: result = send_frame_0x11(); break;
        case 2: result = send_frame_0x12(); break;
        case 3: result = send_frame_0x13(); break;
        case 4: result = send_frame_0x20(); break;
        case 5: result = send_frame_0x30(); break;
        case 6: result = send_frame_0x31(); break;
        case 7: result = send_frame_0x60(); break;
        default: result = -1; break;
    }

    if (result == 0) {
        can_slot_ok[slot]++;
    } else {
        can_slot_fail[slot]++;
        can_slot_last_err[slot] = can_last_status;
    }
    last_send_tick = now;
    slot++;
    if (slot >= 8U) {
        slot = 0U;
    }
}