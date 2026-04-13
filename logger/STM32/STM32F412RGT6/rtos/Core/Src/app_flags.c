#include "app_flags.h"
#include "fm24cl16b.h"

volatile uint16_t flag_100ms = 0U;
volatile app_io_state_t app_io_state = {0};
volatile app_rgb_t app_rgb = {0};
volatile app_buzzer_t app_buzzer = {0};
EventGroupHandle_t appEvents;

void app_flags_apply(uint8_t cfg_flags)
{
    EventBits_t bits_to_set = 0;

    if (cfg_flags & FRAM_FLAG_EXT_RTC_PRESENT) bits_to_set |= EVT_EXT_RTC_PRESENT;
    if (cfg_flags & FRAM_FLAG_FLASH_PRESENT)   bits_to_set |= EVT_FLASH_PRESENT;
    if (cfg_flags & FRAM_FLAG_DISPLAY_PRESENT) bits_to_set |= EVT_LCD_PRESENT;
    if (cfg_flags & FRAM_FLAG_SHT40_PRESENT)   bits_to_set |= EVT_SHT40_PRESENT;
    if (cfg_flags & FRAM_FLAG_BME280_PRESENT)  bits_to_set |= EVT_BME280_PRESENT;
    if (cfg_flags & FRAM_FLAG_ADC_PRESENT)     bits_to_set |= EVT_ADC_PRESENT;
    if (cfg_flags & FRAM_FLAG_CAN_PRESENT)     bits_to_set |= EVT_CAN_PRESENT;

    xEventGroupClearBits(appEvents, EVT_ALL_PERIPHERAL_BITS);
    xEventGroupSetBits(appEvents, bits_to_set);
}