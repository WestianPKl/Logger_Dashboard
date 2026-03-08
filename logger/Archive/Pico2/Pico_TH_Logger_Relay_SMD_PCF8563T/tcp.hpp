/**
 * @file tcp.hpp
 * @brief TCP/HTTP utility built on lwIP for embedded targets.
 *
 * Provides minimal helpers to resolve hostnames, establish TCP connections,
 * and perform HTTP requests to obtain an authorization token, submit sensor
 * data, and report errors. Designed for constrained systems using lwIP.
 *
 * All network operations are performed synchronously/blocking relative to
 * the caller and rely on lwIP. This utility is not thread-safe; call from
 * the appropriate networking/task context for your platform.
 */

/**
 * @class TCP
 * @brief Thin wrapper around lwIP to perform token-based HTTP exchanges.
 *
 * Responsibilities:
 * - Resolve hostnames to IP addresses using lwIP DNS.
 * - Establish TCP connections (via lwIP) and exchange HTTP messages.
 * - Acquire and cache an authorization token with an expiration time.
 * - Submit application data and error logs using HTTP requests.
 */

/**
 * @brief Scratch buffer for accumulating incoming TCP payloads.
 * Size is 1024 bytes.
 */
 
/**
 * @brief Current length of valid data in the receive buffer.
 */

/**
 * @brief Cached authorization token obtained from the server.
 * Size is 256 bytes including the terminating null.
 */
 
/**
 * @brief UNIX epoch time (UTC, seconds) when the cached token expires.
 * A value of 0 indicates no valid token is cached.
 */

/**
 * @brief lwIP connection callback invoked when a TCP connect attempt completes.
 *
 * @param arg User-supplied argument passed to lwIP (unused by this class).
 * @param pcb Pointer to the lwIP TCP protocol control block.
 * @param err lwIP status code for the connection attempt.
 * @return lwIP error code; ERR_OK on success or an lwIP error otherwise.
 *
 * @note This is a static callback conforming to the lwIP API.
 */

/**
 * @brief Resolve a hostname or dotted-quad IP string to an ip_addr_t (blocking).
 *
 * Attempts DNS resolution for hostnames; if the input is already an IP literal,
 * it is parsed directly. The call blocks until resolution completes or the
 * timeout elapses.
 *
 * @param host_or_ip Hostname (e.g., "example.com") or IP literal (e.g., "192.0.2.1").
 * @param out Output pointer to receive the resolved IP address.
 * @param timeout_ms Maximum time to wait for resolution, in milliseconds.
 * @return true if resolution succeeded and 'out' was filled; false on failure or timeout.
 */
 
/**
 * @brief Check if an HTTP response indicates success.
 *
 * Parses the beginning of the HTTP response to determine if the status code
 * is in the success range (e.g., 200 OK).
 *
 * @param response Null-terminated HTTP response buffer.
 * @return true if the response is considered OK; false otherwise.
 */
 
/**
 * @brief Request an authorization token from the server via HTTP GET and cache it.
 *
 * On success, updates the internal token buffer and expiration time.
 *
 * @return true if a token was successfully obtained and cached; false otherwise.
 */
 
/**
 * @brief Submit application data to the server via HTTP POST.
 *
 * Typically used to report time-stamped measurements.
 *
 * @param iso8601_utc Timestamp in ISO 8601 UTC format (e.g., "2024-01-02T03:04:05Z").
 * @param temp Temperature value to send.
 * @param hum Humidity value to send.
 * @param pressure Pressure value to send.
 * @return true if the POST request succeeded; false otherwise.
 *
 * @note May require a valid cached token; see ensure_token().
 */
 
/**
 * @brief Send an error log entry to the server.
 *
 * @param message Short human-readable error message.
 * @param details Optional extended diagnostic information; may be null.
 * @return true if the log was successfully delivered; false otherwise.
 */
 
/**
 * @brief Get a pointer to the currently cached authorization token.
 *
 * @return Pointer to a null-terminated token string if present; otherwise an empty string.
 *
 * @warning The returned pointer is owned by this class and becomes invalid when
 *          a new token is fetched or invalidate_token() is called.
 */
 
/**
 * @brief Ensure a valid token is available for at least the given TTL.
 *
 * If no token is cached or the cached token expires sooner than ttl_sec,
 * attempts to fetch a new token. On success, guarantees that the cached
 * token is valid for at least ttl_sec seconds from now.
 *
 * @param ttl_sec Minimum required time-to-live in seconds (default: 50).
 * @return true if a valid token is available after the call; false otherwise.
 */
 
/**
 * @brief Invalidate and clear the cached token and its expiration time.
 *
 * Safe to call at any time; subsequent operations that require a token must
 * reacquire one (e.g., via ensure_token()).
 */
#ifndef __TCP_HPP__
#define __TCP_HPP__

extern "C" {
    #include "lwip/err.h"
    #include "lwip/tcp.h"
    #include "lwip/dns.h"
    #include "lwip/ip_addr.h"
}

class TCP {
private:
    char   recv_buffer[1024] = {0};
    size_t recv_len = 0;
    char   received_token[256] = {0};
    uint32_t token_expire_epoch = 0;

    static err_t tcp_connected_callback(void *, struct tcp_pcb *, err_t);
    bool resolve_host_blocking(const char* host_or_ip, ip_addr_t* out, uint32_t timeout_ms);
    static bool http_ok(const char* response);

public:
    bool send_token_get_request();
    bool send_data_post_request(const char* iso8601_utc, float temp, float hum, float pressure);
    bool send_error_log(const char* message, const char* details = nullptr);
    const char* get_token();
    bool ensure_token(uint32_t ttl_sec = 50);
    void invalidate_token() { received_token[0] = '\0'; token_expire_epoch = 0; }
};

#endif /* __TCP__ */