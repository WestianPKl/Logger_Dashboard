import machine, time, ujson, os
from machine import RTC, Pin
from mqtt_simple import MQTTClient
from config import get_config
from nvs_system import NVSSystem
from hash import ServiceAuth

cfg = get_config()
nvs_sys = NVSSystem()
auth = ServiceAuth(cfg.get_secret_pass())


class Program:
    def __init__(self):
        self.service_mode = False
        self.service_mode_until = 0
        self.client = None
        self.status_data = {}
        self.load_data()
        self.device_id = ""
        self.topic_status = b""
        self.ip_address = None
        self.stm32_gpio = Pin(cfg.get_stm32_gpio(), Pin.OUT)
        self.led = Pin(cfg.get_led_gpio(), Pin.OUT)
        self.pending_read = False
        self.pending_restart = False
        self.irq_dropped = 0
        self.last_irq_ms = 0
        self.status = Pin(cfg.get_status_gpio(), Pin.IN, Pin.PULL_DOWN)
        self.status.irq(
            handler=self._irq_status,
            trigger=Pin.IRQ_RISING,
        )
        self.rtc = RTC()
        self.stm32 = None
        self.errors = {}
        self.wifi = None

        self.input_channels = [
            (0x01, "BTN1"),
            (0x02, "BTN2"),
            (0x03, "ESP32_STATUS"),
        ]

        self.output_channels = [
            (0x01, "LED1"),
            (0x02, "LED2"),
            (0x03, "PB12"),
            (0x04, "PC0"),
            (0x05, "PC1"),
            (0x06, "PC2"),
            (0x07, "PC3"),
            (0x08, "ESP32_STATUS"),
        ]

        self.pwm_channels = [
            (0x01, "TIM1_CH1"),
            (0x02, "TIM2_CH3"),
            (0x03, "TIM4_CH3"),
            (0x04, "TIM4_CH4"),
        ]

    def _irq_status(self, pin):
        now = time.ticks_ms()
        if self.pending_read:
            self.irq_dropped += 1
            return
        if time.ticks_diff(now, self.last_irq_ms) < cfg.get_debounce_irq_ms():
            return
        self.last_irq_ms = now
        self.pending_read = True

    def _get_service_ymd(self):
        try:
            rtc = self.stm32.req_rtc_read()
            if rtc and len(rtc) >= 10:
                return rtc[0:4] + rtc[5:7] + rtc[8:10]
        except Exception:
            pass

        dt = time.localtime()
        return "{:04d}{:02d}{:02d}".format(dt[0], dt[1], dt[2])

    def service_mode_enable(self, duration_sec):
        self.service_mode = True
        self.service_mode_until = time.time() + duration_sec

    def service_mode_disable(self):
        self.service_mode = False
        self.service_mode_until = 0

    def service_mode_status(self):
        if self.service_mode and time.time() > self.service_mode_until:
            self.service_mode = False
            self.service_mode_until = 0
        return self.service_mode

    def set_wifi(self, wifi):
        self.wifi = wifi

    def apply_config(self):
        logger_id = self.status_data.get("logger_id", "")
        sensor_id = self.status_data.get("sensor_id", "")
        wifi_ssid = self.status_data.get("wifi_ssid", "")
        mqtt_server = self.status_data.get("mqtt_server", "")
        ntp_server = self.status_data.get("ntp_server", "")
        wifi_enabled = self.status_data.get("wifi_enabled", 0)

        wifi_password = nvs_sys.get_secret("wifi_password", "")
        mqtt_user = nvs_sys.get_secret("mqtt_username", "")
        mqtt_password = nvs_sys.get_secret("mqtt_password", "")

        cfg.set_logger_id(logger_id)
        cfg.set_sensor_id(sensor_id)
        cfg.set_ssid(wifi_ssid)
        cfg.set_password(wifi_password)
        cfg.set_mqtt_server(mqtt_server)
        cfg.set_ntp_server_ip(ntp_server)
        cfg.set_mqtt_user(mqtt_user)
        cfg.set_mqtt_password(mqtt_password)
        cfg.set_wifi_enabled(wifi_enabled)

        self.device_id = str(logger_id)
        self.topic_status = "devices/{}/status".format(self.device_id).encode()

    def get_client(self):
        return self.client

    def set_client(self, client):
        self.client = client

    def set_ip_address(self, ip_address):
        self.ip_address = ip_address

    def _get_base_payload(self):
        return {
            "communication_sw": self.status_data.get("version", ""),
            "communication_build": self.status_data.get("build", ""),
            "logger_id": self.status_data.get("logger_id", ""),
            "sensor_id": self.status_data.get("sensor_id", ""),
            "ip_address": self.ip_address,
        }

    def mask_secret(self, value):
        if not value:
            return ""
        if len(value) <= 2:
            return "*" * len(value)
        return value[0] + ("*" * (len(value) - 2)) + value[-1]

    def restart_connections(self):
        c = self.get_client()
        if c is not None:
            try:
                c.disconnect()
            except (OSError, Exception):
                pass
            self.set_client(None)

        if self.wifi is not None:
            try:
                self.wifi.disconnect()
            except (OSError, Exception):
                pass
            try:
                self.wifi.deinit()
            except (OSError, Exception):
                pass

        self.wifi = None

    def mqtt_initialization(self):
        if self.client is not None:
            return self.client

        mqtt_user = cfg.get_mqtt_user()
        mqtt_password = cfg.get_mqtt_password()

        user = mqtt_user.encode("utf-8") if mqtt_user else None
        password = mqtt_password.encode("utf-8") if mqtt_password else None

        self.client = MQTTClient(
            client_id=str(self.device_id).encode("utf-8"),
            server=cfg.get_mqtt_server(),
            port=cfg.get_mqtt_port(),
            user=user,
            password=password,
            keepalive=cfg.get_mqtt_keepalive(),
        )

        self.client.set_callback(self.message_mqtt)
        self.client.connect()
        self.client.subscribe("devices/{}/cmd".format(self.device_id).encode("utf-8"))
        return self.client

    def send_status(self, result, typeData="STATUS", info=None, timestamp=None):
        payload = {
            "result": result,
            "info": info,
            "type": typeData,
            "timestamp": time.time() if timestamp is None else timestamp,
        }
        if self.client is None:
            raise Exception("MQTT client is not connected")
        self.client.publish(self.topic_status, ujson.dumps(payload).encode())

    def _find_channel(self, channels, name):
        for ch_id, ch_name in channels:
            if ch_name == name:
                return ch_id
        return None

    def message_mqtt(self, topic, msg):
        try:
            data = ujson.loads(msg)
            cmd = data.get("cmd")
            params = data.get("params", {})

            self.service_mode_status()

            if cmd == "PING":
                self.send_status(
                    "ALIVE",
                    "STATUS",
                    self._get_base_payload(),
                )
            elif cmd == "RESET":
                if not self.service_mode:
                    raise Exception("Reset command is only allowed in service mode")
                self.send_status(
                    "RESET",
                    "STATUS",
                    self._get_base_payload(),
                )
                time.sleep(0.1)
                self.stm32.req_reset()
            elif cmd == "SYNC_TIME":
                dt = self.set_time()
                sync_time = "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z".format(
                    dt[0], dt[1], dt[2], dt[4], dt[5], dt[6]
                )
                date = self.stm32.req_rtc_read()
                self.send_status(
                    "TIME_SYNCED",
                    "STATUS",
                    {
                        "rtc_time": date,
                        "sync_time": sync_time,
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "READ_RTC":
                date = self.stm32.req_rtc_read()
                self.send_status(
                    "RTC_READ",
                    "STATUS",
                    {
                        "rtc_time": date,
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "READ_INPUTS":
                inputs = {}
                for channel_id, channel_name in self.input_channels:
                    value = self.stm32.req_get_input_states(channel_id)
                    inputs[channel_name] = value
                self.send_status("INPUTS_READ", "STATUS", inputs)
            elif cmd == "SET_OUTPUT":
                channel_id = self._find_channel(
                    self.output_channels, params.get("channel")
                )
                if channel_id is None:
                    raise Exception(
                        "Unknown output channel: {}".format(params.get("channel"))
                    )
                self.stm32.req_set_output(channel_id, params.get("value", 0))
                self.send_status(
                    "OUTPUT_SET",
                    "STATUS",
                    {
                        "channel": params.get("channel"),
                        "value": params.get("value", 0),
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "SET_PWM":
                channel_id = self._find_channel(
                    self.pwm_channels, params.get("channel")
                )
                if channel_id is None:
                    raise Exception(
                        "Unknown PWM channel: {}".format(params.get("channel"))
                    )
                self.stm32.req_set_pwm(channel_id, params.get("duty_cycle", 0))
                self.send_status(
                    "PWM_SET",
                    "STATUS",
                    {
                        "channel": params.get("channel"),
                        "duty_cycle": params.get("duty_cycle", 0),
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "RGB":
                self.stm32.req_set_rgb(
                    params.get("r", 0),
                    params.get("g", 0),
                    params.get("b", 0),
                    params.get("brightness", 100),
                )
                self.send_status(
                    "RGB_SET",
                    "STATUS",
                    {
                        "r": params.get("r", 0),
                        "g": params.get("g", 0),
                        "b": params.get("b", 0),
                        "brightness": params.get("brightness", 100),
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "BUZZER":
                self.stm32.req_set_buzzer(
                    params.get("freq", 1000), params.get("volume", 100)
                )
                self.send_status(
                    "BUZZER_SET",
                    "STATUS",
                    {
                        "freq": params.get("freq", 1000),
                        "volume": params.get("volume", 100),
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "READ_INA":
                bus_voltage, shunt_voltage, current, power, id = (
                    self.stm32.req_ina_read()
                )
                self.send_status(
                    "INA_READ",
                    "STATUS",
                    {
                        "bus_voltage": bus_voltage,
                        "shunt_voltage": shunt_voltage,
                        "current": current,
                        "power": power,
                        "id": id,
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "READ_SHT40":
                t, h = self.stm32.req_sht40()
                self.send_status(
                    "SHT40_READ",
                    "STATUS",
                    {
                        "temperature": t,
                        "humidity": h,
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "READ_BME280":
                tb, hb, pb = self.stm32.req_bme280()
                self.send_status(
                    "BME280_READ",
                    "STATUS",
                    {
                        "temperature": tb,
                        "humidity": hb,
                        "pressure": pb,
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "LCD_CLEAR":
                if not self.service_mode:
                    raise Exception("LCD clear command is only allowed in service mode")
                self.stm32.req_lcd_clear()
                self.send_status(
                    "LCD_CLEARED",
                    "STATUS",
                    {
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "LCD_BACKLIGHT":
                if not self.service_mode:
                    raise Exception(
                        "LCD backlight command is only allowed in service mode"
                    )

                self.stm32.req_lcd_set_backlight(params.get("state", 0))
                self.send_status(
                    "LCD_BACKLIGHT_SET",
                    "STATUS",
                    {
                        "state": params.get("state", 0),
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "WRITE_FRAM":
                if not self.service_mode:
                    raise Exception(
                        "FRAM write command is only allowed in service mode"
                    )

                ok = self.stm32.req_fram_write(
                    params.get("address", 0), bytes(params.get("data", []))
                )
                self.send_status(
                    "FRAM_WRITE",
                    "STATUS",
                    {
                        "address": params.get("address", 0),
                        "data": params.get("data", []),
                        "ok": ok,
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "READ_FRAM":
                if not self.service_mode:
                    raise Exception("FRAM read command is only allowed in service mode")

                data = self.stm32.req_fram_read(
                    params.get("address", 0), params.get("length", 0)
                )
                self.send_status(
                    "FRAM_READ",
                    "STATUS",
                    {
                        "address": params.get("address", 0),
                        "data": list(data) if data is not None else None,
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "READ_FLAGS":
                flags = self.stm32.req_fram_read_flags()
                flags["WIFI_ENABLED"] = self.status_data.get("wifi_enabled", 0)
                self.send_status(
                    "FLAGS_READ",
                    "STATUS",
                    {
                        "flags": flags,
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "WRITE_FLAGS":
                if not self.service_mode:
                    raise Exception(
                        "Flags write command is only allowed in service mode"
                    )

                flags_data = params.get("flags", 0)
                wifi_flag = params.get("wifi_enabled", 0)

                if wifi_flag not in (0, 1):
                    raise Exception("Invalid wifi_enabled value: {}".format(wifi_flag))

                self.status_data["wifi_enabled"] = wifi_flag
                self.save_data()
                self.apply_config()

                if isinstance(flags_data, str):
                    flags_byte = int(flags_data, 0)
                else:
                    flags_byte = int(flags_data)

                if flags_byte < 0 or flags_byte > 255:
                    raise Exception("Invalid flags value: {}".format(flags_data))
                ok = self.stm32.req_fram_write_flags(flags_byte)
                self.send_status(
                    "FLAGS_WRITE",
                    "STATUS",
                    {
                        "flags": flags_byte,
                        "ok": ok,
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "READ_CONFIG":
                cfg_data = self.stm32.read_fram_config()
                cfg_data["wifi_ssid"] = self.status_data.get("wifi_ssid", "")
                wifi_password = nvs_sys.get_secret("wifi_password")
                cfg_data["mqtt_server"] = self.status_data.get("mqtt_server", "")
                cfg_data["ntp_server"] = self.status_data.get("ntp_server", "")
                mqtt_user = nvs_sys.get_secret("mqtt_username")
                mqtt_password = nvs_sys.get_secret("mqtt_password")
                if not self.service_mode:
                    cfg_data["wifi_password"] = (
                        self.mask_secret(wifi_password) if wifi_password else ""
                    )
                    cfg_data["mqtt_user"] = (
                        self.mask_secret(mqtt_user) if mqtt_user else ""
                    )
                    cfg_data["mqtt_password"] = (
                        self.mask_secret(mqtt_password) if mqtt_password else ""
                    )
                else:
                    cfg_data["wifi_password"] = wifi_password if wifi_password else ""
                    cfg_data["mqtt_user"] = mqtt_user if mqtt_user else ""
                    cfg_data["mqtt_password"] = mqtt_password if mqtt_password else ""
                self.send_status("CONFIG_READ", "STATUS", cfg_data)
            elif cmd == "WRITE_IDS":
                if not self.service_mode:
                    raise Exception("IDs write command is only allowed in service mode")

                ok1 = self.stm32.write_fram_config_field(
                    "logger_id", params.get("logger_id", "")
                )
                ok2 = self.stm32.write_fram_config_field(
                    "sensor_id", params.get("sensor_id", "")
                )
                ok = 1 if ok1 and ok2 else 0

                if ok:
                    self.apply_config()

                self.send_status(
                    "IDS_CONFIG_WRITE",
                    "STATUS",
                    {
                        "ok": ok,
                        "logger_id": params.get("logger_id", ""),
                        "sensor_id": params.get("sensor_id", ""),
                    },
                )

                if ok:
                    self.pending_restart = True
            elif cmd == "WRITE_WIFI":
                if not self.service_mode:
                    raise Exception(
                        "WiFi config write command is only allowed in service mode"
                    )

                ssid = params.get("wifi_ssid", "")
                password = params.get("wifi_password", "")

                ok = 0
                if ssid and password:
                    ok_pw = nvs_sys.set_secret("wifi_password", password)
                    if ok_pw:
                        self.status_data["wifi_ssid"] = ssid
                        self.save_data()
                        self.apply_config()
                        ok = 1

                self.send_status(
                    "WIFI_CONFIG_WRITE",
                    "STATUS",
                    {
                        "ok": ok,
                        "wifi_ssid": ssid,
                        "wifi_password": self.mask_secret(password),
                    },
                )

                if ok:
                    self.pending_restart = True
            elif cmd == "WRITE_SERVERS":
                if not self.service_mode:
                    raise Exception(
                        "Servers config write command is only allowed in service mode"
                    )

                mqtt_server = params.get("mqtt_server", "")
                ntp_server = params.get("ntp_server", "")

                ok = 0
                if mqtt_server and ntp_server:
                    self.status_data["mqtt_server"] = mqtt_server
                    self.status_data["ntp_server"] = ntp_server
                    self.save_data()
                    self.apply_config()
                    ok = 1

                self.send_status(
                    "SERVERS_CONFIG_WRITE",
                    "STATUS",
                    {
                        "ok": ok,
                        "mqtt_server": mqtt_server,
                        "ntp_server": ntp_server,
                    },
                )

                if ok:
                    self.pending_restart = True
            elif cmd == "WRITE_MQTT_CREDENTIALS":
                if not self.service_mode:
                    raise Exception(
                        "MQTT credentials config write command is only allowed in service mode"
                    )

                mqtt_user = params.get("mqtt_user", "")
                mqtt_password = params.get("mqtt_password", "")

                ok = 0
                if mqtt_user and mqtt_password:
                    ok_user = nvs_sys.set_secret("mqtt_username", mqtt_user)
                    ok_pw = nvs_sys.set_secret("mqtt_password", mqtt_password)
                    if ok_user and ok_pw:
                        self.apply_config()
                        ok = 1

                self.send_status(
                    "MQTT_CREDENTIALS_CONFIG_WRITE",
                    "STATUS",
                    {
                        "ok": ok,
                        "mqtt_user": mqtt_user,
                        "mqtt_password": self.mask_secret(mqtt_password),
                    },
                )

                if ok:
                    self.pending_restart = True
            elif cmd == "WRITE_FLASH":
                if not self.service_mode:
                    raise Exception(
                        "Flash write command is only allowed in service mode"
                    )

                ok = self.stm32.req_flash_write(
                    params.get("address", 0), bytes(params.get("data", []))
                )
                self.send_status(
                    "FLASH_WRITE",
                    "STATUS",
                    {
                        "address": params.get("address", 0),
                        "data": params.get("data", []),
                        "ok": ok,
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "READ_FLASH":
                if not self.service_mode:
                    raise Exception(
                        "Flash read command is only allowed in service mode"
                    )

                data = self.stm32.req_flash_read(
                    params.get("address", 0), params.get("length", 0)
                )
                self.send_status(
                    "FLASH_READ",
                    "STATUS",
                    {
                        "address": params.get("address", 0),
                        "data": list(data) if data is not None else None,
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "SERVICE_MODE_ENABLE":
                if self.service_mode:
                    raise Exception("Service mode is already enabled")

                password = params.get("password", "")
                ymd = self._get_service_ymd()

                if auth.verify_password_for_date(password, ymd):
                    self.service_mode_enable(300)
                    self.send_status(
                        "SERVICE_MODE_ENABLED",
                        "STATUS",
                        {
                            "duration_sec": 300,
                            "communication_sw": self.status_data.get("version", ""),
                            "communication_build": self.status_data.get("build", ""),
                            "logger_id": self.status_data.get("logger_id", ""),
                            "sensor_id": self.status_data.get("sensor_id", ""),
                            "ip_address": self.ip_address,
                        },
                    )
                else:
                    raise Exception("Invalid service mode password")
            elif cmd == "SERVICE_MODE_DISABLE":
                if not self.service_mode:
                    raise Exception("Service mode is not enabled")

                self.service_mode_disable()
                self.send_status(
                    "SERVICE_MODE_DISABLED",
                    "STATUS",
                    {
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            else:
                raise Exception("Unknown cmd: {}".format(cmd))
        except Exception as e:
            self.error_management("CMD", str(e))

    def load_data(self, filename="status.json"):
        try:
            with open(filename, "r") as f:
                self.status_data = ujson.load(f)
                if "pending_errors" not in self.status_data or not isinstance(
                    self.status_data["pending_errors"], list
                ):
                    self.status_data["pending_errors"] = []
        except Exception:
            self.status_data = {
                "communication_sw": "",
                "logger_id": "",
                "sensor_id": "",
                "communication_build": "",
                "ip_address": "",
                "wifi_ssid": "",
                "mqtt_server": "",
                "ntp_server": "",
                "wifi_enabled": 0,
                "pending_errors": [],
            }

    def save_data(self, filename="status.json"):
        tmp = filename + ".tmp"
        with open(tmp, "w") as f:
            ujson.dump(self.status_data, f)
        os.rename(tmp, filename)

    def error_dict(self, source, msg):
        return {
            "timestamp": time.time(),
            "source": source,
            "msg": str(msg),
            "logger_id": self.status_data.get("logger_id", ""),
            "sensor_id": self.status_data.get("sensor_id", ""),
        }

    def error_queue(self, payload, max_len=50):
        self.status_data["pending_errors"].append(payload)
        if len(self.status_data["pending_errors"]) > max_len:
            self.status_data["pending_errors"] = self.status_data["pending_errors"][
                -max_len:
            ]
        try:
            self.save_data()
        except Exception as e:
            self.errors["SAVE_STATUS"] = e

    def error_management(self, source, msg):
        payload = self.error_dict(source, msg)
        if self.client is None:
            self.error_queue(payload)
            return
        try:
            self.send_status("ERROR", "ERROR", payload)
        except Exception as e:
            self.error_queue(payload)
            self.error_queue(self.error_dict("SEND_STATUS", e))

    def send_errors(self):
        if self.client is None:
            return
        if not self.status_data.get("pending_errors"):
            return
        try:
            self.send_status(
                "ERRORS_FLUSH",
                "ERROR",
                {
                    "items": self.status_data["pending_errors"],
                    "communication_sw": self.status_data.get("version", ""),
                    "communication_build": self.status_data.get("build", ""),
                    "logger_id": self.status_data.get("logger_id", ""),
                    "sensor_id": self.status_data.get("sensor_id", ""),
                    "ip_address": self.ip_address,
                },
            )
            self.status_data["pending_errors"] = []
            self.save_data()
        except Exception as e:
            self.errors["SEND_ERRORS"] = e

    def set_time(self):
        dt = self.rtc.datetime()
        wd = (dt[3] % 7) + 1
        self.stm32.req_rtc_write(dt[0] - 2000, dt[1], dt[2], wd, dt[4], dt[5], dt[6])
        return dt

    def set_ext_time(self):
        dt = self.rtc.datetime()
        wd = (dt[3] % 7) + 1
        self.stm32.req_ext_rtc_write(
            dt[0] - 2000, dt[1], dt[2], wd, dt[4], dt[5], dt[6]
        )
        return dt

    def status_led(self, state: int):
        if state:
            self.led.on()
        else:
            self.led.off()

    def read_data(self):
        t0 = time.time()

        self.stm32_gpio.value(1)
        self.led.on()

        data = {
            "communication_sw": self.status_data.get("version", ""),
            "communication_build": self.status_data.get("build", ""),
            "logger_id": self.status_data.get("logger_id", ""),
            "sensor_id": self.status_data.get("sensor_id", ""),
            "ip_address": self.ip_address,
            "irq_dropped": self.irq_dropped,
        }

        try:
            if self.stm32.req_get_input_states(0x03):
                self.set_time()
                serial = self.stm32.req_serial()
                fw, hw = self.stm32.req_fw_hw_version()
                build_data = self.stm32.req_build_date()
                prod_date = self.stm32.req_prod_date()
                v0, v1 = self.stm32.req_adc()
                t, h = self.stm32.req_sht40()
                tb, hb, pb = self.stm32.req_bme280()
                date = self.stm32.req_rtc_read()

                v0_voltage = self.stm32.adc_to_voltage(v0)
                v1_voltage = self.stm32.adc_to_voltage(v1)

                data.update(
                    {
                        "controller_serial": serial,
                        "controller_sw": fw,
                        "controller_hw": hw,
                        "controller_build_date": build_data,
                        "controller_prod_date": prod_date,
                        "adc": [v0, v1],
                        "adc_voltage": [v0_voltage, v1_voltage],
                        "vin": [
                            self.stm32.vadc_to_vin(v0_voltage),
                            self.stm32.vadc_to_vin(v1_voltage),
                        ],
                        "rtc": date,
                        "stm32_error": 0,
                    }
                )
                if not (tb == 0 and hb == 0 and pb == 0):
                    data.update(
                        {
                            "bme280": {
                                "temperature": tb,
                                "humidity": hb,
                                "pressure": pb,
                            },
                            "bme280_error": 0,
                        }
                    )
                else:
                    data.update({"bme280_error": 1})
                if not (t == 0 and h == 0):
                    data.update(
                        {
                            "sht40": {"temperature": t, "humidity": h},
                            "sht40_error": 0,
                        }
                    )
                else:
                    data.update({"sht40_error": 1})
            else:
                data.update({"stm32_error": 1})

            if self.client is not None:
                self.send_status("DATA", "DATA", data, timestamp=t0)
        except Exception as e:
            self.error_management("READ_DATA", str(e))
        finally:
            self.stm32_gpio.value(0)
            self.led.off()

    def read_usb(self, line):
        try:
            data = [x.strip() for x in line.split(",")]
            cmd = data[0].upper()

            self.service_mode_status()

            if cmd == "PING":
                print(
                    "STATUS:ALIVE,{},{}".format(
                        self.status_data.get("version", ""),
                        self.status_data.get("build", ""),
                    )
                )

            elif cmd == "GET_OUTPUT_CHANNELS":
                channels = [name for _, name in self.output_channels]
                print("OK:OUTPUT_CHANNELS,{}".format(channels))

            elif cmd == "GET_PWM_CHANNELS":
                channels = [name for _, name in self.pwm_channels]
                print("OK:PWM_CHANNELS,{}".format(channels))

            elif cmd == "GET_INPUT_CHANNELS":
                channels = [name for _, name in self.input_channels]
                print("OK:INPUT_CHANNELS,{}".format(channels))

            elif cmd == "GET_STATUS":
                fw, hw = self.stm32.req_fw_hw_version()
                build_data = self.stm32.req_build_date()
                prod_date = self.stm32.req_prod_date()
                status = {
                    "controller_sw": fw,
                    "controller_hw": hw,
                    "controller_build_date": build_data,
                    "controller_prod_date": prod_date,
                    "communication_sw": self.status_data.get("version", ""),
                    "communication_build": self.status_data.get("build", ""),
                    "logger_id": self.status_data.get("logger_id", ""),
                    "sensor_id": self.status_data.get("sensor_id", ""),
                    "ip_address": self.ip_address,
                    "service_mode": self.service_mode,
                }
                print("OK:STATUS,{}".format(status))

            elif cmd == "RESET":
                if not self.service_mode:
                    print("ERR:RESET_SERVICE_MODE_DISABLED")
                    return

                print("OK:RESET")
                time.sleep(0.1)
                self.stm32.req_reset()

            elif cmd == "SYNC_TIME":
                dt = self.set_time()
                sync_time = "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z".format(
                    dt[0], dt[1], dt[2], dt[4], dt[5], dt[6]
                )
                date = self.stm32.req_rtc_read()
                print("OK:SYNC_TIME,{},{}".format(sync_time, date))

            elif cmd == "READ_RTC":
                date = self.stm32.req_rtc_read()
                print("OK:READ_RTC,{}".format(date))

            elif cmd == "READ_INPUTS":
                inputs = {}
                for channel_id, channel_name in self.input_channels:
                    value = self.stm32.req_get_input_states(channel_id)
                    inputs[channel_name] = value
                print("OK:READ_INPUTS,{}".format(inputs))

            elif cmd == "SET_OUTPUT":
                if len(data) < 3:
                    print("ERR:SET_OUTPUT_FORMAT")
                    return

                channel_name = data[1]
                value = int(data[2])

                channel_id = self._find_channel(self.output_channels, channel_name)
                if channel_id is None:
                    print("ERR:UNKNOWN_OUTPUT_CHANNEL,{}".format(channel_name))
                    return

                self.stm32.req_set_output(channel_id, value)
                print("OK:SET_OUTPUT,{},{}".format(channel_name, value))

            elif cmd == "SET_PWM":
                if len(data) < 3:
                    print("ERR:SET_PWM_FORMAT")
                    return

                channel_name = data[1]
                duty_cycle = int(data[2])

                channel_id = self._find_channel(self.pwm_channels, channel_name)
                if channel_id is None:
                    print("ERR:UNKNOWN_PWM_CHANNEL,{}".format(channel_name))
                    return

                self.stm32.req_set_pwm(channel_id, duty_cycle)
                print("OK:SET_PWM,{},{}".format(channel_name, duty_cycle))

            elif cmd == "RGB":
                if len(data) < 5:
                    print("ERR:RGB_FORMAT")
                    return

                r = int(data[1])
                g = int(data[2])
                b = int(data[3])
                brightness = int(data[4])

                self.stm32.req_set_rgb(r, g, b, brightness)
                print("OK:RGB,{},{},{},{}".format(r, g, b, brightness))

            elif cmd == "BUZZER":
                if len(data) < 3:
                    print("ERR:BUZZER_FORMAT")
                    return

                freq = int(data[1])
                volume = int(data[2])

                self.stm32.req_set_buzzer(freq, volume)
                print("OK:BUZZER,{},{}".format(freq, volume))

            elif cmd == "READ_INA":
                bus_voltage, shunt_voltage, current, power, id = (
                    self.stm32.req_ina_read()
                )
                print(
                    "OK:READ_INA,{},{},{},{},{}".format(
                        bus_voltage, shunt_voltage, current, power, id
                    )
                )

            elif cmd == "READ_SHT40":
                t, h = self.stm32.req_sht40()
                print("OK:READ_SHT40,{},{}".format(t, h))

            elif cmd == "READ_BME280":
                tb, hb, pb = self.stm32.req_bme280()
                print("OK:READ_BME280,{},{},{}".format(tb, hb, pb))

            elif cmd == "WRITE_FRAM":
                if not self.service_mode:
                    print("ERR:WRITE_FRAM_SERVICE_MODE_DISABLED")
                    return

                if len(data) < 4:
                    print("ERR:WRITE_FRAM_FORMAT")
                    return

                address = int(data[1], 0)
                bytes_data = bytes(int(x, 0) for x in data[2:])

                ok = self.stm32.req_fram_write(address, bytes_data)
                print("OK:WRITE_FRAM,{},{}".format(address, ok))

            elif cmd == "READ_FRAM":
                if not self.service_mode:
                    print("ERR:READ_FRAM_SERVICE_MODE_DISABLED")
                    return

                if len(data) < 3:
                    print("ERR:READ_FRAM_FORMAT")
                    return

                address = int(data[1], 0)
                length = int(data[2], 0)

                bytes_data = self.stm32.req_fram_read(address, length)
                if bytes_data is not None:
                    list_data = list(bytes_data)
                    print("OK:READ_FRAM,{},{}".format(address, list_data))
                else:
                    print("ERR:READ_FRAM,{},NO_DATA".format(address))

            elif cmd == "READ_FLAGS":
                flags = self.stm32.req_fram_read_flags()
                flags["WIFI_ENABLED"] = self.status_data.get("wifi_enabled", 0)
                print("OK:READ_FLAGS,{}".format(flags))

            elif cmd == "WRITE_FLAGS":
                if not self.service_mode:
                    print("ERR:WRITE_FLAGS_SERVICE_MODE_DISABLED")
                    return

                if len(data) < 3:
                    print("ERR:WRITE_FLAGS_FORMAT")
                    return

                flags_byte = int(data[1], 0)
                wifi_flag = int(data[2], 0)
                if flags_byte < 0 or flags_byte > 255:
                    print("ERR:WRITE_FLAGS_VALUE")
                    return

                if wifi_flag not in (0, 1):
                    print("ERR:WRITE_FLAGS_WIFI_VALUE")
                    return

                self.status_data["wifi_enabled"] = wifi_flag
                self.save_data()
                self.apply_config()

                ok = self.stm32.req_fram_write_flags(flags_byte)
                print("OK:WRITE_FLAGS,{},{}".format(flags_byte, ok))

            elif cmd == "READ_CONFIG":
                cfg_data = self.stm32.read_fram_config()
                cfg_data["wifi_ssid"] = self.status_data.get("wifi_ssid", "")
                wifi_password = nvs_sys.get_secret("wifi_password")
                cfg_data["mqtt_server"] = self.status_data.get("mqtt_server", "")
                cfg_data["ntp_server"] = self.status_data.get("ntp_server", "")
                mqtt_user = nvs_sys.get_secret("mqtt_username")
                mqtt_password = nvs_sys.get_secret("mqtt_password")
                if not self.service_mode:
                    cfg_data["wifi_password"] = (
                        self.mask_secret(wifi_password) if wifi_password else ""
                    )
                    cfg_data["mqtt_user"] = (
                        self.mask_secret(mqtt_user) if mqtt_user else ""
                    )
                    cfg_data["mqtt_password"] = (
                        self.mask_secret(mqtt_password) if mqtt_password else ""
                    )
                else:
                    cfg_data["wifi_password"] = wifi_password if wifi_password else ""
                    cfg_data["mqtt_user"] = mqtt_user if mqtt_user else ""
                    cfg_data["mqtt_password"] = mqtt_password if mqtt_password else ""
                print("OK:READ_CONFIG,{}".format(cfg_data))

            elif cmd == "WRITE_IDS":
                if not self.service_mode:
                    print("ERR:WRITE_IDS_SERVICE_MODE_DISABLED")
                    return

                if len(data) < 3:
                    print("ERR:WRITE_IDS_FORMAT")
                    return

                logger_id = data[1]
                sensor_id = data[2]

                ok1 = self.stm32.write_fram_config_field("logger_id", logger_id)
                ok2 = self.stm32.write_fram_config_field("sensor_id", sensor_id)
                ok = 1 if ok1 and ok2 else 0

                if ok:
                    self.apply_config()
                    self.restart_connections()

                print("OK:WRITE_IDS,{},{},{}".format(logger_id, sensor_id, ok))

            elif cmd == "WRITE_WIFI":
                if not self.service_mode:
                    print("ERR:WRITE_WIFI_SERVICE_MODE_DISABLED")
                    return

                if len(data) < 3:
                    print("ERR:WRITE_WIFI_FORMAT")
                    return

                ssid = data[1]
                password = data[2]

                ok = 0
                ok_pw = nvs_sys.set_secret("wifi_password", password)
                if ok_pw:
                    self.status_data["wifi_ssid"] = ssid
                    self.save_data()
                    self.apply_config()
                    self.restart_connections()
                    ok = 1

                password = self.mask_secret(password)
                print("OK:WRITE_WIFI,{},{},{}".format(ssid, password, ok))

            elif cmd == "WRITE_SERVERS":
                if not self.service_mode:
                    print("ERR:WRITE_SERVERS_SERVICE_MODE_DISABLED")
                    return

                if len(data) < 3:
                    print("ERR:WRITE_SERVERS_FORMAT")
                    return

                mqtt_server = data[1]
                ntp_server = data[2]

                ok = 0
                if mqtt_server and ntp_server:
                    self.status_data["mqtt_server"] = mqtt_server
                    self.status_data["ntp_server"] = ntp_server
                    self.save_data()
                    self.apply_config()
                    self.restart_connections()
                    ok = 1

                print("OK:WRITE_SERVERS,{},{},{}".format(mqtt_server, ntp_server, ok))

            elif cmd == "WRITE_MQTT_CREDENTIALS":
                if not self.service_mode:
                    print("ERR:WRITE_MQTT_CREDENTIALS_SERVICE_MODE_DISABLED")
                    return

                if len(data) < 3:
                    print("ERR:WRITE_MQTT_CREDENTIALS_FORMAT")
                    return

                mqtt_user = data[1]
                mqtt_password = data[2]

                ok = 0
                ok_user = nvs_sys.set_secret("mqtt_username", mqtt_user)
                ok_pw = nvs_sys.set_secret("mqtt_password", mqtt_password)
                if ok_user and ok_pw:
                    self.apply_config()
                    self.restart_connections()
                    ok = 1

                mqtt_password_masked = self.mask_secret(mqtt_password)
                print(
                    "OK:WRITE_MQTT_CREDENTIALS,{},{},{}".format(
                        mqtt_user, mqtt_password_masked, ok
                    )
                )

            elif cmd == "SERVICE_MODE_ENABLE":
                if self.service_mode:
                    print("ERR:SERVICE_MODE_ENABLE_ALREADY_ENABLED")
                    return

                if len(data) < 2:
                    print("ERR:SERVICE_MODE_ENABLE_FORMAT")
                    return

                password = data[1]
                ymd = self._get_service_ymd()

                if auth.verify_password_for_date(password, ymd):
                    self.service_mode_enable(300)
                    print("OK:SERVICE_MODE_ENABLE,300")
                else:
                    print("ERR:SERVICE_MODE_ENABLE_WRONG_PASSWORD")

            elif cmd == "SERVICE_MODE_DISABLE":
                if not self.service_mode:
                    print("ERR:SERVICE_MODE_DISABLE_NOT_ENABLED")
                    return

                self.service_mode_disable()
                print("OK:SERVICE_MODE_DISABLE")

            else:
                print("ERR:UNKNOWN_CMD,{}".format(cmd))

        except Exception as e:
            print("ERR:READ_USB,{}".format(e))
            self.error_management("READ_USB", str(e))
