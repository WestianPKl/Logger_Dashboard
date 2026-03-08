#include "com.hpp"

#include <cstring>
#include <cctype>
#include <cstdio>
#include <cstdarg>

#include "pico/stdlib.h"
#include "tusb.h"
extern "C" {
    #include "lwip/timeouts.h"
}

#include "config.hpp"

extern volatile bool wifi_reconnect_flag;
extern volatile bool wifi_apply_flag;
extern volatile bool device_reset_flag;
static bool s_ready_banner_sent = false;
static volatile bool s_pending_show = false;
static volatile bool s_pending_help = false;
static char s_pending_help_args[64] = {0};
static size_t s_help_index = 0;
static int s_help_page = 10;
static const char* s_help_lines[] = {
    "Commands:",
    "  show                               - print current config",
    "  set k=v | set k v                  - update config key",
    "  save | load | defaults             - persist/load/reset config",
    "  reconnect                          - reconnect Wi-Fi (if enabled)",
    "  reset                              - reboot the device",
    "  echo <text>                        - echo back text",
    "  help [next|reset|all|size=N]       - paged help control",
    "",
    "Keys for set:",
    "  logger_id, sensor_id, server_ip, server_port",
    "  temperature, humidity, pressure, sht",
    "  clock, set_time, wifi_enabled, logging_enabled",
    "  wifi_ssid, wifi_password",
    "  post_time_ms (ms)",
    "",
    "Examples:",
    "  show",
    "  set server_ip=192.168.1.10",
    "  set server_port 3000",
    "  set wifi_enabled 1",
    "  set wifi_enabled 0",
    "  set logging_enabled 1",
    "  set logging_enabled 0",
    "  set logger_id 42",
    "  save",
    "  help size=8   (set page size)",
    "  help reset    (go to the beginning)",
    "  help all      (print everything)",
};
static const size_t s_help_count = sizeof(s_help_lines)/sizeof(s_help_lines[0]);

/**
 * Attempts to write the entire buffer to the USB CDC interface within a 1-second timeout.
 *
 * This function performs a best-effort write of 'len' bytes from 'data' to the CDC endpoint.
 * It iteratively writes available chunks, flushing after each successful write. If no bytes
 * can be written immediately, it yields to the TinyUSB task, performs a brief sleep, and
 * retries until either all bytes are written, the device disconnects, or the 1-second
 * deadline elapses. If interrupted, a partial write may occur.
 *
 * Preconditions:
 * - TinyUSB CDC must be initialized and running.
 * - 'data' points to a valid buffer containing at least 'len' bytes.
 *
 * Behavior:
 * - Returns immediately if 'len' <= 0.
 * - Aborts early if the CDC device is not connected.
 * - May block for up to ~1 second while attempting to complete the transfer.
 *
 * Side effects:
 * - Calls tud_task(), tud_cdc_write(), tud_cdc_write_flush(), and sleep_us() to service
 *   USB events and pace retries.
 *
 * @param data Pointer to the buffer to send (not null-terminated unless intended).
 * @param len  Number of bytes to send. If <= 0, the function does nothing.
 *
 * @note The function does not report the number of bytes actually written; use only when
 *       partial writes are acceptable on timeout or disconnect.
 */
static void cdc_write_all(const char* data, int len) {
    if (len <= 0) return;
    int written = 0;
    absolute_time_t deadline = make_timeout_time_ms(1000);
    while (written < len) {
        if (!tud_cdc_connected()) break;
        int n = tud_cdc_write(data + written, (uint32_t)(len - written));
        if (n > 0) {
            written += n;
            tud_cdc_write_flush();
        } else {
            tud_task();
            tight_loop_contents();
            sleep_us(500);
            if (absolute_time_diff_us(get_absolute_time(), deadline) < 0) break;
        }
    }
}

/**
 * @brief Writes a formatted string to the CDC interface without appending a newline.
 *
 * Formats the message using printf-style semantics into an internal 160-byte buffer
 * and transmits the resulting bytes via the CDC transport. If the formatted output
 * exceeds the buffer capacity, it is truncated to fit. If formatting fails, no data
 * is written.
 *
 * - Maximum number of bytes sent per call: 159 (one byte reserved for the NUL terminator).
 * - The transmitted data does not include a trailing NUL terminator or newline.
 *
 * @param fmt printf-style format string.
 * @param ... Arguments corresponding to the format specifiers in fmt.
 *
 * @return void
 */
static void cdc_write_linef(const char* fmt, ...) {
    char line[160];
    va_list ap; va_start(ap, fmt);
    int m = vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    if (m < 0) return;
    if (m >= (int)sizeof(line)) m = (int)sizeof(line) - 1;
    cdc_write_all(line, m);
}

/**
 * @brief Emits the current configuration as key=value lines over the CDC interface.
 *
 * Retrieves the active configuration via config_get() and the last configuration
 * source via config_last_source(), then writes each field using cdc_write_linef().
 *
 * Output format:
 * - logger_id: unsigned
 * - sensor_id: unsigned
 * - server_ip: string
 * - server_port: unsigned
 * - temperature: unsigned
 * - humidity: unsigned
 * - pressure: unsigned
 * - sht: unsigned
 * - clock: unsigned (1 if enabled, 0 otherwise)
 * - set_time: unsigned (1 if enabled, 0 otherwise)
 * - wifi_enabled: unsigned (1 if enabled, 0 otherwise)
 * - wifi_ssid: string
 * - wifi_password: string
 * - post_time_ms: unsigned
 * - config_source: string ("loaded", "defaults", or "unknown")
 *
 * The sequence is terminated with the line "SHOW_END".
 *
 * Side effects:
 * - Sends multiple lines over the CDC channel; no return value.
 *
 * Notes:
 * - post_time_ms is explicitly cast to unsigned for printing.
 * - wifi_password is printed in plaintext; handle logs accordingly.
 */
static void process_show_output() {
    const auto &cfg = config_get();
    cdc_write_linef("logger_id=%u\n", cfg.logger_id);
    cdc_write_linef("sensor_id=%u\n", cfg.sensor_id);
    cdc_write_linef("server_ip=%s\n", cfg.server_ip);
    cdc_write_linef("server_port=%u\n", cfg.server_port);
    cdc_write_linef("temperature=%u\n", cfg.temperature);
    cdc_write_linef("humidity=%u\n", cfg.humidity);
    cdc_write_linef("pressure=%u\n", cfg.pressure);
    cdc_write_linef("sht=%u\n", cfg.sht);
    cdc_write_linef("clock=%u\n", cfg.clock_enabled);
    cdc_write_linef("set_time=%u\n", cfg.set_time_enabled);
    cdc_write_linef("logging_enabled=%u\n", cfg.logging_enabled);
    cdc_write_linef("wifi_enabled=%u\n", cfg.wifi_enabled);
    cdc_write_linef("wifi_ssid=%s\n", cfg.wifi_ssid);
    cdc_write_linef("wifi_password=%s\n", cfg.wifi_password);
    cdc_write_linef("post_time_ms=%u\n", (unsigned)cfg.post_time_ms);
    auto src = config_last_source();
    const char *src_s = (src == ConfigSource::Loaded) ? "loaded" : (src == ConfigSource::DefaultsSaved) ? "defaults" : "unknown";
    cdc_write_linef("config_source=%s\n", src_s);
    cdc_write_linef("SHOW_END\n");
}

/**
 * @brief Handle 'help' command paging and emit formatted help output.
 *
 * Parses the argument to the 'help' command (case-insensitive, trimmed) and
 * performs one of the following actions:
 * - "" or "next": print the next page of help lines starting at the current index.
 * - "reset": reset pager state to the beginning and emit "HELP_RESET".
 * - "all": print all help lines and then "-- end --".
 * - "size=N": set the page size to N (clamped to [1, 50]) and report "HELP_PAGE_SIZE=N".
 * - "<page>": print a 1-based page number (page size is the current pager size).
 *
 * All successful responses are terminated with "HELP_END". On unrecognized input,
 * emits "ERR help args" followed by "HELP_END".
 *
 * Side effects:
 * - Reads and updates global pager state: s_help_index (current offset),
 *   s_help_page (page size), s_help_count (total lines), and s_help_lines (catalog).
 * - Writes formatted output via cdc_write_linef(), including numbered help entries
 *   and continuation hints ("-- more --", "-- end --", or
 *   "-- end (help reset | help size=N) --").
 *
 * Constraints and notes:
 * - The argument is lowercased and trimmed; inputs longer than 63 characters are truncated.
 * - Not thread-safe due to mutation of global state; intended for single-threaded command handling.
 * - Does not return data; all results are emitted through cdc_write_linef().
 *
 * @param rest_arg Non-null, null-terminated C string with the argument following 'help'.
 */
static void process_help_output(const char* rest_arg) {
    auto trim_inplace = [](char *s) {
        size_t start = 0; while (s[start] && isspace((unsigned char)s[start])) ++start;
        if (start) memmove(s, s + start, strlen(s + start) + 1);
        size_t len = strlen(s);
        while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
    };
    auto tolower_copy = [](const char *src, char *dst, size_t dst_size) {
        if (dst_size == 0) return;
        size_t i = 0; for (; src[i] && i + 1 < dst_size; ++i) dst[i] = (char)tolower((unsigned char)src[i]);
        dst[i] = '\0';
    };

    char rest_lc[64]; tolower_copy(rest_arg, rest_lc, sizeof(rest_lc)); trim_inplace(rest_lc);
    auto print_range = [&](size_t start, size_t count){
        size_t end = start + count; if (end > s_help_count) end = s_help_count;
        for (size_t i = start; i < end; ++i) cdc_write_linef("%2u. %s\n", (unsigned)(i + 1), s_help_lines[i]);
        return end;
    };

    bool did_print = false;
    if (rest_lc[0] == '\0' || strcmp(rest_lc, "next") == 0) {
        size_t new_idx = print_range(s_help_index, s_help_page);
        did_print = true; s_help_index = new_idx;
        if (s_help_index >= s_help_count) cdc_write_linef("-- end (help reset | help size=N) --\n");
        else cdc_write_linef("-- more (help | help next | help all) --\n");
        cdc_write_linef("HELP_END\n");
    } else if (strcmp(rest_lc, "reset") == 0) {
        s_help_index = 0; cdc_write_linef("HELP_RESET\n"); cdc_write_linef("HELP_END\n"); did_print = true;
    } else if (strcmp(rest_lc, "all") == 0) {
        print_range(0, s_help_count); cdc_write_linef("-- end --\n"); cdc_write_linef("HELP_END\n"); s_help_index = s_help_count; did_print = true;
    } else if (strncmp(rest_lc, "size=", 5) == 0) {
        int sz = (int)strtoul(rest_lc + 5, nullptr, 10); if (sz < 1) sz = 1; if (sz > 50) sz = 50; s_help_page = sz;
        cdc_write_linef("HELP_PAGE_SIZE=%d\n", s_help_page); cdc_write_linef("HELP_END\n"); did_print = true;
    } else {
        char *endp = nullptr; long pn = strtol(rest_lc, &endp, 10);
        if (endp && *endp == '\0' && pn >= 1) {
            size_t start = (size_t)(pn - 1) * (size_t)s_help_page; if (start > s_help_count) start = s_help_count;
            s_help_index = print_range(start, s_help_page);
            if (s_help_index >= s_help_count) cdc_write_linef("-- end --\n"); else cdc_write_linef("-- more --\n");
            cdc_write_linef("HELP_END\n"); did_print = true;
        }
    }
    if (!did_print) { cdc_write_linef("ERR help args\n"); cdc_write_linef("HELP_END\n"); }
}

/**
 * Returns whether the communication "ready" banner has already been sent.
 *
 * This read-only query can be used to avoid sending the banner multiple times
 * during initialization or after reconnection.
 *
 * @retval true  The ready banner has already been sent.
 * @retval false The ready banner has not yet been sent.
 */
bool com_ready_banner_sent() { return s_ready_banner_sent; }

/**
 * @brief Polls the USB CDC communication service and drains pending console output.
 *
 * Intended to be called frequently from the main loop. Internally services TinyUSB
 * background tasks and handles connection-dependent console I/O.
 *
 * Behavior:
 * - Runs TinyUSB housekeeping (tud_task()) every call.
 * - When a CDC connection is established for the first time, transmits a one-time
 *   "READY v2\n" banner and flushes the TX buffer. The banner will be sent again
 *   after any disconnect/reconnect cycle.
 * - If connected and a "show" response is pending, emits it via the module’s
 *   show-output handler and clears the pending flag.
 * - If connected and a "help" response is pending, emits it via the module’s
 *   help-output handler using the stored arguments, then clears the flag and
 *   resets the argument buffer.
 *
 * Side effects:
 * - Writes to the USB CDC interface and flushes its TX buffer when needed.
 * - Mutates module-level state flags and buffers (e.g., ready-banner latch, pending
 *   show/help flags, and help-argument buffer).
 *
 * Thread-safety:
 * - Not thread-safe. Call from a single context that owns the module state.
 *
 * Requirements:
 * - TinyUSB must be initialized and a CDC interface configured. Safe to call even
 *   when no host is connected.
 *
 * @note Non-blocking aside from TinyUSB service and CDC I/O buffering.
 * @return void
 */
void com_poll() {
    tud_task();

    if (!s_ready_banner_sent && tud_cdc_connected()) {
        tud_cdc_write_str("READY v2\n");
        tud_cdc_write_flush();
        s_ready_banner_sent = true;
    }
    if (!tud_cdc_connected()) {
        s_ready_banner_sent = false;
    }

    if (s_pending_show && tud_cdc_connected()) {
        s_pending_show = false;
        process_show_output();
    }
    if (s_pending_help && tud_cdc_connected()) {
        s_pending_help = false;
        process_help_output(s_pending_help_args);
        s_pending_help_args[0] = '\0';
    }
}

/**
 * TinyUSB CDC receive callback.
 *
 * Accumulates bytes read from the CDC interface into a line buffer, provides basic
 * line-editing (supports Backspace and DEL, ignores CR), and dispatches commands
 * when a newline is received. Lines are trimmed of leading/trailing whitespace
 * before parsing.
 *
 * Parsing and behavior:
 * - Commands are case-insensitive keywords followed by optional arguments.
 * - On buffer overflow (line > 127 chars), input is discarded on newline and
 *   "ERR too long" is sent.
 * - Unknown commands result in "Unknown cmd".
 * - All responses are written to CDC and flushed immediately.
 *
 * Supported commands:
 * - show
 *   - Sets s_pending_show = true (output is deferred to main context).
 *
 * - set <key>=<value>
 *   - Also accepts: set <key> <value>
 *   - Key is lowercased and trimmed; value is trimmed.
 *   - Aliases: "wifi" -> "wifi_enabled", "set" -> "set_time",
 *              "clock_enabled" -> "clock".
 *   - Recognized keys (types) and effects:
 *     - logger_id (uint32)
 *     - sensor_id (uint32)
 *     - server_ip (string; truncated to fit)
 *     - server_port (uint16)
 *     - temperature (uint8)
 *     - humidity (uint8)
 *     - pressure (uint8)
 *     - sht (uint8)
 *     - clock (uint8)
 *     - set_time (uint8)
 *     - logging_enabled (uint8)
 *     - wifi_enabled (uint8) -> also sets wifi_apply_flag = true
 *     - wifi_ssid (string; truncated to fit)
 *     - wifi_password (string; truncated to fit)
 *     - post_time_ms (uint32; clamped to minimum 1000)
 *   - Replies: "OK" on success, "ERR unknown key" otherwise.
 *
 * - save
 *   - Persists configuration. Replies "SAVED wifi_enabled=%u" or "SAVE_ERR".
 *
 * - load
 *   - Loads configuration, sets wifi_apply_flag = true.
 *   - Replies "LOADED wifi_enabled=%u" or "LOAD_ERR".
 *
 * - defaults
 *   - Restores defaults and attempts to save; sets wifi_apply_flag = true.
 *   - Replies "DEFAULTS_SAVED" on successful save, else "DEFAULTS_SET".
 *
 * - reconnect
 *   - Sets wifi_reconnect_flag = true. Replies "RECONNECTING".
 *
 * - help [args]
 *   - Copies args into s_pending_help_args (truncated to its capacity) and sets
 *     s_pending_help = true (deferred processing).
 *
 * - reset
 *   - Sets device_reset_flag = true. Replies "RESETTING".
 *
 * - echo <text>
 *   - Echoes text back followed by newline.
 *
 * Input handling details:
 * - Reads in chunks, accumulates into a 128-byte command buffer.
 * - Backspace (0x08) and DEL (0x7F) delete the last buffered character.
 * - Carriage return (CR) is ignored; newline (LF) terminates the command.
 * - Command keyword is limited to 15 characters; longer keywords are truncated.
 * - Keys/values/args are truncated to their respective buffers.
 *
 * Concurrency and side effects:
 * - Uses static state (buffer, length, overflow flag); not re-entrant.
 * - Intended to run in the TinyUSB callback context.
 * - Mutates configuration via config_mut(), may set:
 *   - wifi_apply_flag, wifi_reconnect_flag, device_reset_flag,
 *     s_pending_show, s_pending_help, s_pending_help_args.
 *
 * Security considerations:
 * - Accepts and stores plaintext Wi-Fi credentials over CDC.
 *
 * @param itf CDC interface index provided by TinyUSB (unused).
 */
extern "C" void tud_cdc_rx_cb(uint8_t) {
    static char   cmd_buf[128];
    static size_t cmd_len = 0;
    static bool   overflow = false;

    char tmp[64];
    uint32_t n = tud_cdc_read(tmp, sizeof(tmp));
    for (uint32_t i = 0; i < n; ++i) {
        char ch = tmp[i];

        if (ch == '\r') {
            continue;
        }

        if (ch == '\b' || ch == 0x7F) { 
            if (cmd_len > 0) cmd_len--;
            continue;
        }

        if (ch != '\n') {
            if (cmd_len + 1 < sizeof(cmd_buf)) {
                cmd_buf[cmd_len++] = ch;
            } else {
                overflow = true;
            }
            continue;
        }

        if (overflow) {
            overflow = false;
            cmd_len = 0;
            tud_cdc_write_str("ERR too long\n");
            tud_cdc_write_flush();
            continue;
        }

        cmd_buf[cmd_len] = '\0';
        cmd_len = 0;

        auto trim_inplace = [](char *s) {
            size_t start = 0; while (s[start] && isspace((unsigned char)s[start])) ++start;
            if (start) memmove(s, s + start, strlen(s + start) + 1);
            size_t len = strlen(s);
            while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
        };
        trim_inplace(cmd_buf);

        char cmd_kw[16];
        size_t k = 0; 
        while (cmd_buf[k] && !isspace((unsigned char)cmd_buf[k]) && k + 1 < sizeof(cmd_kw)) {
            cmd_kw[k] = (char)tolower((unsigned char)cmd_buf[k]);
            ++k;
        }
        cmd_kw[k] = '\0';

        const char *rest = cmd_buf + k;
        while (*rest && isspace((unsigned char)*rest)) ++rest;

        if (strcmp(cmd_kw, "show") == 0 && (*rest == '\0')) {
            s_pending_show = true;
        }
        else if (strcmp(cmd_kw, "set") == 0) {
            char key_raw[48] = {0};
            char val_raw[80] = {0};

            const char *eqp = strchr(rest, '=');
            if (eqp) {
                size_t key_len = (size_t)(eqp - rest);
                if (key_len >= sizeof(key_raw)) key_len = sizeof(key_raw) - 1;
                memcpy(key_raw, rest, key_len); key_raw[key_len] = '\0';
                strncpy(val_raw, eqp + 1, sizeof(val_raw) - 1);
            } else {
                const char *sp = rest;
                while (*sp && !isspace((unsigned char)*sp)) ++sp;
                size_t key_len = (size_t)(sp - rest);
                if (key_len >= sizeof(key_raw)) key_len = sizeof(key_raw) - 1;
                memcpy(key_raw, rest, key_len); key_raw[key_len] = '\0';
                while (*sp && isspace((unsigned char)*sp)) ++sp;
                strncpy(val_raw, sp, sizeof(val_raw) - 1);
            }

            auto trim2 = [](char *s) {
                size_t start = 0; while (s[start] && isspace((unsigned char)s[start])) ++start;
                if (start) memmove(s, s + start, strlen(s + start) + 1);
                size_t len = strlen(s);
                while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
            };
            trim2(key_raw);
            trim2(val_raw);

            char key_lc[48];
            size_t i2 = 0; for (; key_raw[i2] && i2 + 1 < sizeof(key_lc); ++i2) key_lc[i2] = (char)tolower((unsigned char)key_raw[i2]);
            key_lc[i2] = '\0';

            if (strcmp(key_lc, "wifi") == 0) strcpy(key_lc, "wifi_enabled");
            else if (strcmp(key_lc, "set") == 0) strcpy(key_lc, "set_time");
            else if (strcmp(key_lc, "clock_enabled") == 0) strcpy(key_lc, "clock");

            if (key_lc[0] == '\0' || val_raw[0] == '\0') {
                tud_cdc_write_str("ERR format\n");
                tud_cdc_write_flush();
                continue;
            }

            auto &cfg = config_mut();
            bool ok = true;

            if      (strcmp(key_lc, "logger_id")     == 0) cfg.logger_id      = (uint32_t)strtoul(val_raw, nullptr, 10);
            else if (strcmp(key_lc, "sensor_id")     == 0) cfg.sensor_id      = (uint32_t)strtoul(val_raw, nullptr, 10);
            else if (strcmp(key_lc, "server_ip")     == 0) snprintf(cfg.server_ip, sizeof(cfg.server_ip), "%s", val_raw);
            else if (strcmp(key_lc, "server_port")   == 0) cfg.server_port    = (uint16_t)strtoul(val_raw, nullptr, 10);
            else if (strcmp(key_lc, "temperature")   == 0) cfg.temperature    = (uint8_t)strtoul(val_raw, nullptr, 10);
            else if (strcmp(key_lc, "humidity")      == 0) cfg.humidity       = (uint8_t)strtoul(val_raw, nullptr, 10);
            else if (strcmp(key_lc, "pressure")      == 0) cfg.pressure       = (uint8_t)strtoul(val_raw, nullptr, 10);
            else if (strcmp(key_lc, "sht")           == 0) cfg.sht            = (uint8_t)strtoul(val_raw, nullptr, 10);
            else if (strcmp(key_lc, "clock")         == 0) cfg.clock_enabled  = (uint8_t)strtoul(val_raw, nullptr, 10);
            else if (strcmp(key_lc, "set_time")      == 0) cfg.set_time_enabled = (uint8_t)strtoul(val_raw, nullptr, 10);
            else if (strcmp(key_lc, "logging_enabled") == 0) cfg.logging_enabled = (uint8_t)strtoul(val_raw, nullptr, 10);
            else if (strcmp(key_lc, "wifi_enabled")  == 0) { cfg.wifi_enabled = (uint8_t)strtoul(val_raw, nullptr, 10); wifi_apply_flag = true; }
            else if (strcmp(key_lc, "wifi_ssid")     == 0) snprintf(cfg.wifi_ssid, sizeof(cfg.wifi_ssid), "%s", val_raw);
            else if (strcmp(key_lc, "wifi_password") == 0) snprintf(cfg.wifi_password, sizeof(cfg.wifi_password), "%s", val_raw);
            else if (strcmp(key_lc, "post_time_ms")  == 0) {
                uint32_t v = (uint32_t)strtoul(val_raw, nullptr, 10);
                if (v < 1000) v = 1000;
                cfg.post_time_ms = v;
            }
            else ok = false;

            tud_cdc_write_str(ok ? "OK\n" : "ERR unknown key\n");
            tud_cdc_write_flush();
        }
        else if (strcmp(cmd_kw, "save") == 0 && (*rest == '\0')) {
            bool ok = config_save();
            if (ok) {
                const auto &c = config_get();
                cdc_write_linef("SAVED wifi_enabled=%u\n", c.wifi_enabled);
            } else {
                tud_cdc_write_str("SAVE_ERR\n");
            }
            tud_cdc_write_flush();
        }
        else if (strcmp(cmd_kw, "load") == 0 && (*rest == '\0')) {
            bool ok = config_load();
            if (ok) {
                wifi_apply_flag = true;
                const auto &c = config_get();
                cdc_write_linef("LOADED wifi_enabled=%u\n", c.wifi_enabled);
            } else {
                tud_cdc_write_str("LOAD_ERR\n");
            }
            tud_cdc_write_flush();
        }
        else if (strcmp(cmd_kw, "defaults") == 0 && (*rest == '\0')) {
            config_set_defaults();
            bool ok = config_save();
            wifi_apply_flag = true;
            tud_cdc_write_str(ok ? "DEFAULTS_SAVED\n" : "DEFAULTS_SET\n");
            tud_cdc_write_flush();
        }
        else if (strcmp(cmd_kw, "reconnect") == 0 && (*rest == '\0')) {
            wifi_reconnect_flag = true;
            tud_cdc_write_str("RECONNECTING\n");
            tud_cdc_write_flush();
        }
        else if (strcmp(cmd_kw, "help") == 0) {
            size_t arg_len = strlen(rest);
            if (arg_len >= sizeof(s_pending_help_args)) arg_len = sizeof(s_pending_help_args) - 1;
            memcpy(s_pending_help_args, rest, arg_len);
            s_pending_help_args[arg_len] = '\0';
            s_pending_help = true;
        }
        else if (strcmp(cmd_kw, "reset") == 0 && (*rest == '\0')) {
            device_reset_flag = true;
            tud_cdc_write_str("RESETTING\n");
            tud_cdc_write_flush();
        }
        else if (strcmp(cmd_kw, "echo") == 0) {
            tud_cdc_write(rest, strlen(rest));
            tud_cdc_write_str("\n");
            tud_cdc_write_flush();
        }
        else {
            tud_cdc_write_str("Unknown cmd\n");
            tud_cdc_write_flush();
        }
    }
}
