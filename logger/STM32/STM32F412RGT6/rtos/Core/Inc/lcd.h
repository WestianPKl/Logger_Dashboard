#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include "main.h"
#include "FreeRTOS.h"
#include "i2c.h"
#include "timers.h"

#define LCD_NOTIFY_REFRESH (1UL << 0)
#define LCD_NOTIFY_COMMAND (1UL << 1)
#define LCD_NOTIFY_REINIT  (1UL << 2)

extern QueueHandle_t lcdCmdQueue;

extern TaskHandle_t LCDTaskHandle;
extern TimerHandle_t backlightTimerHandle;

/*
    * @brief  LCD command types for the message queue.
*/
typedef enum {
    LCD_CLEAR,
    LCD_BACKLIGHT,
} lcd_cmd_t;

/*
    * @brief  LCD message structure sent via the command queue.
*/
typedef struct {
    lcd_cmd_t cmd;
    uint8_t flag;
} lcd_msg_t;

/*
    * @brief  FreeRTOS task that manages the 16x2 I2C LCD display.
    *         Handles initialization, backlight control, clear commands, and periodic screen refresh.
    * @param  argument: Unused task parameter.
    * @retval None
*/
void LCDTask(void *argument);

/*
    * @brief  Initialize the HD44780-compatible LCD in 4-bit mode over I2C (PCF8574 expander).
    *         Sends the initialization sequence and configures the display for 2-line mode.
    * @retval None
*/
void lcd_init(void);

/*
    * @brief  Software timer callback that turns off the LCD backlight after a timeout.
    * @param  xTimer: Handle of the timer that expired.
    * @retval None
*/
void lcd_backlight_timer_callback(TimerHandle_t xTimer);

/*
    * @brief  Check whether the LCD was detected on the I2C bus.
    * @retval 1 if present, 0 if not.
*/
uint8_t lcd_is_present(void);

/*
    * @brief  Set the LCD present flag explicitly.
    * @param  present: Non-zero to mark as present, 0 to mark as absent.
    * @retval None
*/
void lcd_mark_present(uint8_t present);

/*
    * @brief  Send a null-terminated string to the LCD at the current cursor position.
    * @param  str: Pointer to the string to display.
    * @retval None
*/
void lcd_send_string(const char *str);

/*
    * @brief  Set the LCD cursor to a given column and row.
    * @param  col: Column index (0..15).
    * @param  row: Row index (0..1).
    * @retval None
*/
void lcd_set_cursor(uint8_t col, uint8_t row);

/*
    * @brief  Turn the LCD backlight on or off.
    * @param  state: Non-zero to enable backlight, 0 to disable.
    * @retval None
*/
void lcd_backlight(uint8_t state);

/*
    * @brief  Clear the entire LCD display.
    * @retval None
*/
void lcd_clear(void);

/*
    * @brief  Clear from a given column to end of line by writing spaces.
    * @param  col: Starting column index (0..15).
    * @param  row: Row index (0..1).
    * @retval None
*/
void lcd_clear_eol(uint8_t col, uint8_t row);

/*
    * @brief  Display a decimal integer on the LCD with a minimum number of digits (zero-padded).
    * @param  num: The integer value to display.
    * @param  digits: Minimum number of digits to show.
    * @retval None
*/
void lcd_send_decimal(int32_t num, uint8_t digits);

/*
    * @brief  Display a hexadecimal value on the LCD with a specified number of hex digits.
    * @param  num: The value to display.
    * @param  digits: Number of hex digits to show (1..8).
    * @retval None
*/
void lcd_send_hex(uint32_t num, uint8_t digits);

/*
    * @brief  Display a fixed-point value with 2 decimal places (value/100).
    * @param  value_x100: The value multiplied by 100.
    * @retval None
*/
void lcd_send_fixed_x100(int32_t value_x100);

/*
    * @brief  Display a temperature with 1 decimal place from a value in hundredths of degrees.
    * @param  t_x100: Temperature in hundredths of degrees Celsius.
    * @retval None
*/
void lcd_send_temp_1dp_from_x100(int16_t t_x100);

/*
    * @brief  Display a relative humidity with 1 decimal place from a value in hundredths of percent.
    * @param  rh_x100: Humidity in hundredths of percent.
    * @retval None
*/
void lcd_send_hum_1dp_from_x100(uint16_t rh_x100);

/*
    * @brief  Display pressure as an integer in hPa from a Q24.8 fixed-point value.
    * @param  p_q24_8: Pressure in Q24.8 format (divide by 256 to get Pa).
    * @retval None
*/
void lcd_send_press_int_from_q24_8(uint32_t p_q24_8);


#endif // LCD_H