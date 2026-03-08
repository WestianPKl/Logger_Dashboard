
/**
 * @file main.hpp
 * @brief Central configuration header defining compile‑time constants for networking, logging, sensors, and runtime features
 *        of the Pico_TH_Logger_Relay_SMD_PCF8563T project.
 *
 * SECTION: Backend / API
 * - TOKEN_PATH  : (const char*) Relative REST endpoint used to obtain or refresh an authorization / data token.
 * - DATA_PATH   : (const char*) Relative REST endpoint for regular telemetry (environmental data) submissions.
 * - ERROR_PATH  : (const char*) Relative REST endpoint dedicated to reporting errors or fault conditions.
 *
 * SECTION: TCP / Server
 * - SERVER_IP   : (const char*) IPv4 address (string) of the backend server the device will connect to.
 * - SERVER_PORT : (int)        TCP port corresponding to the backend service handling token/data/error requests.
 *
 * SECTION: Logger and Sensor Identity
 * - LOGGER_ID   : (int) Unique numeric identifier for the physical logger unit (used for backend association).
 * - SENSOR_ID   : (int) Unique numeric identifier for the attached sensor module or channel grouping.
 *
 * SECTION: Equipment Feature Flags
 * - TEMPERATURE    : (0|1) Enable (1) temperature measurement acquisition pipeline.
 * - HUMIDITY       : (0|1) Enable (1) humidity measurement acquisition pipeline.
 * - PRESSURE       : (0|1) Enable (1) barometric pressure measurement acquisition pipeline.
 * - SHT            : (int) Sensor type selector:
 *                      0  = BME280 (default path when SHT series not used)
 *                      30 = SHT30
 *                      40 = SHT40
 *                    Code should branch on this to instantiate the proper driver.
 * - CLOCK          : (0|1) Enable (1) use of external RTC (PCF8563T) for timekeeping.
 * - SET_TIME       : (0|1) If 1, perform an RTC time set/synchronization routine at startup (e.g., via NTP or server).
 * - LOGGING_ENABLE : (0|1) Master switch to enable periodic local logging / buffering of sensor data.
 *
 * SECTION: Wi‑Fi Configuration
 * - WIFI_ENABLE  : (0|1) Enable (1) Wi‑Fi subsystem initialization and network operations.
 * - WIFI_SSID    : (const char*) SSID of the target Wi‑Fi network (plaintext).
 * - WIFI_PASSWORD: (const char*) WPA/WPA2 passphrase for the specified SSID (plaintext).
 *                  NOTE: Consider refactoring for secure storage (e.g., flash partition, secure element).
 *
 * SECTION: Telemetry Timing
 * - POST_TIME : (unsigned long, ms) Minimum interval between successive telemetry POST operations.
 *               Constraint: Must be >= 1000 ms at runtime (code should validate and clamp if necessary).
 *
 * USAGE GUIDELINES:
 * 1. Modify these macros prior to compilation to adapt device behavior (no runtime reconfiguration implied).
 * 2. Keep identifiers (LOGGER_ID, SENSOR_ID) synchronized with backend registry to avoid data collisions.
 * 3. For production, do not hard‑code credentials; integrate a secure provisioning flow.
 * 4. When adding new endpoints or feature flags, document them here to maintain a single source of truth.
 *
 * EXTENSIBILITY NOTES:
 * - Consider migrating to a structured configuration object (constexpr struct) for stronger type safety.
 * - Introduce environment‑specific overrides (e.g., via CMake or build system -D flags) to avoid editing source.
 * - Add validation layer in initialization code to assert compatibility (e.g., SHT value vs enabled sensors).
 *
 * SECURITY CONSIDERATIONS:
 * - Plaintext Wi‑Fi credentials in firmware can be extracted; employ encryption or dynamic provisioning for deployment.
 * - Token endpoint path suggests authentication flow; ensure tokens are not logged inadvertently in error logs.
 *
 * THREADING / TIMING:
 * - POST_TIME influences network duty cycle and power consumption; adjust based on battery / throughput needs.
 * - If using an RTC (CLOCK=1) and SET_TIME=1, ensure network is available early or queue time sync attempts.
 *
 * MAINTENANCE:
 * - Incrementally refactor magic numbers (e.g., sensor selector values) into scoped enum classes for clarity.
 * - Ensure any changes to SERVER_IP / SERVER_PORT propagate to DNS or load balancer strategy if introduced later.
 */
#ifndef __MAIN_HPP__
#define __MAIN_HPP__

// === Backend/API ===
#define TOKEN_PATH      "/api/data/data-token"
#define DATA_PATH       "/api/data/data-log"
#define ERROR_PATH      "/api/common/error-log"

// === TCP ===
#define SERVER_IP       ""
#define SERVER_PORT     3000

// === Logger and Sensor IDs ===
#define LOGGER_ID       3
#define SENSOR_ID       5

// === Equipment ===
#define TEMPERATURE     1
#define HUMIDITY        1
#define PRESSURE        1
#define SHT             0     // 0 - BME280 / 30 - SHT30 / 40 - SHT40
#define CLOCK           1
#define SET_TIME        1
#define LOGGING_ENABLE  1

// === Wi-Fi ===
#define WIFI_ENABLE     1
#define WIFI_SSID       ""
#define WIFI_PASSWORD   ""

// === Telemetry ===
#define POST_TIME       600000  // ms (min. 1000 in runtime)

#endif /* __MAIN_HPP__ */