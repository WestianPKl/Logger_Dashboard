import machine, time, ujson, os
from machine import RTC, Pin
from mqtt_simple import MQTTClient
from config import Config

cfg = Config()


class Program:
    def __init__(self):
        self.client = None
        self.status_data = {}
        self.load_data()
        self.device_id = str(self.status_data.get("logger_id", ""))
        self.topic_status = f"devices/{self.device_id}/status".encode()
        self.ip_address = None
        self.stm32_gpio = Pin(cfg.get_stm32_gpio(), Pin.OUT)
        self.led = Pin(cfg.get_led_gpio(), Pin.OUT)
        self.pending_read = False
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
        if time.ticks_diff(now, self.last_irq_ms) < 50:
            return
        self.last_irq_ms = now
        self.pending_read = True

    def get_client(self):
        return self.client

    def set_client(self, client):
        self.client = client

    def set_ip_address(self, ip_address):
        self.ip_address = ip_address

    def mqtt_initialization(self):
        self.client = MQTTClient(
            client_id=str(self.device_id).encode(),
            server=cfg.get_mqtt_server(),
            port=cfg.get_mqtt_port(),
            user=cfg.get_mqtt_user().encode(),
            password=cfg.get_mqtt_password().encode(),
            keepalive=cfg.get_mqtt_keepalive(),
        )

        self.client.set_callback(self.message_mqtt)
        self.client.connect()
        self.client.subscribe(f"devices/{self.device_id}/cmd".encode())
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
            if cmd == "PING":
                self.send_status(
                    "ALIVE",
                    "STATUS",
                    {
                        "communication_sw": self.status_data.get("version", ""),
                        "communication_build": self.status_data.get("build", ""),
                        "logger_id": self.status_data.get("logger_id", ""),
                        "sensor_id": self.status_data.get("sensor_id", ""),
                        "ip_address": self.ip_address,
                    },
                )
            elif cmd == "RESET":
                machine.reset()
            elif cmd == "SYNC_TIME":
                dt = self.set_time()
                sync_time = "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z".format(
                    dt[0],
                    dt[1],
                    dt[2],
                    dt[4],
                    dt[5],
                    dt[6],
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
                    params.get("r", 0), params.get("g", 0), params.get("b", 0)
                )
                self.send_status(
                    "RGB_SET",
                    "STATUS",
                    {
                        "r": params.get("r", 0),
                        "g": params.get("g", 0),
                        "b": params.get("b", 0),
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
                "pending_errors": [],
            }

    def save_data(self, filename="status.json"):
        tmp = filename + ".tmp"
        with open(tmp, "w") as f:
            ujson.dump(self.status_data, f)
        os.rename(tmp, filename)

    def error_dict(self, source, msg):
        return {"timestamp": time.time(), "source": source, "msg": str(msg)}

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
                "ERRORS_FLUSH", "ERROR", {"items": self.status_data["pending_errors"]}
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
                        "sht40": {"temperature": t, "humidity": h},
                        "bme280": {"temperature": tb, "humidity": hb, "pressure": pb},
                        "rtc": date,
                        "bme280_error": int(tb == 0 and hb == 0 and pb == 0),
                        "sht40_error": int(t == 0 and h == 0),
                        "stm32_error": 0,
                    }
                )
            else:
                data.update({"stm32_error": 1})

            if self.client is not None:
                self.send_status("DATA", "DATA", data, timestamp=t0)
        except Exception as e:
            self.error_management("READ_DATA", str(e))
        finally:
            self.stm32_gpio.value(0)
            self.led.off()
