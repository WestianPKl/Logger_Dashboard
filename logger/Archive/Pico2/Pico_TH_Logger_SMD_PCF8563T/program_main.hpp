
/**
 * @file program_main.hpp
 * @brief Core application coordinator for the Pico-based temperature/humidity logging and Wi-Fi relay system.
 *
 * This header declares the ProgramMain class, responsible for:
 *  - Initializing and orchestrating hardware peripherals (I2C sensors, PWM backlight, RGB output).
 *  - Managing the BME280 environmental sensor lifecycle and measurement display.
 *  - Handling TCP/Wi-Fi connectivity (initialization, reconnection, enable/disable control).
 *  - Time synchronization (RTC to Unix time conversion and system clock alignment).
 *  - Button debouncing, edge detection, press/long-press handling, and related actions.
 *  - Backlight auto-off timing with activity "kick" extension.
 *  - Conditional logging enable/disable.
 *  - Periodic data transmission to a remote endpoint.
 *
 * Macros:
 *  - I2C_PORT, I2C_SDA, I2C_SCL: Pin and bus configuration for the sensor interface.
 *  - WIFI_INIT_FAIL / WIFI_CONN_FAIL / WIFI_OK: Status codes for Wi-Fi initialization and connection attempts.
 *
 * Button Handling:
 *  - Two buttons (on GPIO 20 & 21 implied externally) are tracked with debounced previous states,
 *    press start timestamps, and long-press one-shot flags.
 *  - Short vs long press differentiation is implemented via duration thresholds (logic in poll_buttons()).
 *
 * Backlight Control:
 *  - A deadline timestamp (backlight_deadline_ms) governs automatic shutoff.
 *  - backlight_kick() extends visibility (default 30 seconds) upon user interaction.
 *
 * Time Utilities:
 *  - make_time_utc_from_rtc_fields() converts discrete RTC fields to a UTC time_t.
 *  - synchronize_time() attempts to align system time (e.g., via network or RTC source).
 *
 * PWM Utilities:
 *  - setup_pwm() configures a GPIO for PWM output.
 *  - set_pwm_duty() adjusts duty cycle (e.g., brightness or LED intensity).
 *
 * Networking:
 *  - init_wifi() performs initial Wi-Fi bring-up; reconnect_wifi() attempts recovery after drop.
 *  - Wi-Fi can be toggled at runtime (set_wifi_enabled()) allowing low-power / offline operation.
 *
 * Logging & Display:
 *  - display_measurement() renders current sensor data to the user interface.
 *  - send_data() packages and transmits readings (e.g., to a remote logging service).
 *  - set_logging_enabled() gates data transmission and possibly local record buffering.
 *
 * RGB Control:
 *  - set_rgb_color() allows setting an RGB indicator (e.g., status / alert states).
 *
 * Public Lifecycle:
 *  - init_equipment() performs aggregated hardware init (sensors, display, networking prerequisites).
 *
 * Thread-Safety / Concurrency:
 *  - The class is designed for single-threaded cooperative invocation (e.g., from a main loop).
 *    External synchronization would be required if accessed from ISRs or RTOS tasks.
 *
 * Performance Notes:
 *  - Time comparisons rely on millisecond ticks from to_ms_since_boot(), assumed monotonic.
 *  - Avoid blocking operations inside frequently called methods (e.g., poll_buttons()).
 *
 * Extension Points:
 *  - Add new sensor types by extending init_equipment() and display_measurement().
 *  - Enhance networking by adding retry backoff logic in reconnect_wifi().
 *  - Integrate persistent storage for offline buffering when Wi-Fi is disabled.
 *
 * Error Handling:
 *  - Wi-Fi functions return explicit status codes instead of exceptions.
 *  - Sensor or network object pointers (myBME280, myTCP) should be checked before use.
 *
 * Invariants:
 *  - myBME280 and myTCP are nullptr until initialized.
 *  - Button state flags reflect the most recent poll cycle.
 *
 * @note This class intentionally keeps hardware abstraction minimal; consider refactoring into
 *       specialized managers (SensorManager, NetworkManager, UIManager) if complexity grows.
 *
 * @warning Ensure poll_buttons() is called at a sufficiently high frequency to guarantee
 *          accurate press duration classification (e.g., <= 10–20 ms cadence).
 */
#ifndef __PROGRAM_MAIN_HPP__
#define __PROGRAM_MAIN_HPP__

#include "bme280.hpp"
#include "tcp.hpp"
#include <time.h>

#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

#define WIFI_INIT_FAIL  255
#define WIFI_CONN_FAIL  1
#define WIFI_OK         0

class ProgramMain{
    BME280* myBME280 = nullptr;
    TCP* myTCP = nullptr;
    bool wifi_active = true;

    bool logging_enabled = true;
    bool btn21_prev = true;
    bool btn20_prev = true;
    uint32_t btn21_last_ms = 0;
    uint32_t btn20_last_ms = 0;

    bool     btn20_pressed = false;
    bool     btn21_pressed = false;
    uint32_t btn20_press_start = 0;
    uint32_t btn21_press_start = 0;
    bool     btn20_long_fired = false;
    bool     btn21_long_fired = false;

    uint32_t backlight_deadline_ms = 0;
    inline uint32_t now_ms() { return to_ms_since_boot(get_absolute_time()); }
    void backlight_kick(uint32_t ms = 30000);

private:
    static time_t make_time_utc_from_rtc_fields(uint16_t y, uint16_t m, uint16_t d,
                                                uint16_t hh, uint16_t mm, uint16_t ss);
    void setup_pwm(uint);
    void set_pwm_duty(uint, uint16_t);
    bool synchronize_time();

public:
    void init_equipment();
    uint8_t init_wifi();
    uint8_t reconnect_wifi();
    void set_wifi_enabled(bool enabled) { wifi_active = enabled; }
    bool is_wifi_enabled() const { return wifi_active; }
    void poll_buttons();
    void backlight_autoff_tick();
    void set_logging_enabled(bool en) { logging_enabled = en; }
    bool is_logging_enabled() const { return logging_enabled; }
    void set_rgb_color(uint8_t, uint8_t, uint8_t);
    void display_measurement();
    void send_data();
};

#endif /* __PROGRAM_MAIN_HPP__ */