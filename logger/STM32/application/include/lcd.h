#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include "stm32l4xx.h"

void lcd_init(void);
void lcd_send_string(const char *str);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_backlight(uint8_t state);
void lcd_clear(void);

#endif // LCD_H