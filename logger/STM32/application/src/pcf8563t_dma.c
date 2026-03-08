#include "pcf8563t_dma.h"
#include "i2c_dma.h"

/* ============================================================================
 * PCF8563 / PCF8563T (RTC, I2C) – DRIVER DOCUMENTATION (polling)
 * ============================================================================
 *
 * 1) OVERVIEW
 * ----------------------------------------------------------------------------
 * PCF8563 is a low-power Real-Time Clock with I2C interface. This driver
 * supports:
 *   - date/time set & get (seconds..year)
 *   - VL (Voltage Low) flag read (time may be invalid after brown-out)
 *   - CLKOUT output (e.g. 1 Hz square wave)
 *   - Alarm configuration (minute/hour/day/weekday) + AF flag polling/clear
 *
 * This driver is designed to work WITHOUT MCU interrupt wiring (INT pin may be
 * not connected). Alarm can be handled by polling the AF flag in CTRL2.
 *
 *
 * 2) I2C ADDRESS
 * ----------------------------------------------------------------------------
 *   PCF8563_I2C_ADDR = 0x51
 *
 *
 * 3) REGISTER MAP USED
 * ----------------------------------------------------------------------------
 * CTRL:
 *   REG_CTRL1   0x00  (normal mode, STOP bit etc.)
 *   REG_CTRL2   0x01  (AIE/AF bits for alarm)
 *
 * DATETIME (BCD):
 *   REG_SECONDS 0x02  bit7 = VL (Voltage Low)
 *   REG_MINUTES 0x03
 *   REG_HOURS   0x04
 *   REG_DAY     0x05
 *   REG_WEEKDAY 0x06
 *   REG_MONTH   0x07  bit7 = century (not used here, kept 0)
 *   REG_YEAR    0x08  00..99
 *
 * ALARM (BCD + AE bit7):
 *   REG_ALRM_MIN  0x09  bit7 = AE (1 disables compare for this field)
 *   REG_ALRM_HOUR 0x0A  bit7 = AE
 *   REG_ALRM_DAY  0x0B  bit7 = AE
 *   REG_ALRM_WDAY 0x0C  bit7 = AE
 *
 * CLKOUT:
 *   REG_CLKOUT 0x0D
 *
 *
 * 4) BCD ENCODING
 * ----------------------------------------------------------------------------
 * All time/date registers are stored in BCD. This driver provides:
 *   - dec_to_bcd(): decimal -> BCD
 *   - bcd_to_dec(): BCD -> decimal
 *
 *
 * 5) VL FLAG (Voltage Low)
 * ----------------------------------------------------------------------------
 * REG_SECONDS bit7 (VL) indicates the oscillator may have stopped due to low
 * supply, and the time may be invalid.
 *
 * Usage:
 *   if (pcf8563t_get_vl_flag()) {
 *       // time may be invalid -> set time again
 *   }
 *
 * Note:
 * - When setting seconds, this driver clears bit7 (VL) by masking 0x7F.
 *
 *
 * 6) DATETIME SET/GET
 * ----------------------------------------------------------------------------
 * Set:
 *   pcf8563t_set_datetime(sec, min, hour, day, wday, month, year);
 *
 * Input ranges (recommended):
 *   sec   0..59
 *   min   0..59
 *   hour  0..23
 *   day   1..31
 *   wday  0..6   (your convention)
 *   month 1..12
 *   year  0..99  (last two digits)
 *
 * Get:
 *   pcf8563t_get_datetime(&sec, &min, &hour, &day, &wday, &month, &year);
 *
 * Returned values are in decimal. Driver masks out control bits:
 *   seconds: &0x7F (VL removed)
 *   minutes: &0x7F
 *   hours  : &0x3F
 *   day    : &0x3F
 *   weekday: &0x07
 *   month  : &0x1F (century removed)
 *
 *
 * 7) CLKOUT (SQUARE WAVE OUTPUT) – 1 Hz BLINK
 * ----------------------------------------------------------------------------
 * PCF8563 can output a square wave on CLKOUT pin. This is useful for:
 *   - LED blinking
 *   - RTC alive / debug
 *   - simple timing reference
 *
 * Enable 1 Hz:
 *   pcf8563t_clkout_1hz_enable();
 *
 * Generic control:
 *   pcf8563t_clkout_set(enable, freq);
 *
 * Frequencies (FD bits) typically:
 *   32.768 kHz, 1.024 kHz, 32 Hz, 1 Hz
 *
 *
 * 8) ALARM (NO MCU INTERRUPTS REQUIRED)
 * ----------------------------------------------------------------------------
 * Alarm compares current time fields against alarm registers.
 * Alarm fields:
 *   minute, hour, day, weekday
 *
 * Each alarm register has AE (bit7):
 *   AE = 0 -> field is used in compare
 *   AE = 1 -> field ignored ("don't care")
 *
 * This driver API uses:
 *   - decimal values for fields
 *   - value 0xFF means "ignore this field" (AE=1)
 *
 * Set alarm:
 *   pcf8563t_alarm_set(minute, hour, day, weekday);
 *
 * Examples:
 *   // Daily at 07:30
 *   pcf8563t_alarm_set(30, 7, 0xFF, 0xFF);
 *
 *   // Every hour at minute 00
 *   pcf8563t_alarm_set(0, 0xFF, 0xFF, 0xFF);
 *
 * Enable/disable alarm (AIE bit in CTRL2):
 *   pcf8563t_alarm_enable(1);  // enable
 *   pcf8563t_alarm_enable(0);  // disable
 *
 * Poll alarm event (AF flag in CTRL2):
 *   if (pcf8563t_alarm_fired()) {
 *       pcf8563t_alarm_clear_flag(); // MUST clear AF after handling
 *       // ... handle alarm event ...
 *   }
 *
 * Notes:
 * - Even if INT pin is not connected, AF flag will be set internally.
 * - AF does NOT clear automatically; firmware must clear it.
 *
 *
 * 9) TYPICAL POLLING WORKFLOW
 * ----------------------------------------------------------------------------
 * pcf8563t_init();
 *
 * if (pcf8563t_get_vl_flag()) {
 *     // set a known valid time (example)
 *     pcf8563t_set_datetime(0, 0, 12, 1, 0, 1, 26);
 * }
 *
 * // Optional: enable 1 Hz on CLKOUT
 * pcf8563t_clkout_1hz_enable();
 *
 * // Optional: set & enable alarm
 * pcf8563t_alarm_set(30, 7, 0xFF, 0xFF);
 * pcf8563t_alarm_enable(1);
 *
 * while (1) {
 *     if (pcf8563t_alarm_fired()) {
 *         pcf8563t_alarm_clear_flag();
 *         // do something
 *     }
 * }
 *
 * ============================================================================
 */

#define PCF8563_I2C_ADDR  0x51
#define REG_CTRL1         0x00
#define REG_CTRL2         0x01
#define REG_SECONDS       0x02
#define REG_MINUTES       0x03
#define REG_HOURS         0x04
#define REG_DAY           0x05
#define REG_WEEKDAY       0x06
#define REG_MONTH         0x07
#define REG_YEAR          0x08
#define REG_ALRM_MIN      0x09
#define REG_ALRM_HOUR     0x0A
#define REG_ALRM_DAY      0x0B
#define REG_ALRM_WDAY     0x0C
#define REG_CLKOUT        0x0D
#define CTRL2_AIE         (1u << 1)
#define CTRL2_AF          (1u << 3)
#define SECONDS_VL        (1u << 7)
#define ALRM_AE           (1u << 7)
#define CLKOUT_FE         (1u << 7)
#define CLKOUT_FD_MASK    (0x03u)

static uint8_t dec_to_bcd(uint8_t val) { return (uint8_t)(((val / 10u) << 4) | (val % 10u)); }
static uint8_t bcd_to_dec(uint8_t val) { return (uint8_t)(((val >> 4) * 10u) + (val & 0x0Fu)); }
static uint8_t u8_min(uint8_t a, uint8_t b) { return (a < b) ? a : b; }

static int pcf8563_read_regs(uint8_t start_reg, uint8_t *data, uint8_t len)
{
    if (!data || len == 0) return -1;

    if (i2c1_write_raw(PCF8563_I2C_ADDR, &start_reg, 1) != 1) return -1;
    if (i2c1_read_raw(PCF8563_I2C_ADDR, data, len) != 1) return -1;

    return 1;
}

static int pcf8563_write_regs(uint8_t start_reg, const uint8_t *data, uint8_t len)
{
    if (!data || len == 0) return -1;

    if (len > 15u) return -1;

    uint8_t buf[1u + 15u];
    buf[0] = start_reg;
    for (uint8_t i = 0; i < len; i++) buf[1u + i] = data[i];

    return (i2c1_write_raw(PCF8563_I2C_ADDR, buf, (uint8_t)(1u + len)) == 1) ? 1 : -1;
}

void pcf8563t_init(void)
{
    uint8_t ctrl[2] = {0x00, 0x00};
    (void)pcf8563_write_regs(REG_CTRL1, ctrl, 2);
}

uint8_t pcf8563t_get_vl_flag(void)
{
    uint8_t sec = 0;
    if (pcf8563_read_regs(REG_SECONDS, &sec, 1) != 1) return 1u;
    return (sec & SECONDS_VL) ? 1u : 0u;
}

void pcf8563t_set_datetime(uint8_t seconds, uint8_t minutes, uint8_t hours,
                           uint8_t day, uint8_t weekday,
                           uint8_t month, uint8_t year)
{
    seconds = u8_min(seconds, 59);
    minutes = u8_min(minutes, 59);
    hours   = u8_min(hours,   23);
    day     = (day == 0) ? 1 : u8_min(day, 31);
    weekday = u8_min(weekday, 6);
    month   = (month == 0) ? 1 : u8_min(month, 12);

    uint8_t data[7];
    data[0] = (uint8_t)(dec_to_bcd(seconds) & 0x7Fu);
    data[1] = (uint8_t)(dec_to_bcd(minutes) & 0x7Fu);
    data[2] = (uint8_t)(dec_to_bcd(hours)   & 0x3Fu);
    data[3] = (uint8_t)(dec_to_bcd(day)     & 0x3Fu);
    data[4] = (uint8_t)(dec_to_bcd(weekday) & 0x07u);
    data[5] = (uint8_t)(dec_to_bcd(month)   & 0x1Fu);
    data[6] = dec_to_bcd(year);

    (void)pcf8563_write_regs(REG_SECONDS, data, 7);
}

int pcf8563t_get_datetime(uint8_t *seconds, uint8_t *minutes, uint8_t *hours,
                          uint8_t *day, uint8_t *weekday,
                          uint8_t *month, uint8_t *year)
{
    uint8_t data[7] = {0};

    if (pcf8563_read_regs(REG_SECONDS, data, 7) != 1) {
        if (seconds) *seconds = 0xFF;
        if (minutes) *minutes = 0xFF;
        if (hours)   *hours   = 0xFF;
        if (day)     *day     = 0xFF;
        if (weekday) *weekday = 0xFF;
        if (month)   *month   = 0xFF;
        if (year)    *year    = 0xFF;
        return -1;
    }

    if (seconds) *seconds = bcd_to_dec((uint8_t)(data[0] & 0x7Fu));
    if (minutes) *minutes = bcd_to_dec((uint8_t)(data[1] & 0x7Fu));
    if (hours)   *hours   = bcd_to_dec((uint8_t)(data[2] & 0x3Fu));
    if (day)     *day     = bcd_to_dec((uint8_t)(data[3] & 0x3Fu));
    if (weekday) *weekday = bcd_to_dec((uint8_t)(data[4] & 0x07u));
    if (month)   *month   = bcd_to_dec((uint8_t)(data[5] & 0x1Fu));
    if (year)    *year    = bcd_to_dec(data[6]);

    return 1;
}

void pcf8563t_clkout_set(uint8_t enable, pcf8563_clkout_freq_t freq)
{
    uint8_t v;

    if (enable) v = (uint8_t)(CLKOUT_FE | ((uint8_t)freq & CLKOUT_FD_MASK));
    else        v = 0x00;

    (void)pcf8563_write_regs(REG_CLKOUT, &v, 1);
}

void pcf8563t_clkout_1hz_enable(void)
{
    pcf8563t_clkout_set(1, PCF8563_CLKOUT_1HZ);
}

void pcf8563t_alarm_set(uint8_t minute, uint8_t hour, uint8_t day, uint8_t weekday)
{
    uint8_t a[4];

    if (minute == 0xFF) a[0] = ALRM_AE;
    else                a[0] = (uint8_t)(dec_to_bcd(u8_min(minute, 59)) & 0x7Fu);

    if (hour == 0xFF)   a[1] = ALRM_AE;
    else                a[1] = (uint8_t)(dec_to_bcd(u8_min(hour, 23)) & 0x3Fu);

    if (day == 0xFF)    a[2] = ALRM_AE;
    else {
        uint8_t d = day;
        if (d == 0) d = 1;
        a[2] = (uint8_t)(dec_to_bcd(u8_min(d, 31)) & 0x3Fu);
    }

    if (weekday == 0xFF) a[3] = ALRM_AE;
    else                 a[3] = (uint8_t)(dec_to_bcd(u8_min(weekday, 6)) & 0x07u);

    (void)pcf8563_write_regs(REG_ALRM_MIN, a, 4);
}

void pcf8563t_alarm_enable(uint8_t enable)
{
    uint8_t ctrl2 = 0;

    if (pcf8563_read_regs(REG_CTRL2, &ctrl2, 1) != 1) return;

    ctrl2 &= (uint8_t)~CTRL2_AF;

    if (enable) ctrl2 |= CTRL2_AIE;
    else        ctrl2 &= (uint8_t)~CTRL2_AIE;

    (void)pcf8563_write_regs(REG_CTRL2, &ctrl2, 1);
}

uint8_t pcf8563t_alarm_fired(void)
{
    uint8_t ctrl2 = 0;
    if (pcf8563_read_regs(REG_CTRL2, &ctrl2, 1) != 1) return 0;
    return (ctrl2 & CTRL2_AF) ? 1u : 0u;
}

void pcf8563t_alarm_clear_flag(void)
{
    uint8_t ctrl2 = 0;
    if (pcf8563_read_regs(REG_CTRL2, &ctrl2, 1) != 1) return;
    ctrl2 &= (uint8_t)~CTRL2_AF;
    (void)pcf8563_write_regs(REG_CTRL2, &ctrl2, 1);
}