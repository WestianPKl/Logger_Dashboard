/**
 * @file /Users/piotrklys/Documents/_Repositories/LoggerDashboard/logger/Pico2/Pico_TH_Logger_Relay_SMD_PCF8563T/com.hpp
 * @brief Communication utilities for USB CDC on the Pico TH Logger.
 *
 * Declares the non-blocking poll routine for the communication layer,
 * a query for whether the initial "ready" banner has been transmitted,
 * and the TinyUSB CDC receive callback.
 *
 * Include this header in application code integrating with TinyUSB.
 * Unless noted otherwise, functions are intended to be called from the main loop.
 */

/**
 * @brief Services the communication layer.
 *
 * Drives transmit/receive state machines and performs any deferred work.
 * Call this frequently from the main loop or a periodic task; the function
 * is non-blocking.
 *
 * @note Should not be called from interrupt context.
 */
 
/**
 * @brief Indicates whether the "ready" banner has been sent over CDC.
 *
 * @return true if the banner has already been transmitted; false otherwise.
 */
 
/**
 * @brief TinyUSB CDC receive callback.
 *
 * Invoked by TinyUSB when new data is available on a CDC interface.
 * Keep the implementation short; defer heavy processing to com_poll().
 *
 * @param itf CDC interface number that received data.
 * @warning Typically called from TinyUSB context (ISR or TinyUSB task, depending on configuration);
 *          avoid blocking operations and heavy processing here.
 * @see com_poll()
 */
#pragma once
#ifndef __COM_HPP__
#define __COM_HPP__

#include <cstddef>
#include <cstdint>

void com_poll();
bool com_ready_banner_sent();

extern "C" void tud_cdc_rx_cb(uint8_t);

#endif /* __COM_HPP__ */