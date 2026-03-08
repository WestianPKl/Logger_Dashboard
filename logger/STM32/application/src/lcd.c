#include "lcd.h"
#include "i2c_dma.h"
#include "systick.h"

#define LCD_I2C_ADDR 0x27 
#define LCD_RS   0x01
#define LCD_RW   0x02
#define LCD_E    0x04
#define LCD_BL   0x08
#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET    0x20
#define LCD_SETDDRAMADDR   0x80
#define LCD_ENTRYLEFT      0x02
#define LCD_DISPLAYON      0x04
#define LCD_2LINE          0x08

static uint8_t lcd_backlight_mask = LCD_BL;

static int8_t lcd_i2c_write(uint8_t data)
{
    return (int8_t)i2c1_write(LCD_I2C_ADDR, 0x00, &data, 1);
}

static inline void lcd_delay_short(void)
{
    for (volatile int i = 0; i < 200; i++) { __NOP(); }
}

static void lcd_pulse_enable(uint8_t data)
{
    lcd_i2c_write(data | LCD_E);
    lcd_delay_short();
    lcd_i2c_write(data & (uint8_t)~LCD_E);
    lcd_delay_short();
}

static void lcd_write4bits(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (uint8_t)((nibble & 0x0F) << 4);
    if (rs) data |= LCD_RS;
    data |= lcd_backlight_mask;
    lcd_pulse_enable(data);
}

static void lcd_write8bits(uint8_t byte, uint8_t rs)
{
    lcd_write4bits((uint8_t)(byte >> 4), rs);
    lcd_write4bits((uint8_t)(byte & 0x0F), rs);
}

static void lcd_command(uint8_t cmd)
{
    lcd_write8bits(cmd, 0);
    if (cmd == LCD_CLEARDISPLAY || cmd == LCD_RETURNHOME) {
        systick_delay_ms(2);
    } else {
        lcd_delay_short();
    }
}

void lcd_init(void)
{
    systick_delay_ms(50);

    lcd_write4bits(0x03, 0);
    systick_delay_ms(5);

    lcd_write4bits(0x03, 0);
    systick_delay_ms(1);

    lcd_write4bits(0x03, 0);
    systick_delay_ms(1);

    lcd_write4bits(0x02, 0);
    systick_delay_ms(1);

    lcd_command(LCD_FUNCTIONSET | LCD_2LINE);
    lcd_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
    lcd_command(LCD_CLEARDISPLAY);
    lcd_command(LCD_ENTRYMODESET | LCD_ENTRYLEFT);
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
    const uint8_t row_offsets[] = {0x00, 0x40};

    if (row > 1) row = 1;
    lcd_command((uint8_t)(LCD_SETDDRAMADDR | (col + row_offsets[row])));
}

void lcd_send_string(const char *str)
{
    while (*str) {
        lcd_write8bits((uint8_t)(*str++), 1);
    }
}

void lcd_backlight(uint8_t state)
{
    lcd_backlight_mask = state ? LCD_BL : 0x00;
    uint8_t data = lcd_backlight_mask;
    lcd_i2c_write(data);
}

void lcd_clear(void)
{
    lcd_command(LCD_CLEARDISPLAY);
}