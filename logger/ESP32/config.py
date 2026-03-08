class Config:
    def __init__(self):
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

    def get_ssid(self):
        return self.SSID

    def get_password(self):
        return self.PASSWORD

    def get_get_ntp_server_ip(self):
        return self.NTP_SERVER_IP

    def get_mqtt_server(self):
        return self.MQTT_SERVER

    def get_mqtt_port(self):
        return self.MQTT_PORT

    def get_mqtt_user(self):
        return self.MQTT_USER

    def get_mqtt_password(self):
        return self.MQTT_PASSWORD

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
