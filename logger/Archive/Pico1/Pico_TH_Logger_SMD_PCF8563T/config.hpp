/**
 * @file config.hpp
 * @brief Configuration data structure and API for the LoggerDashboard Pico-based logger.
 *
 * This module defines a persistent configuration block stored in non-volatile memory
 * (e.g. flash). It provides initialization, load, save, and default population helpers,
 * along with accessors for both const and mutable views of the in-memory configuration.
 *
 * Data Layout (struct Config):
 * - magic: 32-bit sentinel used to validate that stored data is initialized (e.g. a fixed constant).
 * - version: 16-bit configuration format version to support future migrations.
 * - reserved: 16-bit padding / future use (kept for alignment or expansion).
 *
 * Identification:
 * - logger_id: Unique identifier for the logging device.
 * - sensor_id: Identifier for the attached sensor module / board.
 *
 * Network / Server:
 * - server_ip[64]: Null-terminated string containing server hostname or IP (supports FQDNs).
 * - server_port: TCP/UDP destination port for data posting.
 *
 * Sensor / Feature Flags (single-byte enabling / mode indicators):
 * - temperature: Non-zero if temperature measurement is enabled.
 * - humidity: Non-zero if humidity measurement is enabled.
 * - pressure: Non-zero if pressure measurement is enabled.
 * - sht: SHT sensor model selector (0 = none, 30 = SHT30, 40 = SHT40).
 * - clock_enabled: Real-time clock usage flag (1 = enabled).
 * - set_time_enabled: Whether device should attempt to set RTC time (e.g. via network).
 * - wifi_enabled: Wi-Fi subsystem enable flag.
 * - logging_enabled: High-level data logging enable flag.
 *
 * Wi-Fi Credentials:
 * - wifi_ssid[33]: Null-terminated SSID (max 32 chars + terminator).
 * - wifi_password[65]: Null-terminated password (max 64 chars + terminator).
 *
 * Timing:
 * - post_time_ms: Interval (milliseconds) between successive data posts / uploads.
 *
 * Integrity:
 * - crc32: 32-bit CRC over the preceding bytes of the structure for corruption detection.
 *
 * API Functions:
 * - config_init(): Initialize configuration subsystem; typically loads from storage, or sets defaults if invalid.
 * - config_load(): Attempt to load configuration from persistent storage. Returns true on success.
 * - config_save(): Persist the current in-memory configuration to storage. Returns true on success.
 * - config_set_defaults(): Populate the in-memory configuration with safe factory defaults (does not auto-save unless policy dictates).
 * - config_get(): Obtain a const reference to the active configuration (read-only access).
 * - config_mut(): Obtain a mutable reference to the active configuration (callers must invoke config_save() after modifications to persist).
 *
 * ConfigSource Enumeration:
 * - Unknown: The origin of the current configuration cannot be determined (e.g. before init).
 * - Loaded: The configuration was successfully loaded from persistent storage.
 * - DefaultsSaved: Defaults were applied (and likely saved) because no valid prior configuration existed.
 *
 * Usage Pattern:
 * 1. Call config_init() early at startup.
 * 2. Use config_get() for read-only operations (e.g. during normal runtime).
 * 3. When modifying fields, retrieve with config_mut(), edit, then call config_save().
 * 4. Check config_last_source() to determine whether configuration was freshly defaulted or loaded.
 *
 * Thread / Concurrency Notes:
 * - If used in a multithreaded / multi-core context, external synchronization may be required
 *   around config_mut() and config_save() to avoid race conditions.
 *
 * Validation / Integrity:
 * - magic and version are used to detect format compatibility.
 * - crc32 is expected to cover all bytes from start of struct up to (but excluding) crc32 itself.
 *   (Exact implementation details depend on the source; ensure consistency when adding fields.)
 *
 * Extensibility Guidelines:
 * - When adding new fields, increment version and preserve ordering to avoid breaking existing images.
 * - Keep alignment predictable; if changing layout, consider forward/backward migration strategies.
 *
 * Safety:
 * - Ensure strings are always null-terminated when written.
 * - Validate post_time_ms against minimum/maximum operational limits before use.
 */
#pragma once
#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <stdint.h>


struct Config {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;

    uint32_t logger_id;
    uint32_t sensor_id;
    char     server_ip[64];
    uint16_t server_port;

    uint8_t  temperature;
    uint8_t  humidity;
    uint8_t  pressure;
    uint8_t  sht;             // 0/30/40
    uint8_t  clock_enabled;
    uint8_t  set_time_enabled;
    uint8_t  wifi_enabled;
    uint8_t  logging_enabled;

    // Wi-Fi
    char     wifi_ssid[33];
    char     wifi_password[65];
    uint32_t post_time_ms;
    uint32_t crc32;
};

void        config_init();
bool        config_load();
bool        config_save();
void        config_set_defaults();

const Config& config_get();
Config&       config_mut();

enum class ConfigSource : uint8_t {
    Unknown       = 0,
    Loaded        = 1,
    DefaultsSaved = 2,
};
ConfigSource  config_last_source();

#endif /* __CONFIG_HPP__ */