#include "lcd.h"
#include "i2c.h"
#include "support.h"

#define LCD_I2C_ADDR 0x27U /* 7-bit address */

#define LCD_RS   0x01U
#define LCD_RW   0x02U
#define LCD_E    0x04U
#define LCD_BL   0x08U

#define LCD_CLEARDISPLAY   0x01U
#define LCD_RETURNHOME     0x02U
#define LCD_ENTRYMODESET   0x04U
#define LCD_DISPLAYCONTROL 0x08U
#define LCD_FUNCTIONSET    0x20U
#define LCD_SETDDRAMADDR   0x80U

#define LCD_ENTRYLEFT      0x02U
#define LCD_DISPLAYON      0x04U
#define LCD_2LINE          0x08U

static uint8_t lcd_present = 1U;
static uint8_t lcd_backlight_mask = LCD_BL;

static int8_t lcd_i2c_write(uint8_t data)
{
    int r;

    if (!lcd_present) return -1;

    r = i2c1_write_raw(LCD_I2C_ADDR, &data, 1U);
    if (r != 1) {
        lcd_present = 0U;
    }

    return (int8_t)r;
}

static inline void lcd_delay_short(void)
{
    for (volatile int i = 0; i < 200; i++) {
        __NOP();
    }
}

static void lcd_pulse_enable(uint8_t data)
{
    if (!lcd_present) return;

    if (lcd_i2c_write((uint8_t)(data | LCD_E)) != 1) return;
    lcd_delay_short();
    (void)lcd_i2c_write((uint8_t)(data & (uint8_t)~LCD_E));
    lcd_delay_short();
}

static void lcd_write4bits(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (uint8_t)((nibble & 0x0FU) << 4);

    if (rs) {
        data |= LCD_RS;
    }

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
    lcd_write8bits(cmd, 0U);

    if ((cmd == LCD_CLEARDISPLAY) || (cmd == LCD_RETURNHOME)) {
        HAL_Delay(2U);
    } else {
        lcd_delay_short();
    }
}

static int32_t x100_to_x10_round(int32_t v_x100)
{
    if (v_x100 >= 0) {
        return (v_x100 + 5) / 10;
    } else {
        return (v_x100 - 5) / 10;
    }
}

static void lcd_send_fixed_x10(int32_t v_x10, uint8_t int_digits)
{
    int32_t ip;
    int32_t fp;

    if (!lcd_present) return;

    ip = v_x10 / 10;
    fp = v_x10 % 10;
    if (fp < 0) fp = -fp;

    lcd_send_decimal(ip, int_digits);
    lcd_send_string(".");
    lcd_send_decimal(fp, 1U);
}

uint8_t lcd_is_present(void)
{
    return lcd_present;
}

void lcd_mark_present(uint8_t present)
{
    lcd_present = present ? 1U : 0U;
}

void lcd_init(void)
{
    lcd_present = 1U;
    lcd_backlight_mask = LCD_BL;

    if (lcd_i2c_write(lcd_backlight_mask) != 1) {
        lcd_present = 0U;
        return;
    }

    HAL_Delay(50U);

    lcd_write4bits(0x03U, 0U);
    HAL_Delay(5U);

    lcd_write4bits(0x03U, 0U);
    HAL_Delay(1U);

    lcd_write4bits(0x03U, 0U);
    HAL_Delay(1U);

    lcd_write4bits(0x02U, 0U);
    HAL_Delay(1U);

    lcd_command((uint8_t)(LCD_FUNCTIONSET | LCD_2LINE));
    lcd_command((uint8_t)(LCD_DISPLAYCONTROL | LCD_DISPLAYON));
    lcd_command(LCD_CLEARDISPLAY);
    lcd_command((uint8_t)(LCD_ENTRYMODESET | LCD_ENTRYLEFT));
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
    static const uint8_t row_offsets[] = {0x00U, 0x40U};

    if (!lcd_present) return;
    if (row > 1U) row = 1U;

    lcd_command((uint8_t)(LCD_SETDDRAMADDR | (col + row_offsets[row])));
}

void lcd_send_string(const char *str)
{
    if ((!lcd_present) || (str == NULL)) return;

    while (*str != '\0') {
        lcd_write8bits((uint8_t)(*str), 1U);
        str++;
    }
}

void lcd_send_decimal(int32_t num, uint8_t digits)
{
    char buf[12];
    char temp[10];
    int idx = 0;
    int temp_idx = 0;
    int32_t n = num;

    if (!lcd_present) return;

    if (num < 0) {
        buf[idx++] = '-';
        n = -n;
    }

    do {
        temp[temp_idx++] = (char)('0' + (n % 10));
        n /= 10;
    } while ((n > 0) && (temp_idx < (int)sizeof(temp)));

    while (temp_idx < digits) {
        temp[temp_idx++] = '0';
    }

    for (int i = temp_idx - 1; i >= 0; i--) {
        buf[idx++] = temp[i];
    }

    buf[idx] = '\0';
    lcd_send_string(buf);
}

void lcd_send_hex(uint32_t num, uint8_t digits)
{
    char buf[9];
    int idx = 0;
    int start;

    if (!lcd_present) return;
    if (digits == 0U) return;

    start = (digits > 8U) ? 7 : ((int)digits - 1);

    for (int i = start; i >= 0; i--) {
        uint8_t nibble = (uint8_t)((num >> (i * 4)) & 0x0FU);
        buf[idx++] = (char)((nibble < 10U) ? ('0' + nibble) : ('A' + nibble - 10U));
    }

    buf[idx] = '\0';
    lcd_send_string(buf);
}

void lcd_send_fixed_x100(int32_t value_x100)
{
    int32_t ipart;
    int32_t fpart;

    if (!lcd_present) return;

    ipart = value_x100 / 100;
    fpart = value_x100 % 100;
    if (fpart < 0) fpart = -fpart;

    lcd_send_decimal(ipart, 1U);
    lcd_send_string(".");
    lcd_send_decimal(fpart, 2U);
}

void lcd_send_temp_1dp_from_x100(int16_t t_x100)
{
    int32_t t = (int32_t)t_x100;
    int32_t t_x10;

    if (!lcd_present) return;

    if (t < -5000) t = -5000;
    if (t > 12000) t = 12000;

    t_x10 = x100_to_x10_round(t);
    lcd_send_fixed_x10(t_x10, 1U);
}

void lcd_send_hum_1dp_from_x100(uint16_t rh_x100)
{
    uint32_t rh = rh_x100;
    int32_t rh_x10;
    int32_t ip;
    uint8_t digits;

    if (!lcd_present) return;

    if (rh > 10000U) rh = 10000U;

    rh_x10 = (int32_t)((rh + 5U) / 10U);
    ip = rh_x10 / 10;
    digits = (ip >= 100) ? 3U : 2U;

    lcd_send_fixed_x10(rh_x10, digits);
}

void lcd_send_press_int_from_q24_8(uint32_t p_q24_8)
{
    uint32_t p_hpa;

    if (!lcd_present) return;

    p_hpa = (p_q24_8 + 128U) / 256U;
    lcd_send_decimal((int32_t)p_hpa, 5U);
}

void lcd_backlight(uint8_t state)
{
    uint8_t data;

    if (!lcd_present) return;

    lcd_backlight_mask = state ? LCD_BL : 0x00U;
    data = lcd_backlight_mask;
    (void)lcd_i2c_write(data);
}

void lcd_clear(void)
{
    if (!lcd_present) return;
    lcd_command(LCD_CLEARDISPLAY);
}