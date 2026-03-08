#include "pico/cyw43_arch.h"
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tcp.hpp"
#include "main.hpp"
#include "config.hpp"

extern "C" {
    #include "lwip/timeouts.h"
    #include "lwip/tcp.h"
    #include "lwip/dns.h"
    #include "lwip/ip_addr.h"
}

struct tcp_context_base_t {
    volatile bool done   = false;
    volatile bool failed = false;
    uint8_t poll_ticks   = 0;
};

struct token_ctx_t : public tcp_context_base_t {
    TCP* self = nullptr;
};

struct post_ctx_t : public tcp_context_base_t {
    char request[768];
    char response[1024];
    size_t response_len = 0;
};

struct dns_wait_ctx {
    volatile bool done = false;
    ip_addr_t addr{};
};


/**
 * @brief Determine if an HTTP status line indicates a successful (2xx) response.
 *
 * Parses the start of an HTTP status line of the form "HTTP/<version> <code> ...".
 * Returns true only when a numeric status code can be parsed and it falls within
 * the 2xx success range.
 *
 * @param resp Null-terminated C string beginning with an HTTP status line
 *             (e.g., "HTTP/1.1 200 OK"). May be nullptr.
 * @return true if a status code is parsed and is between 200 and 299 (inclusive);
 *         false if resp is null, malformed, or the code is outside the 2xx range.
 *
 * @note Only the version token and numeric status code are inspected; the reason
 *       phrase and any trailing content are ignored.
 */
static bool http_status_ok_impl(const char* resp) {
    if (!resp) return false;
    int st = 0;
    if (sscanf(resp, "HTTP/%*s %d", &st) != 1) return false;
    return (st >= 200 && st <= 299);
}

/**
 * Locate the start of the HTTP message body in a raw HTTP response buffer.
 *
 * Scans the null-terminated response for the header/body separator sequence
 * "\r\n\r\n" and returns a pointer to the first byte of the body. If the input
 * is null or the separator is not found, returns nullptr.
 *
 * Notes:
 * - The returned pointer aliases the input buffer; no copying is performed.
 * - The input must be a valid, null-terminated C string containing a complete HTTP response.
 * - Only the first empty line is used to determine the boundary; no further HTTP validation is performed.
 *
 * Complexity: O(n) in the length of the input.
 *
 * @param resp Pointer to a null-terminated C string with a full HTTP response.
 * @return Pointer to the first character of the message body, or nullptr if not found or input is null.
 */
static const char* http_body_start(const char* resp) {
    if (!resp) return nullptr;
    const char* b = strstr(resp, "\r\n\r\n");
    return b ? (b + 4) : nullptr;
}

/**
 * Determines whether an HTTP response declares "Transfer-Encoding: chunked".
 *
 * Scans header lines from the beginning of the response buffer up to the first
 * empty line (CRLF). If a line that starts with "Transfer-Encoding:" is found,
 * it checks (case-insensitively) whether its value contains the token "chunked".
 *
 * @param resp Pointer to a NUL-terminated buffer containing the HTTP response
 *             starting at the status line or header section. May be nullptr.
 * @return true if a "Transfer-Encoding:" header exists whose value includes
 *         "chunked" (case-insensitive); false otherwise or if resp is nullptr.
 *
 * Limitations:
 * - Expects CRLF line endings and stops at the first empty line (end of headers).
 * - Does not trim whitespace or parse comma-separated tokens; any occurrence of
 *   "chunked" in the header value matches.
 * - Does not handle folded/continued headers or leading whitespace before the
 *   header name.
 * - Ignores content beyond the header section and does not validate HTTP syntax.
 */
static bool http_has_chunked(const char* resp) {
    if (!resp) return false;
    const char* cur = resp;
    while (true) {
        const char* line_end = strstr(cur, "\r\n");
        if (!line_end) break;
        size_t len = (size_t)(line_end - cur);
        if (len == 0) break;
        const char* p = cur;
        const char* hdr = "Transfer-Encoding:";
        size_t i = 0;
        while (i < len && hdr[i] && (tolower((unsigned char)p[i]) == tolower((unsigned char)hdr[i]))) i++;
        if (!hdr[i]) {
            const char* val = p + i;
            for (size_t j = 0; j < len - i; ++j) {
                const char* k = "chunked";
                size_t m = 0;
                while (j + m < len - i && k[m] &&
                       tolower((unsigned char)val[j + m]) == tolower((unsigned char)k[m])) m++;
                if (!k[m]) return true;
            }
        }
        cur = line_end + 2;
    }
    return false;
}

/**
 * @brief Closes a TCP connection gracefully when possible, otherwise aborts it.
 *
 * Attempts to terminate the given lwIP TCP protocol control block (PCB) using
 * tcp_close() if a graceful shutdown is permitted. If a graceful close is not
 * allowed, or if tcp_close() fails, the connection is forcefully terminated
 * with tcp_abort().
 *
 * @param pcb Pointer to the lwIP TCP PCB to terminate. If null, the function returns immediately.
 * @param ok  When true, attempt a graceful close (FIN). When false, or if the
 *            graceful close fails, the connection is aborted (RST).
 *
 * @note After this call, the PCB is no longer valid and must not be used.
 * @note Aborting sends an RST to the peer and immediately frees PCB resources.
 * @note Must be called from the appropriate lwIP TCP context (e.g., within TCP
 *       callbacks or with the core lock held, depending on your lwIP threading model).
 */
static void tcp_close_or_abort(struct tcp_pcb* pcb, bool ok) {
    if (!pcb) return;
    if (ok) {
        if (tcp_close(pcb) != ERR_OK) tcp_abort(pcb);
    } else {
        tcp_abort(pcb);
    }
}


/**
 * @brief DNS lookup completion callback (lwIP-style).
 *
 * Interprets the user-supplied argument as a dns_wait_ctx, sets its completion
 * flag, and, if a resolved address is provided, copies it into the context.
 *
 * @param name   The queried hostname (unused).
 * @param ipaddr Pointer to the resolved IP address; nullptr on failure.
 * @param arg    Opaque user data expected to be a dns_wait_ctx*; if nullptr, the callback does nothing.
 *
 * @note When the context is valid, the completion flag is set regardless of success.
 *       If ipaddr is nullptr, the stored address is not modified, allowing the caller
 *       to infer failure by checking whether the address was updated.
 * @pre  arg must point to a valid dns_wait_ctx for the results to be captured.
 */
static void dns_cb(const char*, const ip_addr_t* ipaddr, void* arg) {
    auto* c = static_cast<dns_wait_ctx*>(arg);
    if (!c) return;
    if (ipaddr) c->addr = *ipaddr;
    c->done = true;
}

/**
 * @brief Resolve a hostname or numeric IP string to an ip_addr_t, blocking until completion or timeout.
 *
 * First attempts to parse host_or_ip as a literal IP address; if successful, returns immediately.
 * Otherwise, starts an lwIP DNS query and blocks, periodically polling the CYW43 driver and
 * processing lwIP timers, until the query completes or the timeout elapses.
 *
 * While waiting, the function calls cyw43_arch_poll() and sys_check_timeouts(), and sleeps
 * in ~10 ms intervals to yield time.
 *
 * @param host_or_ip  Null-terminated hostname or numeric IPv4/IPv6 string to resolve. Must not be null.
 * @param out         Non-null pointer that receives the resolved address on success.
 * @param timeout_ms  Maximum time to wait, in milliseconds. If 0, a default of 5000 ms is used.
 *
 * @return true on success (and *out is set); false if arguments are invalid, DNS fails immediately,
 *         or the operation times out (in which case *out is not modified).
 *
 * @pre The network stack (lwIP/CYW43) must be initialized and, if applicable, connected.
 * @note This is a blocking call; use only where blocking is acceptable. Not suitable for ISR context.
 * @warning Follow lwIP threading rules; typically call from the same context that drives lwIP or
 *          ensure proper locking if used from other threads.
 */
bool TCP::resolve_host_blocking(const char* host_or_ip, ip_addr_t* out, uint32_t timeout_ms) {
    if (!host_or_ip || !out) return false;

    ip_addr_t ip;
    if (ipaddr_aton(host_or_ip, &ip)) {
        *out = ip;
        return true;
    }

    dns_wait_ctx ctx{};
    err_t e = dns_gethostbyname(host_or_ip, &ctx.addr, dns_cb, &ctx);
    if (e == ERR_OK) { *out = ctx.addr; return true; }
    if (e != ERR_INPROGRESS) return false;

    absolute_time_t dl = make_timeout_time_ms(timeout_ms ? timeout_ms : 5000);
    while (!time_reached(dl) && !ctx.done) {
        cyw43_arch_poll();
        sys_check_timeouts();
        sleep_ms(10);
    }
    if (!ctx.done) return false;
    *out = ctx.addr;
    return true;
}

/**
 * Determine whether the given HTTP response indicates a successful “OK” status.
 *
 * Parses the start line of the provided HTTP response and returns true if it
 * represents a successful status (typically 200 OK). The input must be a
 * non-null, null-terminated C string containing at least the HTTP status line.
 *
 * @param response Null-terminated C-string containing the HTTP response data.
 * @return true if the HTTP status is considered OK; false otherwise.
 *
 * @note The function does not modify the input buffer and does not take ownership of it.
 */
bool TCP::http_ok(const char* response) {
    return http_status_ok_impl(response);
}

/**
 * Retrieves a pointer to the internally stored, null-terminated token string.
 *
 * The returned pointer refers to memory owned by the TCP instance and must not
 * be modified or freed by the caller.
 *
 * Lifetime: The pointer remains valid until the token is updated or the TCP
 * object is destroyed.
 * Thread-safety: Access is not synchronized; coordinate externally if accessed
 * from multiple threads.
 *
 * @return const char* Pointer to the current token string.
 */
const char* TCP::get_token() {
    return received_token;
}

/**
 * Ensures a valid authentication token is available, refreshing it if expired or missing.
 * If a non-expired token is already present, the function returns immediately.
 * Otherwise, it attempts to obtain a new token and, when the current time is known,
 * sets the internal expiration to now + (ttl_sec != 0 ? ttl_sec : 1200).
 *
 * @param ttl_sec Optional token time-to-live in seconds; if 0, a default of 1200 seconds is used.
 * @return true if a valid token is present or successfully obtained; false if acquiring a token fails.
 * @note If the system time cannot be determined, expiration tracking is skipped.
 */
bool TCP::ensure_token(uint32_t ttl_sec) {
    time_t now = time(NULL);
    if (received_token[0] && token_expire_epoch && now != (time_t)-1) {
        if (now < (time_t)token_expire_epoch) return true;
    }
    if (!send_token_get_request()) return false;
    if (!received_token[0]) return false;
    if (now != (time_t)-1) {
        token_expire_epoch = (uint32_t)(now + (ttl_sec ? ttl_sec : 1200));
    }
    return true;
}

/**
 * @brief Synchronously retrieves an authentication token via HTTP GET over TCP.
 *
 * Resolves the configured server host, opens a lwIP TCP connection to the configured port,
 * sends a minimal HTTP/1.1 GET request to TOKEN_PATH with "Connection: close", and accumulates
 * the response until the connection closes. The response is validated (HTTP OK, not chunked),
 * and the JSON field "token" is extracted from the body into received_token.
 *
 * Operation and timing:
 * - Drives lwIP and the CYW43 driver in a polling loop for up to 8 seconds or until completion.
 * - Uses tcp_poll and an absolute deadline to detect timeouts; on timeout/error the PCB is aborted.
 *
 * Side effects:
 * - Clears recv_buffer, received_token, and recv_len before starting.
 * - Performs DNS resolution and network I/O.
 * - Closes or aborts the TCP PCB on exit depending on success/failure.
 *
 * Error conditions resulting in false:
 * - DNS resolution, allocation, or connection failure.
 * - Timeout or TCP error callbacks.
 * - Non-OK HTTP status or chunked transfer encoding.
 * - Failure to locate a "token" field in the response body.
 *
 * Dependencies:
 * - config_get(), resolve_host_blocking(), http_ok(), http_has_chunked(), http_body_start(),
 *   tcp_close_or_abort(); lwIP TCP APIs and CYW43 polling.
 *
 * Preconditions:
 * - Wi-Fi must be connected; caller must be able to regularly service lwIP (cyw43_arch_poll()
 *   and sys_check_timeouts()).
 * - config_get() provides a valid server_ip (hostname or dotted-quad) and server_port.
 * - TOKEN_PATH is defined.
 *
 * Postconditions:
 * - On success, received_token contains a NUL-terminated token string and the function returns true.
 * - On failure, received_token is empty and the function returns false.
 *
 * Thread-safety: Not reentrant; mutates instance buffers. Call from a context that can safely
 * drive the networking stack. No TLS is used; avoid sending sensitive data over untrusted networks.
 *
 * @return true if a token was successfully parsed from a valid HTTP response; false otherwise.
 */
bool TCP::send_token_get_request() {
    const auto &cfg = config_get();

    ip_addr_t server_ip;
    if (!resolve_host_blocking(cfg.server_ip, &server_ip, 5000)) {
        return false;
    }

    recv_len = 0;
    memset(recv_buffer, 0, sizeof(recv_buffer));
    memset(received_token, 0, sizeof(received_token));

    token_ctx_t *ctx = (token_ctx_t*)calloc(1, sizeof(token_ctx_t));
    if (!ctx) return false;
    ctx->self = this;

    struct tcp_pcb* pcb = tcp_new();
    if (!pcb) {
        free(ctx);
        return false;
    }

    tcp_arg(pcb, ctx);

    tcp_err(pcb, [](void *arg, err_t) {
        auto *c = static_cast<token_ctx_t*>(arg);
        if (c) { c->failed = true; c->done = true; }
    });

    tcp_recv(pcb, [](void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) -> err_t {
        auto *c = static_cast<token_ctx_t*>(arg);
        if (!c || !c->self) {
            if (p) pbuf_free(p);
            tcp_abort(pcb);
            return ERR_ABRT;
        }

        if (err != ERR_OK) {
            if (p) pbuf_free(p);
            c->failed = true; c->done = true;
            tcp_abort(pcb);
            return ERR_ABRT;
        }

        if (!p) {
            c->self->recv_buffer[c->self->recv_len] = '\0';

            if (!TCP::http_ok(c->self->recv_buffer) || http_has_chunked(c->self->recv_buffer)) {
                c->failed = true; c->done = true;
                return ERR_OK;
            }

            const char* body = http_body_start(c->self->recv_buffer);
            if (body) {
                const char *key = "\"token\":\"";
                const char *pos = strstr(body, key);
                if (pos) {
                    pos += strlen(key);
                    size_t i = 0;
                    while (*pos && *pos != '"' && i + 1 < sizeof(c->self->received_token)) {
                        c->self->received_token[i++] = *pos++;
                    }
                    c->self->received_token[i] = '\0';
                }
            }
            c->done = true;
            return ERR_OK;
        }

        u16_t ack = 0;
        for (struct pbuf *q = p; q; q = q->next) {
            size_t copy = q->len;
            if (c->self->recv_len + copy >= sizeof(c->self->recv_buffer) - 1) {
                copy = (sizeof(c->self->recv_buffer) - 1) - c->self->recv_len;
            }
            if (copy > 0) {
                memcpy(c->self->recv_buffer + c->self->recv_len, q->payload, copy);
                c->self->recv_len += copy;
            }
            ack += q->len;
        }
        if (ack) tcp_recved(pcb, ack);
        pbuf_free(p);

        return ERR_OK;
    });

    tcp_poll(pcb, [](void *arg, struct tcp_pcb *pcb) -> err_t {
        auto *c = static_cast<token_ctx_t*>(arg);
        if (!c) return ERR_OK;
        if (++c->poll_ticks > 20) { 
            c->failed = true; c->done = true;
            tcp_abort(pcb);
            return ERR_ABRT;
        }
        return ERR_OK;
    }, 2);

    err_t result = tcp_connect(pcb, &server_ip, cfg.server_port,
        [](void *arg, struct tcp_pcb *pcb, err_t err) -> err_t {
            auto *c = static_cast<token_ctx_t*>(arg);
            if (!c || err != ERR_OK) return err;

            const auto &cfg2 = config_get();
            char host_line[64];
            snprintf(host_line, sizeof(host_line), "%s", cfg2.server_ip);

            std::string req = std::string("GET " TOKEN_PATH " HTTP/1.1\r\n")
                            + "Host: " + host_line + "\r\n"
                            + "User-Agent: pico-logger/1.0\r\n"
                            + "Connection: close\r\n\r\n";

            err_t w = tcp_write(pcb, req.c_str(), (u16_t)req.size(), TCP_WRITE_FLAG_COPY);
            if (w != ERR_OK) return w;
            return tcp_output(pcb);
        });

    if (result != ERR_OK) {
        tcp_abort(pcb);
        free(ctx);
        return false;
    }

    absolute_time_t deadline = make_timeout_time_ms(8000);
    while (!time_reached(deadline) && !ctx->done) {
        cyw43_arch_poll();
        sys_check_timeouts();
        sleep_ms(5);
    }

    tcp_close_or_abort(pcb, !ctx->failed);

    bool ok = strlen(received_token) > 0 && !ctx->failed;
    free(ctx);
    return ok;
}

/**
 * @brief lwIP TCP connected callback that sends a prepared request.
 *
 * On a successful connection, this callback retrieves the user context,
 * writes the null-terminated request string to the TCP socket using
 * TCP_WRITE_FLAG_COPY, and immediately flushes the output.
 *
 * @param arg User context set via tcp_arg(); expected to be a post_ctx_t*
 *            containing a valid, null-terminated request string in
 *            ctx->request. The request length must fit in u16_t.
 * @param pcb Active lwIP TCP protocol control block for the established connection.
 * @param err Connection result from lwIP. If not ERR_OK, no data is sent.
 *
 * @return
 * - ERR_OK on successful write and flush.
 * - An lwIP error code if the connection failed, tcp_write() fails, or tcp_output() fails.
 *
 * @pre
 * - arg points to a valid post_ctx_t with a non-null, null-terminated request.
 * - pcb is a valid, connected TCP PCB provided by lwIP.
 *
 * @note
 * - Uses TCP_WRITE_FLAG_COPY; ownership of ctx->request remains with the caller.
 * - Does not free the context or close the connection; higher-level logic must manage lifecycle.
 * - Invoked in the lwIP TCP/IP thread context.
 */
err_t TCP::tcp_connected_callback(void *arg, struct tcp_pcb *pcb, err_t err) {
    post_ctx_t *ctx = static_cast<post_ctx_t*>(arg);
    if (err != ERR_OK) return err;

    err_t w = tcp_write(pcb, ctx->request, (u16_t)strlen(ctx->request), TCP_WRITE_FLAG_COPY);
    if (w != ERR_OK) return w;
    return tcp_output(pcb);
}

/**
 * Sends one HTTP POST request with the current temperature, humidity, and atmospheric pressure
 * to the server defined in the active configuration.
 *
 * Behavior:
 * - Builds a JSON array of three entries (temperature, humidity, atmPressure), each including:
 *   - "time": the provided timestamp (or empty string if nullptr),
 *   - "value": the provided measurement (formatted to two decimal places),
 *   - "definition": one of "temperature", "humidity", "atmPressure",
 *   - "equLoggerId"/"equSensorId": taken from the current configuration.
 * - Resolves the server hostname/IP from configuration (5 s DNS timeout).
 * - Opens a TCP connection to cfg.server_port and sends an HTTP/1.1 POST to DATA_PATH with headers:
 *   Host, Authorization: Bearer <received_token>, User-Agent, Content-Type: application/json,
 *   Content-Length, Connection: close.
 * - Processes the HTTP response synchronously and succeeds only if the response is HTTP 200 OK
 *   and the response is not chunked (Transfer-Encoding: chunked is treated as failure).
 * - Blocks the caller while polling the network stack; overall wait is approximately up to 8 seconds.
 *
 * Parameters:
 * @param timestamp  ISO-8601-like string used as "time" for all three samples; if nullptr, an empty string is sent.
 * @param temp       Temperature value (Celsius); serialized with two decimal places.
 * @param hum        Relative humidity (percent); serialized with two decimal places.
 * @param pressure   Atmospheric pressure (e.g., hPa); serialized with two decimal places.
 *
 * Returns:
 * @return true  If the request was constructed, the TCP connection succeeded, an HTTP response was received,
 *               and it indicated success (HTTP 200) without chunked encoding.
 * @return false On memory allocation failure, DNS/TCP/connect errors, timeout, TCP error callback,
 *               non-OK HTTP status, or when the response uses chunked transfer encoding.
 *
 * Notes:
 * - The JSON body buffer is limited to 512 bytes.
 * - The function does not parse the response body; only the status line/headers are validated.
 * - Uses global configuration (server address/port, logger/sensor IDs) and an external bearer token.
 * - Closes the TCP connection on completion or aborts on error.
 *
 * Threading/Blocking:
 * - Synchronous and blocking; intended to be called from the context where cyw43_arch_poll/sys_check_timeouts
 *   can be serviced. Not suitable for time-critical paths without accommodating the ~8 s wait.
 */
bool TCP::send_data_post_request(const char* timestamp, float temp, float hum, float pressure) {
    const auto &cfg = config_get();

    char json_body[512];
    snprintf(json_body, sizeof(json_body),
        "["
        "{\"time\":\"%s\",\"value\":%.2f,\"definition\":\"temperature\",\"equLoggerId\":%u,\"equSensorId\":%u},"
        "{\"time\":\"%s\",\"value\":%.2f,\"definition\":\"humidity\",\"equLoggerId\":%u,\"equSensorId\":%u},"
        "{\"time\":\"%s\",\"value\":%.2f,\"definition\":\"atmPressure\",\"equLoggerId\":%u,\"equSensorId\":%u}"
        "]",
        timestamp ? timestamp : "", temp, cfg.logger_id, cfg.sensor_id,
        timestamp ? timestamp : "", hum, cfg.logger_id, cfg.sensor_id,
        timestamp ? timestamp : "", pressure, cfg.logger_id, cfg.sensor_id
    );

    post_ctx_t* ctx = (post_ctx_t*)calloc(1, sizeof(post_ctx_t));
    if (!ctx) return false;

    const char* host_header = cfg.server_ip;

    snprintf(ctx->request, sizeof(ctx->request),
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Authorization: Bearer %s\r\n"
        "User-Agent: pico-logger/1.0\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        DATA_PATH, host_header, received_token,
        strlen(json_body), json_body
    );

    ip_addr_t server_ip;
    if (!resolve_host_blocking(cfg.server_ip, &server_ip, 5000)) {
        free(ctx);
        return false;
    }

    struct tcp_pcb* pcb = tcp_new();
    if (!pcb) {
        free(ctx);
        return false;
    }

    tcp_arg(pcb, ctx);

    tcp_err(pcb, [](void *arg, err_t) {
        auto *c = static_cast<post_ctx_t*>(arg);
        if (c) { c->failed = true; c->done = true; }
    });

    tcp_recv(pcb, [](void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) -> err_t {
        auto *c = static_cast<post_ctx_t*>(arg);
        if (!c) {
            if (p) pbuf_free(p);
            tcp_abort(pcb);
            return ERR_ABRT;
        }

        if (err != ERR_OK) {
            if (p) pbuf_free(p);
            c->failed = true; c->done = true;
            tcp_abort(pcb);
            return ERR_ABRT;
        }

        if (!p) {
            c->response[c->response_len] = '\0';

            if (!TCP::http_ok(c->response) || http_has_chunked(c->response)) {
                c->failed = true;
            }

            c->done = true;
            return ERR_OK;
        }

        u16_t ack = 0;
        for (struct pbuf* q = p; q; q = q->next) {
            size_t copy = q->len;
            if (c->response_len + copy >= sizeof(c->response) - 1) {
                copy = (sizeof(c->response) - 1) - c->response_len;
            }
            if (copy > 0) {
                memcpy(c->response + c->response_len, q->payload, copy);
                c->response_len += copy;
            }
            ack += q->len;
        }
        if (ack) tcp_recved(pcb, ack);
        pbuf_free(p);

        return ERR_OK;
    });

    tcp_poll(pcb, [](void *arg, struct tcp_pcb *pcb) -> err_t {
        auto *c = static_cast<post_ctx_t*>(arg);
        if (!c) return ERR_OK;
        if (++c->poll_ticks > 20) {
            c->failed = true; c->done = true;
            tcp_abort(pcb);
            return ERR_ABRT;
        }
        return ERR_OK;
    }, 2);

    err_t result = tcp_connect(pcb, &server_ip, cfg.server_port, tcp_connected_callback);
    if (result != ERR_OK) {
        tcp_abort(pcb);
        free(ctx);
        return false;
    }

    absolute_time_t deadline = make_timeout_time_ms(8000);
    while (!time_reached(deadline) && !ctx->done) {
        cyw43_arch_poll();
        sys_check_timeouts();
        sleep_ms(5);
    }

    tcp_close_or_abort(pcb, !ctx->failed);

    bool ok = !ctx->failed;
    free(ctx);
    return ok;
}

/**
 * Sends a single error log entry to the configured HTTP endpoint via a blocking TCP/HTTP POST.
 *
 * The function:
 * - Constructs a JSON payload including equipmentId (from configuration), message, details,
 *   severity="error", and type="Equipment".
 * - Resolves the server, opens a TCP connection to cfg.server_ip:cfg.server_port, and POSTs to ERROR_PATH.
 * - Waits for the HTTP response and treats the operation as successful only if:
 *   - The exchange completes before the timeout (~8 seconds),
 *   - The response status is OK (2xx), and
 *   - The response is not chunked (Transfer-Encoding: chunked is rejected).
 *
 * This call drives the network stack by polling (cyw43_arch_poll/sys_check_timeouts) and blocks
 * the caller until completion or timeout. It is not suitable for ISR context.
 *
 * Limitations and notes:
 * - message and details may be nullptr; nulls are serialized as empty strings.
 * - message and details are inserted verbatim into JSON; they must be valid UTF-8 and must not contain
 *   unescaped double quotes (") or control characters, otherwise the JSON may become invalid.
 * - The JSON body is limited to 512 bytes; overly long inputs will be truncated.
 * - DNS/connection/write/read errors, timeouts, non-2xx responses, and chunked responses cause failure.
 *
 * Thread-safety:
 * - Uses shared networking resources; concurrent calls may require external serialization depending
 *   on the environment and lwIP configuration.
 *
 * Preconditions:
 * - Wi-Fi/network stack initialized and configured.
 * - cfg.server_ip, cfg.server_port, and ERROR_PATH are valid.
 *
 * @param message  Null-terminated error message string (may be nullptr, treated as "").
 * @param details  Null-terminated details string (may be nullptr, treated as "").
 * @return true if a non-chunked HTTP 2xx response is received before timeout; false otherwise.
 */
bool TCP::send_error_log(const char* message, const char* details) {
    const auto &cfg = config_get();

    char json_body[512];
    snprintf(json_body, sizeof(json_body),
        "{"
        "\"equipmentId\":%u,"
        "\"message\":\"%s\","
        "\"details\":\"%s\","
        "\"severity\":\"error\","
        "\"type\":\"Equipment\""
        "}",
        cfg.logger_id,
        message ? message : "",
        details ? details : ""
    );

    post_ctx_t* ctx = (post_ctx_t*)calloc(1, sizeof(post_ctx_t));
    if (!ctx) return false;

    const char* host_header = cfg.server_ip;

    snprintf(ctx->request, sizeof(ctx->request),
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: pico-logger/1.0\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        ERROR_PATH, host_header,
        strlen(json_body), json_body
    );

    ip_addr_t server_ip;
    if (!resolve_host_blocking(cfg.server_ip, &server_ip, 5000)) {
        free(ctx);
        return false;
    }

    struct tcp_pcb* pcb = tcp_new();
    if (!pcb) {
        free(ctx);
        return false;
    }

    tcp_arg(pcb, ctx);

    tcp_err(pcb, [](void *arg, err_t) {
        auto *c = static_cast<post_ctx_t*>(arg);
        if (c) { c->failed = true; c->done = true; }
    });

    tcp_recv(pcb, [](void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err) -> err_t {
        auto *c = static_cast<post_ctx_t*>(arg);
        if (!c) {
            if (p) pbuf_free(p);
            tcp_abort(pcb);
            return ERR_ABRT;
        }

        if (err != ERR_OK) {
            if (p) pbuf_free(p);
            c->failed = true; c->done = true;
            tcp_abort(pcb);
            return ERR_ABRT;
        }

        if (!p) {
            c->response[c->response_len] = '\0';
            if (!TCP::http_ok(c->response) || http_has_chunked(c->response)) {
                c->failed = true;
            }
            c->done = true;
            return ERR_OK;
        }

        u16_t ack = 0;
        for (struct pbuf* q = p; q; q = q->next) {
            size_t copy = q->len;
            if (c->response_len + copy >= sizeof(c->response) - 1) {
                copy = (sizeof(c->response) - 1) - c->response_len;
            }
            if (copy > 0) {
                memcpy(c->response + c->response_len, q->payload, copy);
                c->response_len += copy;
            }
            ack += q->len;
        }
        if (ack) tcp_recved(pcb, ack);
        pbuf_free(p);

        return ERR_OK;
    });

    tcp_poll(pcb, [](void *arg, struct tcp_pcb *pcb) -> err_t {
        auto *c = static_cast<post_ctx_t*>(arg);
        if (!c) return ERR_OK;
        if (++c->poll_ticks > 20) {
            c->failed = true; c->done = true;
            tcp_abort(pcb);
            return ERR_ABRT;
        }
        return ERR_OK;
    }, 2);

    err_t result = tcp_connect(pcb, &server_ip, cfg.server_port,
        [](void *arg, struct tcp_pcb *pcb, err_t err) -> err_t {
            auto *ctx = static_cast<post_ctx_t*>(arg);
            if (err != ERR_OK) return err;
            err_t w = tcp_write(pcb, ctx->request, (u16_t)strlen(ctx->request), TCP_WRITE_FLAG_COPY);
            if (w != ERR_OK) return w;
            return tcp_output(pcb);
        });

    if (result != ERR_OK) {
        tcp_abort(pcb);
        free(ctx);
        return false;
    }

    absolute_time_t deadline = make_timeout_time_ms(8000);
    while (!time_reached(deadline) && !ctx->done) {
        cyw43_arch_poll();
        sys_check_timeouts();
        sleep_ms(5);
    }

    tcp_close_or_abort(pcb, !ctx->failed);

    bool ok = !ctx->failed;
    free(ctx);
    return ok;
}