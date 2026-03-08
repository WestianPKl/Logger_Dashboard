class Config:
    def __init__(self):
        self.LOGGER_ID = 361
        self.SENSOR_ID = 362

        self.WIFI_ENABLED = True
        self.WIFI_SSID = ""
        self.WIFI_PASSWORD = ""

        self.SERVER_IP = ""
        self.SERVER_PORT = 3000

        self.POST_TIME = 600

        self.BASE_URL = f"http://{self.SERVER_IP}:{self.SERVER_PORT}"
        self.TOKEN_URL = "/api/data/data-token"
        self.DATA_URL = "/api/data/data-log"
        self.ERROR_URL = "/api/common/error-log"

        self.TEMPERATURE = 1
        self.HUMIDITY = 1
        self.PRESSURE = 0
        self.SHT = 40
        self.CLOCK = 1
        self.LOGGING_ENABLED = 1
        self.SET_TIME = 1

        self.RELAY_VERSION = False
        self.RELAY1_PIN = 12
        self.RELAY2_PIN = 13
        self.RELAY3_PIN = 14
        self.RELAY4_PIN = 15

        self.backlight_flag = 0

        self.LED_PINS = [19, 20, 18]
        if self.RELAY_VERSION:
            self.LED_PINS = [7, 8, 6]

        self.BUZZER_PIN = 11
        if self.RELAY_VERSION:
            self.BUZZER_PIN = 13

        self.SWITCH1_PIN = 17
        self.SWITCH2_PIN = 16
        if self.RELAY_VERSION:
            self.SWITCH1_PIN = 21
            self.SWITCH2_PIN = 20

        self.TEMP_LOW_THRESHOLD = 20.0
        self.TEMP_HIGH_THRESHOLD = 25.0
        self.HUMIDITY_LOW_THRESHOLD = 30.0
        self.HUMIDITY_HIGH_THRESHOLD = 80.0

    def get_base_url(self):
        return self.BASE_URL

    def get_error_url(self):
        return self.ERROR_URL

    def get_token_url(self):
        return self.TOKEN_URL

    def get_data_url(self):
        return self.DATA_URL

    def get_logger_id(self):
        return self.LOGGER_ID

    def get_sensor_id(self):
        return self.SENSOR_ID

    def is_wifi_enabled(self):
        return self.WIFI_ENABLED

    def get_ssid(self):
        return self.WIFI_SSID

    def get_password(self):
        return self.WIFI_PASSWORD

    def get_server_config(self):
        return self.SERVER_IP, self.SERVER_PORT

    def get_post_time(self):
        return self.POST_TIME

    def is_temperature_enabled(self):
        return self.TEMPERATURE

    def is_humidity_enabled(self):
        return self.HUMIDITY

    def is_pressure_enabled(self):
        return self.PRESSURE

    def get_sht_type(self):
        return self.SHT

    def is_clock_enabled(self):
        return self.CLOCK

    def is_logging_enabled(self):
        return self.LOGGING_ENABLED

    def set_logging_enabled(self, enabled: bool):
        self.LOGGING_ENABLED = enabled

    def is_set_time_enabled(self):
        return self.SET_TIME

    def is_relay_version(self):
        return self.RELAY_VERSION

    def get_relay_pins(self):
        return self.RELAY1_PIN, self.RELAY2_PIN, self.RELAY3_PIN, self.RELAY4_PIN

    def get_led_pins(self):
        return self.LED_PINS

    def get_buzzer_pin(self):
        return self.BUZZER_PIN

    def get_switch_pins(self):
        return self.SWITCH1_PIN, self.SWITCH2_PIN

    def get_thresholds(self):
        return (
            self.TEMP_LOW_THRESHOLD,
            self.TEMP_HIGH_THRESHOLD,
            self.HUMIDITY_LOW_THRESHOLD,
            self.HUMIDITY_HIGH_THRESHOLD,
        )

    def get_backlight_flag(self):
        return self.backlight_flag

    def set_backlight_flag(self, flag: int):
        self.backlight_flag = flag
