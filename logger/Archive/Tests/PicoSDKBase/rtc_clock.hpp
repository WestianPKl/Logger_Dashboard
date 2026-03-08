/**
 * @file rtc_clock.hpp
 * @brief Helper API for the NXP PCF8563T real-time clock on RP2040 (Raspberry Pi Pico) I2C.
 * @details Provides initialization, calendar/alarm configuration, CLKOUT control,
 *          and conversions to/from Unix epoch. All functions perform I2C transactions
 *          and return false on communication errors where applicable.
 *
 * @note The I2C instance must be initialized and configured (pins, speed, etc.) before use.
 * @note The PCF8563T default 7-bit I2C address is typically 0x51.
 */

/**
 * @brief Initialize and probe the PCF8563T on the given I2C bus.
 *
 * @param i2c Pointer to the I2C instance (e.g., i2c0 or i2c1).
 * @return true if the device was detected and initialized successfully, false on I2C error.
 */
 
/**
 * @brief Set the RTC calendar time.
 *
 * @param i2c Pointer to the I2C instance.
 * @param sec Seconds [0..59].
 * @param min Minutes [0..59].
 * @param hour Hours in 24-hour format [0..23].
 * @param day_of_week Day of week [0..6] as used by PCF8563T (implementation-defined mapping).
 * @param day_of_month Day of month [1..31].
 * @param month Month [1..12].
 * @param year Full year (e.g., 2025). Internally mapped to the device's 2-digit year and century flag.
 * @return true on success, false on I2C error or invalid parameters.
 *
 * @note The device stores year as 00..99 plus a century bit; the implementation derives these from the full year.
 */
 
/**
 * @brief Read the current time from the RTC.
 *
 * @param i2c Pointer to the I2C instance.
 * @param converted_time Output pointer that will receive an implementation-defined 16-bit
 *        packed representation of the current time (e.g., for display/transport). See implementation for encoding.
 * @return true on success, false on I2C error.
 */
 
/**
 * @brief Enable or disable a 1 Hz square wave on the CLKOUT pin.
 *
 * @param i2c Pointer to the I2C instance.
 * @param enable true to enable 1 Hz output, false to disable CLKOUT.
 *
 * @note Configures the CLKOUT control register for 1 Hz when enabled.
 */
 
/**
 * @brief Configure the RTC alarm.
 *
 * @param i2c Pointer to the I2C instance.
 * @param min Minute match [0..59] or 0xFF to "don't care" (disable matching on minutes).
 * @param hour Hour match [0..23] or 0xFF to "don't care".
 * @param day Day-of-month match [1..31] or 0xFF to "don't care". Ignored if use_weekday is true.
 * @param weekday Weekday match [0..6] or 0xFF to "don't care". Used only if use_weekday is true.
 * @param use_weekday When true, the alarm matches on weekday; when false, it matches on day-of-month.
 *
 * @note The PCF8563T supports per-field "don't care" via the AE bits; passing 0xFF indicates "don't care".
 */
 
/**
 * @brief Enable or disable the alarm interrupt.
 *
 * @param i2c Pointer to the I2C instance.
 * @param enable true to enable alarm interrupt generation, false to disable.
 *
 * @note This controls the AIE (Alarm Interrupt Enable) behavior in the control registers.
 */
 
/**
 * @brief Clear the alarm flag (AF) and acknowledge any pending alarm interrupt.
 *
 * @param i2c Pointer to the I2C instance.
 * @return true on success, false on I2C error.
 */
 
/**
 * @brief Set the RTC time from a Unix epoch value.
 *
 * @param i2c Pointer to the I2C instance.
 * @param epoch_utc Seconds since 1970-01-01 00:00:00 UTC.
 * @param as_local When true, interpret epoch_utc as a local-time instant before writing
 *        calendar fields; when false, treat it as UTC. The RTC itself is timezone-agnostic.
 * @return true on success, false on I2C error or conversion failure.
 *
 * @note Timezone handling depends on the platform's time conversion facilities if as_local is true.
 */
 
/**
 * @brief Read the RTC time as a Unix epoch value (UTC).
 *
 * @param i2c Pointer to the I2C instance.
 * @param out_epoch_utc Output pointer to receive seconds since 1970-01-01 00:00:00 UTC.
 * @param fields_are_local When true, interpret the device calendar fields as local time and convert to UTC;
 *        when false, interpret the fields as UTC directly.
 * @return true on success, false on I2C error or conversion failure.
 *
 * @note Timezone conversion (when fields_are_local is true) relies on the system's time configuration.
 */
#ifndef __RTC_CLOCK_HPP__
#define __RTC_CLOCK_HPP__

#include "hardware/i2c.h"
#include <time.h>

bool pcf8563t_init(i2c_inst_t *);
bool pcf8563t_set_time(i2c_inst_t *, uint sec, uint min, uint hour,
                       uint day_of_week, uint day_of_month,
                       uint month, uint year);
bool pcf8563t_read_time(i2c_inst_t *, uint16_t *converted_time);
void pcf8563t_set_clkout_1hz(i2c_inst_t *, bool enable);
void rtc_alarm_set(i2c_inst_t *, uint8_t min, uint8_t hour,
                   uint8_t day, uint8_t weekday, bool use_weekday);
void rtc_alarm_enable(i2c_inst_t *, bool enable);
bool rtc_alarm_flag_clear(i2c_inst_t *);
bool pcf8563t_set_time_epoch(i2c_inst_t*, time_t epoch_utc, bool as_local);
bool pcf8563t_read_time_epoch(i2c_inst_t*, time_t* out_epoch_utc, bool fields_are_local);

#endif /* __RTC_CLOCK_HPP__ */