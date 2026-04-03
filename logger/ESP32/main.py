import time, ntptime, sys, uselect
from machine import UART
from wireless import WiFi
from stm32_uart import STM32UART
from program import Program
from config import get_config
import esp

esp.osdebug(None)

cfg = get_config()


def main():
    fram_ok = True
    program = Program()

    try:
        uart_device = UART(
            1, baudrate=115200, tx=20, rx=21, timeout=50, timeout_char=10
        )
        stm32 = STM32UART(uart_device)
        program.stm32 = stm32

        time.sleep(4)

        if not stm32.req_ping():
            raise Exception("STM32 ping failed")

        program.sync_ids_from_stm32_fram()

        program.apply_config()

        logger_id = cfg.get_logger_id()
        if logger_id in ("", None, -1, "-1"):
            fram_ok = False
            program.error_management("FRAM", "Missing logger_id")
        elif cfg.is_wifi_enabled() and (
            cfg.get_ssid() == "" or cfg.get_mqtt_server() == ""
        ):
            fram_ok = False
            program.error_management("FRAM", "Incomplete network config")
    except Exception as e:
        fram_ok = False
        program.error_management("STM32", "STM32 communication failed: {}".format(e))

    wifi = None

    if fram_ok and cfg.is_wifi_enabled():
        wifi = WiFi(cfg.get_ssid(), cfg.get_password())
        program.set_wifi(wifi)

    ip_address = None

    if wifi is not None and cfg.is_wifi_enabled():
        try:
            wifi.connect()
            ip_address = wifi.get_ip()
        except (OSError, Exception) as e:
            ip_address = None
            program.errors["WIFI_CONNECT"] = e

    if ip_address is None and cfg.is_wifi_enabled():
        program.error_management("WIFI", "Wifi could not be connected")
        program.set_client(None)
    elif not cfg.is_wifi_enabled():
        pass
    else:
        program.set_ip_address(ip_address)
        try:
            ntptime.host = cfg.get_ntp_server_ip()
            ntptime.settime()
        except Exception as e:
            program.error_management("NTP", "NTP settime failed: {}".format(e))

        try:
            program.mqtt_initialization()
        except Exception as e:
            program.error_management(
                "MQTT",
                "MQTT client initialization failed: {}".format(e),
            )
            program.set_client(None)

    if program.get_client() is not None and cfg.is_wifi_enabled():
        program.send_errors()
        try:
            program.send_status(
                "START",
                "STATUS",
                {
                    "communication_sw": program.status_data.get("version", ""),
                    "communication_build": program.status_data.get("build", ""),
                    "logger_id": program.status_data.get("logger_id", ""),
                    "sensor_id": program.status_data.get("sensor_id", ""),
                    "ip_address": ip_address,
                },
            )
        except Exception as e:
            program.error_management("MQTT", "Failed to send status: {}".format(e))

    if program.stm32 is not None and ip_address is not None and cfg.is_wifi_enabled():
        try:
            program.set_time()
            time.sleep(0.1)

            control_date = program.stm32.req_rtc_read()
            tries = 0
            max_tries = cfg.get_rtc_sync_max_retries()

            while control_date.startswith("2000") and tries < max_tries:
                program.set_time()
                time.sleep(0.1)
                control_date = program.stm32.req_rtc_read()
                tries += 1

            if control_date.startswith("2000"):
                program.error_management(
                    "STM32",
                    "Failed to synchronize time after {} attempts".format(max_tries),
                )
        except Exception as e:
            program.error_management(
                "STM32", "Failed to synchronize time: {}".format(e)
            )

    wifi_reconnect_ms = cfg.get_wifi_reconnect_ms()
    mqtt_reconnect_ms = cfg.get_mqtt_reconnect_ms()
    last_wifi_try = time.ticks_ms()
    last_mqtt_try = time.ticks_ms()
    ntp_done_after_wifi = False

    poll = uselect.poll()
    poll.register(sys.stdin, uselect.POLLIN)

    while True:
        now = time.ticks_ms()

        events = poll.poll(0)
        if events:
            line = sys.stdin.readline()
            if line:
                line = line.strip()
                program.read_usb(line)

        if program.pending_restart:
            program.pending_restart = False
            program.restart_connections()

            wifi = None
            if cfg.is_wifi_enabled():
                wifi = WiFi(cfg.get_ssid(), cfg.get_password())
                program.set_wifi(wifi)
            else:
                program.set_wifi(None)

            ip_address = None
            program.set_ip_address(None)
            ntp_done_after_wifi = False
            last_wifi_try = time.ticks_ms()
            last_mqtt_try = time.ticks_ms()

            time.sleep(0.2)
            continue

        if program.pending_read:
            program.pending_read = False
            program.read_data()

        if not cfg.is_wifi_enabled():
            time.sleep(cfg.get_poll_interval_ms() / 1000.0)
            continue

        if cfg.get_ssid() == "" or cfg.get_password() == "":
            time.sleep(cfg.get_no_wifi_sleep_ms() / 1000.0)
            continue

        if wifi is None:
            c = program.get_client()
            if c is not None:
                try:
                    c.disconnect()
                except (OSError, Exception):
                    pass
                program.set_client(None)

            time.sleep(cfg.get_no_wifi_sleep_ms() / 1000.0)
            continue

        if not wifi.is_connected():
            ntp_done_after_wifi = False

            c = program.get_client()
            if c is not None:
                try:
                    c.disconnect()
                except (OSError, Exception):
                    pass
                program.set_client(None)

            if time.ticks_diff(now, last_wifi_try) >= wifi_reconnect_ms:
                last_wifi_try = now
                try:
                    wifi.connect(timeout=cfg.get_wifi_connect_timeout())
                    ip_address = wifi.get_ip()
                    program.set_ip_address(ip_address)
                except (OSError, Exception) as e:
                    program.error_management("WIFI", "Reconnect failed: {}".format(e))

            time.sleep(cfg.get_no_wifi_sleep_ms() / 1000.0)
            continue

        if not ntp_done_after_wifi:
            try:
                ntptime.host = cfg.get_ntp_server_ip()
                ntptime.settime()
                program.set_time()
                ntp_done_after_wifi = True
            except Exception as e:
                program.error_management("NTP", "settime failed: {}".format(e))

        if program.get_client() is None:
            if time.ticks_diff(now, last_mqtt_try) >= mqtt_reconnect_ms:
                last_mqtt_try = now
                try:
                    program.mqtt_initialization()
                    program.send_errors()
                    program.send_status(
                        "RECONNECTED",
                        "STATUS",
                        {"ip_address": wifi.get_ip()},
                    )
                except Exception as e:
                    program.error_management("MQTT", "Reconnect failed: {}".format(e))
                    program.set_client(None)

        c = program.get_client()
        if c is not None:
            try:
                c.check_msg()
            except (OSError, Exception) as e:
                program.error_management("MQTT", "MQTT check_msg failed: {}".format(e))
                try:
                    c.disconnect()
                except (OSError, Exception):
                    pass
                program.set_client(None)

        time.sleep(cfg.get_poll_interval_ms() / 1000.0)


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print("Fatal error: {}".format(e))
