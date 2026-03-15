class Config:
    def __init__(self):
        self.WIFI_ENABLED = 0

        self.SECRET_PASS = ""

        self.LOGGER_ID = -1
        self.SENSOR_ID = -1
        self.SSID = ""
        self.PASSWORD = ""
        self.NTP_SERVER_IP = ""

        self.MQTT_SERVER = ""
        self.MQTT_PORT = 1883
        self.MQTT_USER = ""
        self.MQTT_PASSWORD = ""
        self.MQTT_KEEPALIVE = 7200

        self.STATUS_GPIO = 18
        self.STM32_GPIO = 19
        self.LED_GPIO = 22

        self.DEV_ADDR = 0xB2
        self.FRAME_LEN_APP = 24
        self.STATUS_OK = 0x40
        self.STATUS_ERR = 0x7F

        self.FRAM_ADDR_LOGGER_ID = 0x008
        self.FRAM_ADDR_SENSOR_ID = 0x028
        self.FRAM_ADDR_MEASURE_INTERVAL = 0x128

        self.WIFI_RECONNECT_MS = 5000
        self.MQTT_RECONNECT_MS = 5000
        self.WIFI_CONNECT_TIMEOUT = 5
        self.DEBOUNCE_IRQ_MS = 50
        self.POLL_INTERVAL_MS = 10
        self.NO_WIFI_SLEEP_MS = 200
        self.RTC_SYNC_MAX_RETRIES = 10

    def is_wifi_enabled(self):
        return self.WIFI_ENABLED

    def set_wifi_enabled(self, enabled):
        self.WIFI_ENABLED = enabled

    def get_secret_pass(self):
        return self.SECRET_PASS

    def set_secret_pass(self, secret_pass):
        self.SECRET_PASS = secret_pass

    def get_logger_id(self):
        return self.LOGGER_ID

    def set_logger_id(self, logger_id):
        self.LOGGER_ID = logger_id

    def get_sensor_id(self):
        return self.SENSOR_ID

    def set_sensor_id(self, sensor_id):
        self.SENSOR_ID = sensor_id

    def get_ssid(self):
        return self.SSID

    def set_ssid(self, ssid):
        self.SSID = ssid

    def get_password(self):
        return self.PASSWORD

    def set_password(self, password):
        self.PASSWORD = password

    def get_ntp_server_ip(self):
        return self.NTP_SERVER_IP

    def set_ntp_server_ip(self, ntp_server_ip):
        self.NTP_SERVER_IP = ntp_server_ip

    def get_mqtt_server(self):
        return self.MQTT_SERVER

    def set_mqtt_server(self, mqtt_server):
        self.MQTT_SERVER = mqtt_server

    def get_mqtt_port(self):
        return self.MQTT_PORT

    def set_mqtt_port(self, mqtt_port):
        self.MQTT_PORT = mqtt_port

    def get_mqtt_user(self):
        return self.MQTT_USER

    def set_mqtt_user(self, mqtt_user):
        self.MQTT_USER = mqtt_user

    def get_mqtt_password(self):
        return self.MQTT_PASSWORD

    def set_mqtt_password(self, mqtt_password):
        self.MQTT_PASSWORD = mqtt_password

    def get_mqtt_keepalive(self):
        return self.MQTT_KEEPALIVE

    def get_status_gpio(self):
        return self.STATUS_GPIO

    def get_stm32_gpio(self):
        return self.STM32_GPIO

    def get_led_gpio(self):
        return self.LED_GPIO

    def get_dev_addr(self):
        return self.DEV_ADDR

    def get_frame_len_app(self):
        return self.FRAME_LEN_APP

    def get_status_ok(self):
        return self.STATUS_OK

    def get_status_err(self):
        return self.STATUS_ERR

    def get_fram_addr_logger_id(self):
        return self.FRAM_ADDR_LOGGER_ID

    def get_fram_addr_sensor_id(self):
        return self.FRAM_ADDR_SENSOR_ID

    def get_fram_addr_measure_interval(self):
        return self.FRAM_ADDR_MEASURE_INTERVAL

    def get_wifi_reconnect_ms(self):
        return self.WIFI_RECONNECT_MS

    def get_mqtt_reconnect_ms(self):
        return self.MQTT_RECONNECT_MS

    def get_wifi_connect_timeout(self):
        return self.WIFI_CONNECT_TIMEOUT

    def get_debounce_irq_ms(self):
        return self.DEBOUNCE_IRQ_MS

    def get_poll_interval_ms(self):
        return self.POLL_INTERVAL_MS

    def get_no_wifi_sleep_ms(self):
        return self.NO_WIFI_SLEEP_MS

    def get_rtc_sync_max_retries(self):
        return self.RTC_SYNC_MAX_RETRIES


_cfg_singleton = None


def get_config():
    global _cfg_singleton
    if _cfg_singleton is None:
        _cfg_singleton = Config()
    return _cfg_singleton
