
/**
 * @file lcd_1602_i2c.hpp
 * @brief Interface definitions for controlling a 16x2 HD44780-compatible LCD over I2C (PCF8574 backpack).
 *
 * This header declares command constants and function prototypes used to operate a 16x2 character LCD
 * via an I2C I/O expander. The abstraction supports basic text output, cursor positioning, display
 * clearing, and backlight control.
 *
 * Constants:
 *  - LCD_* command bytes: Bit masks and base opcodes corresponding to HD44780 instruction set
 *    (clear display, return home, entry mode, display control, cursor shift, function set, CGRAM/DDRAM addressing).
 *  - Mode / option flags: Combinations used to build full command bytes (e.g., display on, cursor on, blink on).
 *  - LCD_BACKLIGHT: Bit used to control the backlight through the expander.
 *  - LCD_ENABLE_BIT: Strobe bit for latching data via the expander.
 *  - addr: Default I2C address of the LCD backpack (modifiable if different hardware is used).
 *
 * Macros:
 *  - LCD_CHARACTER / LCD_COMMAND: Indicators distinguishing data vs. command transmissions.
 *  - MAX_LINES / MAX_CHARS: Physical display geometry (2 lines x 16 columns).
 *
 * Public API:
 *  - void lcd_init():
 *      Initialize I2C interface, LCD function mode (lines, font), entry mode, display state, and clear the screen.
 *
 *  - void lcd_clear():
 *      Issue a clear display command and reset DDRAM address. May require a delay (execution time ~1.52 ms).
 *
 *  - void lcd_set_cursor(int col, int row):
 *      Position the cursor at the specified zero-based column and row within display bounds.
 *      Out-of-range inputs should be clamped or ignored by the implementation.
 *
 *  - void lcd_string(const char *text):
 *      Write a null-terminated string starting at current cursor position. Stops at newline, string end,
 *      or when display bounds are reached. Does not automatically wrap unless implemented internally.
 *
 *  - void lcd_set_backlight(bool on):
 *      Enable or disable LCD module backlight via the expander control bit.
 *
 *  - bool lcd_get_backlight():
 *      Return the last persisted backlight state tracked by the driver (may not read hardware state directly).
 *
 * Implementation Notes (expected behavior):
 *  - All low-level writes typically require splitting bytes into high/low nibbles when operating in 4-bit mode via I2C.
 *  - Timing: Certain commands (clear, return home) require longer post-command delays.
 *  - Concurrency: If used in multi-context environments, guard I2C transactions.
 *  - Backlight state is often OR'ed into every transmitted byte to avoid unintended toggling.
 *
 * Usage Example (conceptual):
 *    lcd_init();
 *    lcd_set_backlight(true);
 *    lcd_set_cursor(0, 0);
 *    lcd_string("Hello, world!");
 *    lcd_set_cursor(0, 1);
 *    lcd_string("Temp: 23.4C");
 *
 * Extendability:
 *  - Add functions for custom characters (CGRAM), scrolling, and formatted printing.
 *  - Support different I2C addresses by exposing a setter for 'addr'.
 *
 * Safety:
 *  - Ensure I2C is initialized before any function that transmits.
 *  - Validate pointers (lcd_string) to avoid dereferencing null.
 */
#ifndef __LCD_1602_I2C_HPP__
#define __LCD_1602_I2C_HPP__

const uint8_t LCD_CLEARDISPLAY = 0x01;
const uint8_t LCD_RETURNHOME = 0x02;
const uint8_t LCD_ENTRYMODESET = 0x04;
const uint8_t LCD_DISPLAYCONTROL = 0x08;
const uint8_t LCD_CURSORSHIFT = 0x10;
const uint8_t LCD_FUNCTIONSET = 0x20;
const uint8_t LCD_SETCGRAMADDR = 0x40;
const uint8_t LCD_SETDDRAMADDR = 0x80;
const uint8_t LCD_ENTRYSHIFTINCREMENT = 0x01;
const uint8_t LCD_ENTRYLEFT = 0x02;
const uint8_t LCD_BLINKON = 0x01;
const uint8_t LCD_CURSORON = 0x02;
const uint8_t LCD_DISPLAYON = 0x04;
const uint8_t LCD_MOVERIGHT = 0x04;
const uint8_t LCD_DISPLAYMOVE = 0x08;
const uint8_t LCD_5x10DOTS = 0x04;
const uint8_t LCD_2LINE = 0x08;
const uint8_t LCD_8BITMODE = 0x10;
const uint8_t LCD_BACKLIGHT = 0x08;
const uint8_t LCD_ENABLE_BIT = 0x04;
static uint8_t addr = 0x27;

#define LCD_CHARACTER  1
#define LCD_COMMAND    0
#define MAX_LINES      2
#define MAX_CHARS      16

void lcd_clear();
void lcd_set_cursor(int , int);
void lcd_string(const char *);
void lcd_set_backlight(bool);
bool lcd_get_backlight();
void lcd_init();

#endif /* __LCD_1602_I2C_HPP__ */