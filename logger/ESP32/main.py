import time, ntptime
from machine import UART
from wireless import WiFi
from stm32_uart import STM32UART
from program import Program
from config import Config

cfg = Config()


def main():
    wifi = WiFi(cfg.get_ssid(), cfg.get_password())

    program = Program()

    try:
        wifi.connect()
        ip_address = wifi.get_ip()
    except Exception as e:
        ip_address = None
        program.errors["WIFI_CONNECT"] = e
    if ip_address is None:
        program.error_management("WIFI", "Wifi could not be connected")
        program.set_client(None)
    else:
        program.set_ip_address(ip_address)
        try:
            ntptime.host = cfg.get_get_ntp_server_ip()
            ntptime.settime()
            program.mqtt_initialization()
        except Exception as e:
            program.error_management(
                "MQTT",
                "MQTT client initialization failed: {}".format(e),
            )
            program.set_client(None)

    if program.get_client() is not None:
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

    try:
        uart_device = UART(
            1, baudrate=115200, tx=20, rx=21, timeout=50, timeout_char=10
        )
        stm32 = STM32UART(uart_device)

        program.stm32 = stm32

        time.sleep(4)

        stm32.req_ping()
        program.set_time()
        time.sleep(0.1)
        control_date = stm32.req_rtc_read()
        while control_date.startswith("2000"):
            program.set_time()
            time.sleep(0.1)
            control_date = stm32.req_rtc_read()

    except Exception as e:
        program.error_management("STM32", "STM32 communication failed: {}".format(e))

    wifi_reconnect_ms = 5000
    mqtt_reconnect_ms = 5000
    last_wifi_try = time.ticks_ms()
    last_mqtt_try = time.ticks_ms()
    ntp_done_after_wifi = False

    while True:
        now = time.ticks_ms()

        if not wifi.is_connected():
            ntp_done_after_wifi = False
            c = program.get_client()
            if c is not None:
                try:
                    c.disconnect()
                except:
                    pass
                program.set_client(None)

            if time.ticks_diff(now, last_wifi_try) >= wifi_reconnect_ms:
                last_wifi_try = now
                try:
                    wifi.connect(timeout=5)
                    ip_address = wifi.get_ip()
                    program.set_ip_address(ip_address)
                except Exception as e:
                    program.error_management("WIFI", "Reconnect failed: {}".format(e))

            time.sleep(0.2)
            continue

        if not ntp_done_after_wifi:
            try:
                ntptime.host = cfg.get_get_ntp_server_ip()
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
                        "RECONNECTED", "STATUS", {"ip_address": wifi.get_ip()}
                    )
                except Exception as e:
                    program.error_management("MQTT", "Reconnect failed: {}".format(e))
                    program.set_client(None)

        if program.pending_read:
            program.pending_read = False
            program.read_data()

        c = program.get_client()
        if c is not None:
            try:
                c.check_msg()
            except Exception as e:
                program.error_management("MQTT", "MQTT check_msg failed: {}".format(e))
                try:
                    c.disconnect()
                except:
                    pass
                program.set_client(None)
        time.sleep(0.01)


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print("Fatal error: {}".format(e))
