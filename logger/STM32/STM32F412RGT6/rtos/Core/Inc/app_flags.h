#ifndef APP_FLAGS_H
#define APP_FLAGS_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "event_groups.h"

#define EVT_LCD_REINIT         (1UL << 0)
#define EVT_RTC_REINIT         (1UL << 1)
#define EVT_BME280_REINIT      (1UL << 2)

#define EVT_SHT40_ERROR        (1UL << 3)
#define EVT_BME280_ERROR       (1UL << 4)

#define EVT_EXT_RTC_PRESENT    (1UL << 5)
#define EVT_LCD_PRESENT        (1UL << 6)
#define EVT_SHT40_PRESENT      (1UL << 7)
#define EVT_BME280_PRESENT     (1UL << 8)
#define EVT_FLASH_PRESENT      (1UL << 9)
#define EVT_ADC_PRESENT        (1UL << 10)
#define EVT_CAN_PRESENT        (1UL << 11)

/*
    * @brief  Structure holding the state of application I/O: buttons, LEDs, and GPIO pins.
*/
typedef struct
{
    uint8_t btn1_pressed;
    uint8_t btn2_pressed;
    uint8_t led1_state;
    uint8_t led2_state;
    uint8_t pb12_state;
    uint8_t pc0_state;
    uint8_t pc1_state;
    uint8_t pc2_state;
    uint8_t pc3_state;
    uint8_t esp32_state;
    uint8_t esp32_write_state;
} app_io_state_t;

/*
    * @brief  Structure holding RGB LED color settings.
*/
typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t brightness;
} app_rgb_t;

/*
    * @brief  Structure holding buzzer configuration: frequency and duty cycle.
*/
typedef struct
{
    uint32_t freq;
    uint32_t duty_cycle;
} app_buzzer_t;

#define EVT_ALL_PERIPHERAL_BITS ( \
    EVT_EXT_RTC_PRESENT | EVT_FLASH_PRESENT | EVT_LCD_PRESENT | \
    EVT_SHT40_PRESENT | EVT_BME280_PRESENT | EVT_ADC_PRESENT | EVT_CAN_PRESENT)

extern volatile uint16_t flag_100ms;
extern volatile app_io_state_t app_io_state;
extern volatile app_rgb_t app_rgb;
extern volatile app_buzzer_t app_buzzer;
extern EventGroupHandle_t appEvents;

/*
    * @brief  Apply the configuration flags read from FRAM to the application event group.
    *         This function translates the bitwise flags from FRAM into corresponding event bits
    *         in the FreeRTOS event group, allowing tasks to react to the presence of peripherals.
    * @param  cfg_flags: The configuration flags read from FRAM indicating which peripherals are present.
    * @retval None      
*/
void app_flags_apply(uint8_t cfg_flags);

#endif // APP_FLAGS_H