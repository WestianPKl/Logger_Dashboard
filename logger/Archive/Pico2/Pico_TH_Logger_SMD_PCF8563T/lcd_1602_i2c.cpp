#include <string.h>
#include "hardware/i2c.h"
#include "lcd_1602_i2c.hpp"

static volatile bool s_backlight_on = true;

void i2c_write_byte(uint8_t);
void lcd_toggle_enable(uint8_t);
void lcd_send_byte(uint8_t , int);
static inline void lcd_char(char);

/**
 * @brief Writes a single byte to the current I2C slave using a blocking transfer.
 *
 * Sends one byte on the default I2C controller (i2c_default) to the device at the
 * address specified by the global/static variable `addr`. A STOP condition is issued
 * at the end of the transfer.
 *
 * Compilation:
 * - If i2c_default is not defined at compile time, this function compiles to a no-op.
 *
 * Preconditions:
 * - i2c_default is defined and the I2C peripheral has been initialized (e.g., i2c_init and pin setup).
 * - `addr` contains a valid 7-bit I2C slave address.
 *
 * Behavior:
 * - Blocking; execution halts until the byte is transmitted or the transfer fails.
 *
 * Error handling:
 * - No status is returned to the caller; transmission errors are not reported by this function.
 *   Use the underlying I2C API directly if you need error/status information.
 *
 * Thread-safety:
 * - Not thread-safe for concurrent use on the same I2C instance without external synchronization.
 *
 * @param val The 8-bit value to transmit on the I2C bus.
 */
void i2c_write_byte(uint8_t val) {
#ifdef i2c_default
    i2c_write_blocking(i2c_default, addr, &val, 1, false);
#endif
}

/**
 * @brief Generate an enable pulse to latch the current LCD data/command nibble via I2C.
 *
 * This function briefly asserts and then clears the LCD enable (EN) bit while preserving
 * the other control/data bits provided in val. It performs two I2C writes separated by
 * microsecond-scale delays to satisfy HD44780-compatible timing requirements.
 *
 * @param val The base byte to send to the I2C expander (data nibble, RS, backlight, etc.).
 *            The function will OR this value with LCD_ENABLE_BIT to set EN high, then clear
 *            the bit to bring EN low and latch the nibble.
 *
 * @pre I2C has been initialized and the LCD is configured. The desired data or command nibble
 *      must already be encoded into val before calling.
 *
 * @note Introduces blocking delays of roughly 600 µs before, between, and after the EN pulse
 *       (~1.8 ms total). Typically called once per nibble (twice per byte in 4-bit mode).
 *
 * @warning This call performs blocking waits and I2C transactions; ensure it is not used in
 *          time-critical paths without consideration of the added latency.
 */
void lcd_toggle_enable(uint8_t val) {
#define DELAY_US 600
    sleep_us(DELAY_US);
    i2c_write_byte(val | LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
    i2c_write_byte(val & ~LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
}

/**
 * @brief Send an 8-bit value to an HD44780-compatible LCD over I2C in 4-bit mode.
 *
 * Splits the byte into high and low nibbles (MSB first), ORs each with the provided
 * control mode and LCD_BACKLIGHT, writes them over I2C, and pulses the enable line
 * after each nibble to latch the transfer.
 *
 * @param val  The 8-bit value to transmit (command or character data).
 * @param mode Control bits to OR with the data; typically includes RS
 *             (0 = command, 1 = data). RW must be 0 (write). The E line is
 *             driven internally by lcd_toggle_enable().
 *
 * @pre The I2C interface and the LCD’s I2C expander/backpack are initialized.
 * @note This function always ORs LCD_BACKLIGHT into the transfer and does not alter
 *       backlight state beyond that.
 * @see lcd_toggle_enable(), i2c_write_byte()
 */
void lcd_send_byte(uint8_t val, int mode) {
    uint8_t bl = s_backlight_on ? LCD_BACKLIGHT : 0;
    uint8_t high = mode | (val & 0xF0) | bl;
    uint8_t low = mode | ((val << 4) & 0xF0) | bl;
    i2c_write_byte(high);
    lcd_toggle_enable(high);
    i2c_write_byte(low);
    lcd_toggle_enable(low);
}

/**
 * @brief Clears the LCD display.
 *
 * Sends the HD44780-compatible "clear display" command via I2C, removing all
 * characters from the screen and returning the cursor to the home position
 * (row 0, column 0).
 *
 * Note:
 * - This instruction takes longer than most LCD commands (typically ~1.5–2 ms).
 *   Ensure the command path handles the required delay or busy-flag polling.
 *
 * Preconditions:
 * - The LCD interface has been initialized and is ready to accept commands.
 *
 * Side effects:
 * - Display memory is cleared and the cursor position is reset to home.
 */
void lcd_clear() {
    lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND);
}

/**
 * @brief Set the LCD cursor to a given line and column.
 *
 * Computes and sends the HD44780 DDRAM address for the requested position
 * (0x80 for line 0, 0xC0 for line 1, plus the column offset) as a command.
 *
 * @param line Line index: 0 = first line, 1 = second line.
 * @param position Column index on the selected line (typically 0–15 for a 16x2 LCD).
 * @pre The LCD must be initialized and ready to accept commands.
 * @note Out-of-range values may wrap per the controller’s addressing behavior.
 * @see lcd_send_byte, LCD_COMMAND
 */
void lcd_set_cursor(int line, int position) {
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(val, LCD_COMMAND);
}

/**
 * @brief Write a single character to the LCD at the current cursor position.
 *
 * Sends the given byte as character data (not a command) to the display.
 *
 * @param val Character to write (ASCII or CGRAM code). Control characters (e.g., '\n') are not interpreted.
 *
 * @pre The LCD and I2C interface must be initialized and ready.
 * @note Cursor movement and wrapping follow the LCD controller's current entry mode settings.
 */
static inline void lcd_char(char val) {
    lcd_send_byte(val, LCD_CHARACTER);
}

/**
 * Writes a null-terminated C string to the LCD, sending one character at a time via lcd_char().
 *
 * Each byte from the input buffer is forwarded to lcd_char(), which is responsible for any
 * device-specific timing, cursor movement, and character mapping.
 *
 * Complexity: O(n), where n is the number of characters in the string.
 *
 * @pre s != nullptr and points to a null-terminated string.
 * @param s Pointer to the null-terminated string to display.
 *
 * @warning Passing a null pointer results in undefined behavior.
 * @note Multi-byte encodings (e.g., UTF-8) are sent as raw bytes and may not render correctly
 *       on HD44780-compatible 16x2 displays.
 * @see lcd_char
 */
void lcd_string(const char *s) {
    while (*s) {
        lcd_char(*s++);
    }
}

/**
 * @brief Initialize the HD44780-compatible 16x2 LCD over I2C.
 *
 * Configures the controller for 4-bit operation, sets 2-line mode,
 * enables left-to-right entry mode, turns the display on, and clears
 * the screen, returning the cursor to the home position.
 *
 * This follows the standard HD44780 4-bit initialization sequence and
 * relies on lcd_send_byte() to perform command writes and required delays.
 *
 * @pre I2C bus and any I/O expander/backpack required by the LCD are initialized
 *      and configured; lcd_send_byte() and lcd_clear() are functional.
 * @post Display DDRAM is cleared; cursor is at (0,0); display is enabled.
 *
 * @note Call once after power-up and before any other LCD API calls.
 * @warning This operation erases any existing display contents.
 * @see lcd_send_byte(), lcd_clear()
 */
void lcd_init() {
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x02, LCD_COMMAND);
    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
    lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);
    lcd_clear();
}

/**
 * @brief Enable or disable the LCD backlight.
 *
 * Updates the internal backlight state flag and writes the appropriate control
 * value over I2C to physically switch the LCD module's backlight on or off.
 *
 * @param on true to turn the backlight on, false to turn it off.
 *
 * @note This function performs an immediate I2C write; ensure the I2C bus is
 *       initialized prior to calling. If multiple tasks/threads may access the
 *       display, external synchronization should be applied around calls to
 *       this function to avoid interleaved I2C transactions.
 */
void lcd_set_backlight(bool on) {
    s_backlight_on = on;
    uint8_t val = on ? LCD_BACKLIGHT : 0x00;
    i2c_write_byte(val);
}

/**
 * Retrieves the current state of self.i2c.writeto(self.i2c_addr, bytes([0]))the LCD backlight.
 *
 * This accessor returns the last known backlight state tracked internally
 * (i.e., whether the backlight is considered ON or OFF). It does not
 * actively query the hardware; instead, it reflects the value maintained
 * by the driver logic (s_backlight_on).
 *
 * Returns:
 *   true  - the driver considers the backlight enabled
 *   false - the driver considers the backlight disabled
 *
 * Thread-safety:
 *   If the backlight state may be modified from multiple contexts (e.g.,
 *   ISR vs main loop), ensure appropriate synchronization around writes
 *   to the underlying state variable.
 */
bool lcd_get_backlight() {
    return s_backlight_on;
}