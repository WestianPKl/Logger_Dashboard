#include "config.hpp"
#include "main.hpp"

#include <cstdint>
#include <cstring>
#include <cstddef>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/regs/addressmap.h"

#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (2u * 1024u * 1024u)
#endif

static_assert(sizeof(Config) <= FLASH_PAGE_SIZE,
              "Config size must fit into a single flash page");

static constexpr uint32_t CONFIG_MAGIC   = 0x434F4E46u;
static constexpr uint16_t CONFIG_VERSION = 4;

static Config       g_config{};
static ConfigSource g_last_source = ConfigSource::Unknown;

struct ConfigV3 {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    uint32_t logger_id;
    uint32_t sensor_id;
    char     server_ip[16];
    uint16_t server_port;
    uint8_t  temperature;
    uint8_t  humidity;
    uint8_t  pressure;
    uint8_t  sht;
    uint8_t  clock_enabled;
    uint8_t  set_time_enabled;
    uint8_t  wifi_enabled;
    uint8_t  logging_enabled;
    char     wifi_ssid[33];
    char     wifi_password[65];
    uint32_t post_time_ms;
    uint32_t crc32;
};

static_assert(sizeof(ConfigV3) <= FLASH_PAGE_SIZE, "v3 config must fit flash page");

/**
 * @brief Computes/updates a CRC-32 (IEEE 802.3) checksum over a byte buffer.
 *
 * Implements the reflected CRC-32 with polynomial 0xEDB88320 (normal form 0x04C11DB7),
 * using an initial value of 0xFFFFFFFF and a final XOR of 0xFFFFFFFF.
 * Supports incremental computation: pass 0 to start a new checksum, or pass the
 * previously returned CRC to continue over subsequent data chunks.
 *
 * @param crc  Current CRC-32 value. Use 0 to start fresh, or the previous return
 *             value to continue incrementally.
 * @param data Pointer to the input bytes to process (must be non-null when len > 0).
 * @param len  Number of bytes to process from data.
 * @return Updated CRC-32 value after processing the provided bytes.
 *
 * @note Bitwise implementation without lookup tables for minimal footprint; slower
 *       than table-driven variants but deterministic and portable.
 */
static uint32_t crc32_update(uint32_t crc, const uint8_t* data, size_t len) {
    crc = crc ^ 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int k = 0; k < 8; ++k) {
            uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

/**
 * Compute the v4 CRC-32 over a Config instance.
 *
 * The checksum covers the raw bytes from the 'version' member (inclusive)
 * up to the 'crc32' member (exclusive). The 'crc32' field itself is not
 * included in the calculation. The CRC is initialized with a seed of 0 and
 * is computed over the in-memory representation of Config.
 *
 * Notes and requirements:
 * - Config must be standard-layout and contain 'version' and 'crc32' in this order.
 * - The result depends on the exact memory layout (including padding), endianness,
 *   and compiler packing rules; compute and verify on the same target/configuration.
 * - Any change to fields between 'version' and 'crc32' invalidates previously stored CRCs.
 * - The function does not modify cfg.
 *
 * @param cfg The configuration instance to checksum.
 * @return The 32-bit CRC of the specified slice of cfg.
 */
static uint32_t calc_crc32_v4(const Config& cfg) {
    const uint8_t* base = reinterpret_cast<const uint8_t*>(&cfg);
    const size_t start = offsetof(Config, version);
    const size_t end   = offsetof(Config, crc32);
    return crc32_update(0, base + start, end - start);
}

/**
 * @brief Calculates a CRC-32 checksum for a ConfigV3 instance.
 *
 * Computes the checksum over the contiguous bytes from ConfigV3::version (inclusive)
 * up to ConfigV3::crc32 (exclusive), thus excluding the checksum field itself. The
 * CRC calculation is seeded with 0 and delegated to crc32_update().
 *
 * @param cfg Reference to the configuration instance to checksum.
 * @return 32-bit CRC-32 value representing the integrity of the selected fields.
 *
 * @pre ConfigV3 is a standard-layout type that defines the fields `version` and
 *      `crc32` in that order.
 * @note The result depends on the exact in-memory layout of ConfigV3. Changes in
 *       field order, packing, alignment, or compiler/architecture ABI can alter
 *       the checksum. Ensure a stable layout across builds if the value is persisted.
 * @see crc32_update, ConfigV3
 */
static uint32_t calc_crc32_v3(const ConfigV3& cfg) {
    const uint8_t* base = reinterpret_cast<const uint8_t*>(&cfg);
    const size_t start = offsetof(ConfigV3, version);
    const size_t end   = offsetof(ConfigV3, crc32);
    return crc32_update(0, base + start, end - start);
}

/**
 * @brief Compute the start offset of the last flash sector.
 *
 * @details Returns the byte offset (relative to the XIP flash base) that marks
 * the beginning of the final erase sector in on-board flash. This is typically
 * used to reserve a dedicated region for persistent data (e.g., configuration
 * or logs) that does not overlap with application code.
 *
 * Assumptions:
 * - PICO_FLASH_SIZE_BYTES and FLASH_SECTOR_SIZE accurately describe the device.
 * - The final sector is reserved and not used by the program image or filesystem.
 *
 * Characteristics:
 * - The returned offset is sector-aligned.
 * - No side effects; thread-safe.
 *
 * @return uint32_t Byte offset from XIP_BASE to the start of the last flash sector.
 */
static uint32_t get_storage_offset() {
    return PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;
}

/**
 * @brief Initialize the global configuration with compile-time default values.
 *
 * This function resets and populates the global configuration structure (g_config)
 * with factory/default settings derived from compile-time constants. It:
 * - Zero-initializes the entire structure to ensure deterministic state.
 * - Sets compatibility fields: CONFIG_MAGIC and CONFIG_VERSION (reserved is cleared).
 * - Assigns device identifiers: LOGGER_ID and SENSOR_ID.
 * - Configures server endpoint: SERVER_IP (null-terminated within buffer) and SERVER_PORT.
 * - Enables/disables sensor features: TEMPERATURE, HUMIDITY, PRESSURE, SHT.
 * - Configures clock behavior: CLOCK and SET_TIME.
 * - Enables Wi‑Fi: WIFI_ENABLE and sets WIFI_SSID/WIFI_PASSWORD (null-terminated within buffers).
 * - Sets posting interval: POST_TIME (milliseconds).
 * - Finalizes the structure integrity by computing and storing CRC32 via calc_crc32_v4.
 *
 * Notes:
 * - String fields are cleared and then copied with truncation-safe semantics; values longer
 *   than their destination buffers will be truncated but always null-terminated.
 * - The CRC covers the fully initialized structure (padding is deterministic due to zeroing).
 *
 * Side effects:
 * - Overwrites any existing contents of g_config.
 *
 * Thread-safety:
 * - Not thread-safe; synchronize external access if g_config may be used concurrently.
 *
 * Preconditions:
 * - Compile-time constants/macros (e.g., CONFIG_MAGIC, SERVER_IP, WIFI_SSID) are defined and valid.
 *
 * Postconditions:
 * - g_config contains a consistent, default configuration with a valid crc32 field.
 *
 * @see calc_crc32_v4
 */
void config_set_defaults() {
    std::memset(&g_config, 0, sizeof(g_config));

    g_config.magic    = CONFIG_MAGIC;
    g_config.version  = CONFIG_VERSION;
    g_config.reserved = 0;

    g_config.logger_id   = LOGGER_ID;
    g_config.sensor_id   = SENSOR_ID;

    std::memset(g_config.server_ip, 0, sizeof(g_config.server_ip));
    std::strncpy(g_config.server_ip, SERVER_IP, sizeof(g_config.server_ip) - 1);
    g_config.server_port = SERVER_PORT;

    g_config.temperature     = TEMPERATURE;
    g_config.humidity        = HUMIDITY;
    g_config.pressure        = PRESSURE;
    g_config.sht             = SHT;
    g_config.clock_enabled   = CLOCK;
    g_config.set_time_enabled= SET_TIME;
    g_config.wifi_enabled    = WIFI_ENABLE;
    g_config.logging_enabled = LOGGING_ENABLE;

    std::memset(g_config.wifi_ssid,     0, sizeof(g_config.wifi_ssid));
    std::memset(g_config.wifi_password, 0, sizeof(g_config.wifi_password));
    std::strncpy(g_config.wifi_ssid,     WIFI_SSID,     sizeof(g_config.wifi_ssid) - 1);
    std::strncpy(g_config.wifi_password, WIFI_PASSWORD, sizeof(g_config.wifi_password) - 1);

    g_config.post_time_ms = POST_TIME;
    g_config.crc32 = calc_crc32_v4(g_config);
}

/**
 * @brief Load configuration from flash into the runtime configuration.
 *
 * @details
 * - Reads a header from the flash region at the offset returned by get_storage_offset().
 * - Validates the magic value (CONFIG_MAGIC). If it does not match, returns false.
 * - If the version matches CONFIG_VERSION:
 *   - Reads the current Config, validates its CRC32 via calc_crc32_v4, and on success
 *     copies it into g_config.
 * - If the version is 3:
 *   - Reads legacy ConfigV3, validates its CRC32 via calc_crc32_v3.
 *   - On success, calls config_set_defaults(), migrates compatible fields to g_config,
 *     updates g_config.version to CONFIG_VERSION, recalculates CRC via calc_crc32_v4,
 *     and stores it in g_config.
 * - Any magic/CRC failure or unsupported version results in no changes and false.
 *
 * @post On success, g_config contains the loaded or migrated configuration and
 *       g_last_source is set to ConfigSource::Loaded.
 * @return true if a configuration was validated and loaded (or migrated) successfully; false otherwise.
 * @note This function accesses global state and is not thread-safe.
 */
bool config_load() {
    const uint32_t offset    = get_storage_offset();
    const uint8_t* flash_ptr = reinterpret_cast<const uint8_t*>(XIP_BASE + offset);

    struct Header {
        uint32_t magic;
        uint16_t version;
    } hdr{};
    static_assert(sizeof(Header) <= sizeof(Config), "Header fits");

    std::memcpy(&hdr, flash_ptr, sizeof(Header));

    if (hdr.magic != CONFIG_MAGIC) {
        return false;
    }

    if (hdr.version == CONFIG_VERSION) {
        Config stored{};
        std::memcpy(&stored, flash_ptr, sizeof(Config));
        const uint32_t crc = calc_crc32_v4(stored);
        if (crc != stored.crc32) {
            return false;
        }
        g_config = stored;
        g_last_source = ConfigSource::Loaded;
        return true;
    } else if (hdr.version == 3) {
        ConfigV3 old{};
        std::memcpy(&old, flash_ptr, sizeof(ConfigV3));
        const uint32_t crc = calc_crc32_v3(old);
        if (crc != old.crc32) {
            return false;
        }

        config_set_defaults();
        g_config.logger_id   = old.logger_id;
        g_config.sensor_id   = old.sensor_id;
        std::memset(g_config.server_ip, 0, sizeof(g_config.server_ip));
        std::strncpy(g_config.server_ip, old.server_ip, sizeof(g_config.server_ip) - 1);
        g_config.server_port = old.server_port;

        g_config.temperature      = old.temperature;
        g_config.humidity         = old.humidity;
        g_config.pressure         = old.pressure;
        g_config.sht              = old.sht;
        g_config.clock_enabled    = old.clock_enabled;
        g_config.set_time_enabled = old.set_time_enabled;
        g_config.wifi_enabled     = old.wifi_enabled;
        g_config.logging_enabled  = old.logging_enabled;

        std::strncpy(g_config.wifi_ssid,     old.wifi_ssid,     sizeof(g_config.wifi_ssid) - 1);
        std::strncpy(g_config.wifi_password, old.wifi_password, sizeof(g_config.wifi_password) - 1);

        g_config.post_time_ms = old.post_time_ms;
        g_config.version = CONFIG_VERSION;
        g_config.crc32   = calc_crc32_v4(g_config);

        g_last_source = ConfigSource::Loaded;
        return true;
    } else {
        return false;
    }
}

/**
 * Saves the current global configuration to on-board flash and verifies the write.
 *
 * Process:
 * - Sets g_config.magic and g_config.version, and computes g_config.crc32.
 * - Resolves the flash address via get_storage_offset().
 * - Disables interrupts, erases the target flash sector, and programs one page
 *   with g_config (all remaining bytes are 0xFF), then restores interrupts.
 * - Reads back from XIP and verifies magic/version/crc32 to confirm success.
 *
 * Returns:
 * - true  if the verification succeeds.
 * - false if the data read back does not match (write/verify failure).
 *
 * Side effects:
 * - Modifies g_config.{magic, version, crc32}.
 * - Erases one flash sector and programs one flash page at the computed offset.
 * - Temporarily disables interrupts during erase/program operations.
 *
 * Preconditions:
 * - get_storage_offset() points to a reserved region aligned for erase/program.
 * - sizeof(Config) <= FLASH_PAGE_SIZE and Config is trivially copyable.
 *
 * Notes:
 * - Not reentrant; do not call concurrently from multiple contexts.
 * - Power loss during erase/program may corrupt the target sector.
 * - Unused bytes in the programmed page are set to 0xFF.
 */
bool config_save() {
    g_config.magic   = CONFIG_MAGIC;
    g_config.version = CONFIG_VERSION;
    g_config.crc32   = calc_crc32_v4(g_config);

    const uint32_t offset = get_storage_offset();

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(offset, FLASH_SECTOR_SIZE);

    alignas(FLASH_PAGE_SIZE) uint8_t page_buffer[FLASH_PAGE_SIZE]{};
    std::memset(page_buffer, 0xFF, sizeof(page_buffer));
    std::memcpy(page_buffer, &g_config, sizeof(Config));
    flash_range_program(offset, page_buffer, FLASH_PAGE_SIZE);
    restore_interrupts(ints);

    Config verify{};
    const uint8_t* flash_ptr = reinterpret_cast<const uint8_t*>(XIP_BASE + offset);
    std::memcpy(&verify, flash_ptr, sizeof(Config));
    return (verify.magic == g_config.magic) &&
           (verify.version == g_config.version) &&
           (verify.crc32 == g_config.crc32);
}

/**
 * @brief Initialize the application configuration.
 *
 * Attempts to load persisted configuration. If loading fails, restores default
 * settings, attempts to persist them, and marks the configuration source as
 * ConfigSource::DefaultsSaved.
 *
 * Behavior:
 * - On successful load: leaves existing configuration as-is.
 * - On load failure: calls config_set_defaults(), then config_save() (its return
 *   value is intentionally ignored), and updates g_last_source accordingly.
 *
 * Intended usage: call once during startup before any configuration-dependent
 * components are used.
 *
 * Thread-safety: should be invoked in a single-threaded initialization phase.
 */
void config_init() {
    if (!config_load()) {
        config_set_defaults();
        (void)config_save();
        g_last_source = ConfigSource::DefaultsSaved;
    }
}

const Config& config_get() { return g_config; }
Config&       config_mut() { return g_config; }
ConfigSource  config_last_source() { return g_last_source; }
