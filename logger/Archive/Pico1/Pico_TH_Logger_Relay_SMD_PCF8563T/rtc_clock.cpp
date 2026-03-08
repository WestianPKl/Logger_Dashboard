#include "rtc_clock.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

/**
 * Converts an unsigned decimal value to packed BCD (Binary-Coded Decimal).
 *
 * Recommended input range: 0–99.
 * For values >= 100, only the last two decimal digits are encoded (val % 100).
 * The tens digit is placed in the upper nibble; the ones digit in the lower nibble.
 *
 * Examples: 7 -> 0x07, 42 -> 0x42.
 * Commonly used when writing time/date fields to RTC registers (e.g., PCF8563T).
 *
 * @param val Decimal value to convert.
 * @return Packed BCD representation of the input.
 */
static inline uint8_t dec2bcd(uint8_t val) { return ((val / 10) << 4) | (val % 10); }

/**
 * Converts a packed Binary-Coded Decimal (BCD) byte to its decimal value.
 *
 * The input is expected to be in packed BCD format (0x00–0x99), where the high
 * nibble encodes the tens digit (0–9) and the low nibble encodes the ones digit (0–9).
 *
 * @param val Packed BCD byte (e.g., 0x42 represents decimal 42).
 * @return Unsigned 8-bit decimal value in the range 0–99.
 *
 * @note No validation is performed; if either nibble is greater than 9, the result
 *       is not meaningful for non-BCD inputs.
 */
static inline uint8_t bcd2dec(uint8_t val) { return ((val >> 4) * 10) + (val & 0x0F); }

/**
 * @brief Portable equivalent of timegm: convert a UTC broken-down time to time_t.
 *
 * Temporarily sets the process time zone to UTC ("TZ=UTC0"), calls mktime(3) to
 * interpret the provided struct tm as UTC, then restores the original time zone.
 *
 * @param t Pointer to a struct tm representing a UTC time. The structure may be
 *          normalized/modified by mktime (e.g., fields adjusted, tm_isdst set).
 *
 * @return Seconds since the Unix epoch (UTC) on success; (time_t)-1 on failure
 *         if the time cannot be represented.
 *
 * @note Not thread-safe or async-signal-safe: it modifies process-global state
 *       (the TZ environment variable) and calls tzset(3). Prefer native timegm(3)
 *       where available. The argument must not be null.
 */
static time_t timegm_compat(struct tm* t) {
    const char* old_tz = getenv("TZ");
    if (old_tz) old_tz = strdup(old_tz);

    setenv("TZ", "UTC0", 1);
    tzset();
    time_t epoch = mktime(t);

    if (old_tz) {
        setenv("TZ", old_tz, 1);
        free((void*)old_tz);
    } else {
        unsetenv("TZ");
    }
    tzset();
    return epoch;
}

/**
 * @brief Initializes the PCF8563T real-time clock over I2C.
 *
 * Writes default values to Control/Status 1 and 2 registers (0x00) to clear
 * pending flags and ensure the oscillator runs, then disables the 1 Hz CLKOUT output.
 *
 * @param i2c Pointer to an initialized I2C instance used to communicate with the PCF8563T.
 * @return true if both control registers were written successfully; false otherwise.
 *
 * @pre The I2C bus must be initialized and the device reachable at PCF8563_I2C_ADDR.
 * @post The RTC oscillator is running; CLKOUT at 1 Hz is disabled; time/date registers are unchanged.
 *
 * @note This routine relies on I2C ACKs only (no device ID readback) and ignores
 *       any error returned by the CLKOUT configuration helper.
 */
bool pcf8563t_init(i2c_inst_t *i2c){
    uint8_t wr1[] = {REG_CTRL1, 0x00};
    uint8_t wr2[] = {REG_CTRL2, 0x00};
    if (i2c_write_blocking(i2c, PCF8563_I2C_ADDR, wr1, sizeof(wr1), false) != (int)sizeof(wr1)) return false;
    if (i2c_write_blocking(i2c, PCF8563_I2C_ADDR, wr2, sizeof(wr2), false) != (int)sizeof(wr2)) return false;

    pcf8563t_set_clkout_1hz(i2c, false);
    return true;
}

/**
 * @brief Sets the date and time on a PCF8563/PCF8563T RTC via I2C.
 *
 * Converts the provided time fields to BCD and writes the seconds through year
 * registers in a single burst starting at REG_SECONDS. The seconds VL flag is
 * cleared, the weekday is stored as a 3-bit value, and the month register’s
 * century bit (bit 7) is managed automatically:
 *   - year >= 2000: century bit cleared (0), year stored as (year - 2000).
 *   - 1900 <= year < 2000: century bit set (1), year stored as (year - 1900).
 *   - year  < 1900: century bit set (1), year stored as 00.
 *
 * Input validation:
 *   - sec:          0..59
 *   - min:          0..59
 *   - hour:         0..23 (24-hour format)
 *   - day_of_month: 1..31
 *   - month:        1..12
 *   - day_of_week:  0..6 (mapping is application-defined)
 *
 * No cross-field validation is performed (e.g., month/day pairing or leap years).
 * Performs a blocking I2C write; the operation is not re-entrant.
 *
 * @param i2c          Pointer to the I2C instance used for communication.
 * @param sec          Seconds (0–59).
 * @param min          Minutes (0–59).
 * @param hour         Hours (0–23), 24-hour format.
 * @param day_of_week  Day of week (0–6).
 * @param day_of_month Day of month (1–31).
 * @param month        Month (1–12).
 * @param year         Absolute year (e.g., 2025). Intended range: 1900–2099.
 *
 * @return true on success (all bytes written); false if input validation fails
 *         or the I2C write does not transfer the expected number of bytes.
 *
 * @warning Years outside 1900–2099 may produce undefined or unintended values in the RTC.
 */
bool pcf8563t_set_time(i2c_inst_t *i2c, uint sec, uint min, uint hour,
                       uint day_of_week, uint day_of_month,
                       uint month, uint year) {
    if (sec > 59 || min > 59 || hour > 23 ||
        day_of_month < 1 || day_of_month > 31 ||
        month < 1 || month > 12 ||
        day_of_week > 6) {
        return false;
    }

    uint8_t bcd_sec     = dec2bcd((uint8_t)sec)          & 0x7F;
    uint8_t bcd_min     = dec2bcd((uint8_t)min)          & 0x7F;
    uint8_t bcd_hour    = dec2bcd((uint8_t)hour)         & 0x3F;
    uint8_t bcd_day     = dec2bcd((uint8_t)day_of_month) & 0x3F;
    uint8_t bin_wday    = (uint8_t)day_of_week & 0x07;
    uint8_t bcd_month   = dec2bcd((uint8_t)month)        & 0x1F;

    uint year2 = year;
    if (year2 >= 2000) {
        year2 -= 2000;
    } else {
        bcd_month |= 0x80;
        year2 = (year2 >= 1900) ? (year2 - 1900) : 0;
    }
    uint8_t bcd_year = dec2bcd((uint8_t)year2);

    uint8_t frame[1 + 7];
    frame[0] = REG_SECONDS;
    frame[1] = bcd_sec;
    frame[2] = bcd_min;
    frame[3] = bcd_hour;
    frame[4] = bcd_day;
    frame[5] = bin_wday;
    frame[6] = bcd_month;
    frame[7] = bcd_year;

    int wr = i2c_write_blocking(i2c, PCF8563_I2C_ADDR, frame, (int)sizeof(frame), false);
    return wr == (int)sizeof(frame);
}

/**
 * @brief Reads the current time from a PCF8563 RTC over I2C and decodes it to integers.
 *
 * Performs a blocking I2C read of 7 consecutive registers starting at REG_SECONDS and
 * converts BCD-encoded fields into human-readable integers. On success, writes the
 * decoded values into the provided array as follows:
 *   converted_time[0] = seconds (0–59)
 *   converted_time[1] = minutes (0–59)
 *   converted_time[2] = hours   (0–23)
 *   converted_time[3] = day     (1–31)
 *   converted_time[4] = weekday (0–6, as provided by the chip)
 *   converted_time[5] = month   (1–12)
 *   converted_time[6] = year    (full year, e.g., 1999 or 2025; century derived from Month register)
 *
 * The function fails and returns false if:
 *   - converted_time is null,
 *   - the I2C write or read operation fails,
 *   - the oscillator stop (OS) flag is set in the Seconds register (time invalid/unstable).
 *
 * Century handling:
 *   - If the Century bit (bit 7) in the Month register is set, year = 1900 + BCD(Year).
 *   - Otherwise, year = 2000 + BCD(Year).
 *
 * @param i2c Initialized Pico SDK I2C instance connected to the PCF8563 device.
 * @param[out] converted_time Pointer to an array of at least 7 uint16_t elements to receive the decoded time/date.
 * @return true on success; false on error or if the OS flag indicates invalid time.
 *
 * @note This function uses blocking I2C transfers and does not allocate memory.
 * @note Weekday is not BCD-encoded on PCF8563; it is masked to the lower 3 bits.
 */
bool pcf8563t_read_time(i2c_inst_t *i2c, uint16_t *converted_time) {
    if (!converted_time) return false;

    uint8_t addr = REG_SECONDS;
    if (i2c_write_blocking(i2c, PCF8563_I2C_ADDR, &addr, 1, true) != 1) return false;

    uint8_t buffer[7];
    if (i2c_read_blocking(i2c, PCF8563_I2C_ADDR, buffer, 7, false) != 7) return false;

    if (buffer[0] & 0x80) return false;

    converted_time[0] = bcd2dec(buffer[0] & 0x7F);
    converted_time[1] = bcd2dec(buffer[1] & 0x7F);
    converted_time[2] = bcd2dec(buffer[2] & 0x3F);
    converted_time[3] = bcd2dec(buffer[3] & 0x3F); 
    converted_time[4] = (buffer[4] & 0x07);
    converted_time[5] = bcd2dec(buffer[5] & 0x1F);

    if (buffer[5] & 0x80) {
        converted_time[6] = 1900 + bcd2dec(buffer[6]);
    } else {
        converted_time[6] = 2000 + bcd2dec(buffer[6]);
    }
    return true;
}

/**
 * @brief Enable or disable a 1 Hz square-wave on the PCF8563T CLKOUT pin.
 *
 * Writes the CLKOUT control register of the PCF8563T to either enable a 1 Hz
 * output on its CLKOUT pin or disable the output.
 *
 * @param i2c    Pointer to an initialized I2C instance used to communicate with the RTC.
 * @param enable Set to true to enable CLKOUT at 1 Hz; set to false to disable CLKOUT.
 *
 * @pre The I2C bus must be initialized and the PCF8563T must be reachable at PCF8563_I2C_ADDR.
 *
 * @post When enabled and the write succeeds, the CLKOUT pin outputs a 1 Hz square wave.
 *
 * @note This function performs a blocking I2C write and ignores the return value.
 *       If you need error handling, check the result of i2c_write_blocking().
 *
 * @details This call overwrites any previous CLKOUT frequency configuration.
 */
void pcf8563t_set_clkout_1hz(i2c_inst_t *i2c, bool enable) {
    uint8_t val = enable ? (uint8_t)(0x80 | 0x03) : (uint8_t)0x00;
    uint8_t buf[] = {REG_CLKOUT, val};
    (void)i2c_write_blocking(i2c, PCF8563_I2C_ADDR, buf, 2, false);
}

/**
 * Configure the PCF8563 alarm and enable/disable individual comparators.
 *
 * Each alarm field (minute, hour, day-of-month, weekday) can be enabled for comparison
 * or disabled ("don't care"). A field is disabled by passing 0xFF, which sets the field's
 * AEN bit (bit 7). Enabled fields are written in BCD and masked to the device limits
 * (minute: 0–59, hour: 0–23, day: 1–31, weekday: 0–6 per device convention).
 *
 * If use_weekday is true, the weekday comparator is used; otherwise the weekday comparator
 * is disabled regardless of the 'weekday' argument. The day-of-month comparator is controlled
 * solely by the 'day' argument.
 *
 * The function performs a single I2C write starting at REG_ALRM_MIN and does not modify
 * control/status flags. Any pending alarm flags must be cleared separately.
 *
 * @param i2c         Initialized I2C instance to use.
 * @param min         Alarm minute [0–59], or 0xFF to disable minute compare.
 * @param hour        Alarm hour [0–23], or 0xFF to disable hour compare.
 * @param day         Alarm day-of-month [1–31], or 0xFF to disable day compare.
 * @param weekday     Alarm weekday [0–6], or 0xFF to disable; only considered when use_weekday is true.
 * @param use_weekday When true, enable weekday comparison; when false, disable weekday comparison.
 */
void rtc_alarm_set(i2c_inst_t *i2c, uint8_t min, uint8_t hour,
                   uint8_t day, uint8_t weekday, bool use_weekday) {
    uint8_t alarm_min   = (min  == 0xFF) ? 0x80 : (dec2bcd(min)  & 0x7F);
    uint8_t alarm_hour  = (hour == 0xFF) ? 0x80 : (dec2bcd(hour) & 0x3F);
    uint8_t alarm_day   = (day  == 0xFF) ? 0x80 : (dec2bcd(day)  & 0x3F);
    uint8_t alarm_wday  = (weekday == 0xFF) ? 0x80 : (weekday & 0x07);

    uint8_t seq[5] = {
        REG_ALRM_MIN,
        alarm_min,
        alarm_hour,
        alarm_day,
        use_weekday ? (alarm_wday & 0x07) : 0x80
    };
    (void)i2c_write_blocking(i2c, PCF8563_I2C_ADDR, seq, 5, false);
}

/**
 * @brief Enable or disable the PCF8563 alarm interrupt.
 *
 * Reads the Control/Status 2 register (REG_CTRL2) of the PCF8563 RTC, sets or clears
 * the AIE bit (Alarm Interrupt Enable, bit 1), and writes the updated value back.
 * All other bits in the register are preserved.
 *
 * Behavior:
 * - Performs a register address write without a stop, followed by a single-byte read.
 * - If the address or read operation fails, the function returns early without changes.
 * - The final write operation is attempted but its result is not checked.
 * - Does not configure alarm time registers nor clear the AF (Alarm Flag) bit.
 *
 * @pre The I2C peripheral must be initialized and the PCF8563 available at PCF8563_I2C_ADDR.
 *
 * @param i2c    Pointer to the Pico SDK I2C instance to use.
 * @param enable Set to true to enable the alarm interrupt (AIE=1), false to disable it (AIE=0).
 */
void rtc_alarm_enable(i2c_inst_t *i2c, bool enable) {
    uint8_t addr = REG_CTRL2;
    if (i2c_write_blocking(i2c, PCF8563_I2C_ADDR, &addr, 1, true) != 1) return;
    uint8_t val = 0;
    if (i2c_read_blocking(i2c, PCF8563_I2C_ADDR, &val, 1, false) != 1) return;

    if (enable) val |=  (1u << 1);
    else        val &= ~(1u << 1);

    uint8_t wr[] = {REG_CTRL2, val};
    (void)i2c_write_blocking(i2c, PCF8563_I2C_ADDR, wr, 2, false);
}

/**
 * Clears the PCF8563 alarm flag (AF) and reports whether it was previously set.
 *
 * Reads Control/Status 2 (REG_CTRL2) from the PCF8563 via the provided I2C
 * instance, checks bit 3 (AF). If set, writes the register back with AF
 * cleared. Returns whether AF was set before this call.
 *
 * @param i2c Pointer to an initialized I2C instance connected to the PCF8563.
 * @return true if the alarm flag was set prior to the call (an attempt to clear
 *         it is made); false if the flag was not set or if an I2C operation
 *         failed during the address or read phase.
 *
 * @note The AF bit is bit 3 of Control/Status 2 (REG_CTRL2).
 * @warning This function does not distinguish between "flag not set" and I2C
 *          failure; both yield false. It also does not verify that the clearing
 *          write succeeds, so it may return true even if the flag was not
 *          successfully cleared.
 * @pre i2c is non-null, initialized, and configured for PCF8563_I2C_ADDR.
 * @post If AF was set, a write is issued to clear it; otherwise, no write occurs.
 */
bool rtc_alarm_flag_clear(i2c_inst_t *i2c) {
    uint8_t addr = REG_CTRL2;
    if (i2c_write_blocking(i2c, PCF8563_I2C_ADDR, &addr, 1, true) != 1) return false;
    uint8_t val = 0;
    if (i2c_read_blocking(i2c, PCF8563_I2C_ADDR, &val, 1, false) != 1) return false;

    bool was_set = (val & (1u << 3)) != 0;
    if (was_set) {
        val &= ~(1u << 3);
        uint8_t wr[] = {REG_CTRL2, val};
        (void)i2c_write_blocking(i2c, PCF8563_I2C_ADDR, wr, 2, false);
    }
    return was_set;
}

/**
 * Sets the PCF8563T RTC date/time from a Unix epoch value.
 *
 * Converts the supplied Unix time (seconds since 1970-01-01 00:00:00 UTC) to
 * calendar fields using either UTC or the current local time zone, then writes
 * the result to the PCF8563T via I2C.
 *
 * Parameters:
 * - i2c       Pointer to the initialized I2C instance connected to the PCF8563T.
 * - epoch_utc Unix timestamp in seconds since the Unix epoch (UTC).
 * - as_local  If true, converts the epoch to local civil time (localtime_r) and
 *             programs the RTC with local time; if false, uses UTC (gmtime_r).
 *
 * Returns:
 * - true on success; false if time conversion fails or the underlying RTC write fails.
 *
 * Notes:
 * - When as_local is true, the current process time zone and DST settings are honored.
 * - The valid timestamp range is limited by the platform’s time_t and localtime_r/gmtime_r,
 *   as well as the calendar range supported by the PCF8563T.
 * - Writes second, minute, hour, day-of-week, day-of-month, month, and year fields;
 *   sub-second precision is not supported.
 * - Thread-safe: uses reentrant time conversion functions and no internal static state.
 */
bool pcf8563t_set_time_epoch(i2c_inst_t* i2c, time_t epoch_utc, bool as_local) {
    struct tm tmv;
    memset(&tmv, 0, sizeof(tmv));
    if (as_local) {
        if (!localtime_r(&epoch_utc, &tmv)) return false;
    } else {
        if (!gmtime_r(&epoch_utc, &tmv)) return false;
    }
    uint y   = (uint)(tmv.tm_year + 1900);
    uint mon = (uint)(tmv.tm_mon + 1);
    uint dom = (uint)tmv.tm_mday;
    uint dow = (uint)tmv.tm_wday;
    uint hh  = (uint)tmv.tm_hour;
    uint mm  = (uint)tmv.tm_min;
    uint ss  = (uint)tmv.tm_sec;
    return pcf8563t_set_time(i2c, ss, mm, hh, dow, dom, mon, y);
}

/**
 * @brief Read the current date/time from a PCF8563T RTC and return it as Unix epoch seconds (UTC).
 *
 * This function queries the PCF8563T over the given I2C instance, assembles a struct tm
 * from the RTC fields, and converts it to a time_t epoch value. The value written to
 * out_epoch_utc is always seconds since 1970-01-01 00:00:00 UTC, regardless of how the
 * RTC fields are interpreted.
 *
 * @param i2c               Initialized I2C instance used to communicate with the PCF8563T.
 *                          Must not be null and must be configured for the RTC bus.
 * @param out_epoch_utc     Output pointer that receives the resulting Unix epoch (UTC).
 *                          Must not be null. Updated only on success.
 * @param fields_are_local  If true, interpret the RTC fields as local time and convert
 *                          using mktime() (affected by the process time zone/DST settings).
 *                          If false, interpret the RTC fields as UTC and convert using
 *                          timegm_compat(). In both cases, the returned epoch is UTC.
 *
 * @return true on success; false on error (null output pointer, I2C read failure, or
 *         failed time conversion).
 *
 * @note The conversion path depends on fields_are_local:
 *       - fields_are_local == true: mktime() is used and is sensitive to the current
 *         time zone and DST configuration of the process/environment.
 *       - fields_are_local == false: timegm_compat() interprets the fields as UTC.
 *
 * @warning The function treats a conversion result of (time_t)-1 as failure. If your
 *          platform represents a legitimate timestamp as -1, this may be indistinguishable
 *          from a conversion error.
 *
 * @pre The PCF8563T must be present on the I2C bus and responding. If fields_are_local
 *      is true, ensure the environment time zone is configured as expected.
 *
 * @post On success, *out_epoch_utc contains the UTC epoch seconds. On failure, the
 *       value pointed to by out_epoch_utc is not modified.
 *
 * @thread_safety The function does not use static state, but the mktime() path depends
 *                on process-global time zone settings and may be affected by concurrent
 *                changes to TZ or tzset().
 *
 * @see pcf8563t_read_time(), timegm_compat(), mktime()
 */
bool pcf8563t_read_time_epoch(i2c_inst_t* i2c, time_t* out_epoch_utc, bool fields_are_local) {
    if (!out_epoch_utc) return false;
    uint16_t t[7];
    if (!pcf8563t_read_time(i2c, t)) return false;

    struct tm tmv;
    memset(&tmv, 0, sizeof(tmv));
    tmv.tm_year = (int)t[6] - 1900;
    tmv.tm_mon  = (int)t[5] - 1;
    tmv.tm_mday = (int)t[3];
    tmv.tm_hour = (int)t[2];
    tmv.tm_min  = (int)t[1];
    tmv.tm_sec  = (int)t[0];
    tmv.tm_wday = (int)t[4];

    time_t epoch = fields_are_local ? mktime(&tmv) : timegm_compat(&tmv);
    if (epoch == (time_t)-1) return false;
    *out_epoch_utc = epoch;
    return true;
}