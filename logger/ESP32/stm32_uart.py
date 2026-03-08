import time
from machine import UART
from config import Config

cfg = Config()


class STM32UART:
    def __init__(self, uart_device: UART):
        self.uart = uart_device

    def crc8_atm(self, data: bytes) -> int:
        crc = 0x00
        for b in data:
            crc ^= b
            for _ in range(8):
                if crc & 0x80:
                    crc = ((crc << 1) ^ 0x07) & 0xFF
                else:
                    crc = (crc << 1) & 0xFF
        return crc

    def uart_message_application(
        self, cmd: int, param_addr: int = 0, payload: bytes = b""
    ) -> bytes:
        frame = bytearray(cfg.get_frame_len_app())
        frame[0] = cfg.get_dev_addr()
        frame[1] = 0x00
        frame[2] = cmd & 0xFF
        frame[3] = param_addr & 0xFF

        for i, b in enumerate(payload):
            if 4 + i >= cfg.get_frame_len_app() - 1:
                break
            frame[4 + i] = b

        frame[cfg.get_frame_len_app() - 1] = self.crc8_atm(
            frame[: cfg.get_frame_len_app() - 1]
        )
        return bytes(frame)

    def read_exact(self, n: int, timeout_ms: int = 500) -> bytes | None:
        buf = bytearray()
        t0 = time.ticks_ms()
        while len(buf) < n and time.ticks_diff(time.ticks_ms(), t0) < timeout_ms:
            chunk = self.uart.read(n - len(buf))
            if chunk:
                buf.extend(chunk)
            else:
                time.sleep_ms(2)
        if len(buf) == n:
            return bytes(buf)
        return None

    def _flush_rx(self):
        while True:
            d = self.uart.read()
            if not d:
                break
            time.sleep_ms(1)

    def uart_loop_application(self, frame: bytes) -> bytes | None:
        self._flush_rx()
        self.uart.write(frame)
        t0 = time.ticks_ms()
        while time.ticks_diff(time.ticks_ms(), t0) < 1500:
            b = self.uart.read(1)
            if not b:
                time.sleep_ms(2)
                continue
            if b[0] != cfg.get_dev_addr():
                continue

            rest = self.read_exact(cfg.get_frame_len_app() - 1, 1200)
            if rest is None:
                return None

            data = bytes([cfg.get_dev_addr()]) + rest
            if (
                self.crc8_atm(data[: cfg.get_frame_len_app() - 1])
                != data[cfg.get_frame_len_app() - 1]
            ):
                continue
            return data
        return None

    def parse_resp(self, resp: bytes):
        if not resp or len(resp) != cfg.get_frame_len_app():
            return None
        addr = resp[0]
        status = resp[1]
        cmd = resp[2]
        param = resp[3]
        payload = resp[4 : cfg.get_frame_len_app() - 1]
        crc = resp[cfg.get_frame_len_app() - 1]
        return addr, status, cmd, param, payload, crc

    def i16_from_be(self, payload: bytes, offset=0) -> int:
        v = (payload[offset] << 8) | payload[offset + 1]
        if v & 0x8000:
            v -= 0x10000
        return v

    def u16_from_be(self, payload, offset=0) -> int:
        return (payload[offset] << 8) | payload[offset + 1]

    def i32_from_be(self, payload, offset=0) -> int:
        v = (
            (payload[offset] << 24)
            | (payload[offset + 1] << 16)
            | (payload[offset + 2] << 8)
            | payload[offset + 3]
        )
        if v & 0x80000000:
            v -= 0x100000000
        return v

    def u32_from_be(self, payload, offset=0) -> int:
        return (
            (payload[offset] << 24)
            | (payload[offset + 1] << 16)
            | (payload[offset + 2] << 8)
            | payload[offset + 3]
        )

    def adc_to_voltage(self, adc_value, vref=3.3):
        return (adc_value / 4095.0) * vref

    def vadc_to_vin(self, vadc, r_top=200_000.0, r_bottom=68_000.0):
        return vadc * (r_top + r_bottom) / r_bottom

    def _send_cmd(self, cmd, param=0, payload=b"", check_status=True):
        resp = self.uart_loop_application(
            self.uart_message_application(cmd, param, payload)
        )
        parsed = self.parse_resp(resp)
        if not parsed:
            return None
        _, status, _, _, data, _ = parsed
        if check_status and status != cfg.get_status_ok():
            return None
        return data

    def req_ping(self):
        return self._send_cmd(0x00, 0x00, check_status=False) is not None

    def req_serial(self):
        p = self._send_cmd(0x01, 0x00, check_status=False)
        return self.u32_from_be(p, 0) if p else 0

    def req_fw_hw_version(self):
        p = self._send_cmd(0x01, 0x01, check_status=False)
        if not p:
            return ["", ""]
        return [f"{p[0]}.{p[1]}.{p[2]}", f"{p[3]}.{p[4]}"]

    def req_build_date(self):
        p = self._send_cmd(0x01, 0x02, check_status=False)
        if not p:
            return ""
        return bytes(p[:10]).split(b"\x00")[0].decode("ascii")

    def req_prod_date(self):
        p = self._send_cmd(0x01, 0x03, check_status=False)
        if not p:
            return ""
        return bytes(p[:10]).split(b"\x00")[0].decode("ascii")

    def req_adc(self):
        p = self._send_cmd(0x02, 0x00, check_status=False)
        if not p:
            return [0, 0]
        return [self.u16_from_be(p, 0), self.u16_from_be(p, 2)]

    def req_sht40(self):
        p = self._send_cmd(0x03, 0x00)
        if not p:
            return [0.0, 0.0]
        return [self.i16_from_be(p, 0) / 100.0, self.u16_from_be(p, 2) / 100.0]

    def req_bme280(self):
        p = self._send_cmd(0x03, 0x01)
        if not p:
            return [0.0, 0.0, 0.0]
        return [
            self.i32_from_be(p, 0) / 100.0,
            self.u32_from_be(p, 4) / 1024.0,
            self.u32_from_be(p, 8) / 25600.0,
        ]

    def req_get_input_states(self, channel: int):
        p = self._send_cmd(0x02, channel)
        return p[0] if p else 0

    def req_output_states(self):
        p = self._send_cmd(0x04, 0x00)
        if not p:
            return [0, 0, 0, 0, 0]
        return [p[0], p[1], p[2], p[3], p[4]]

    def req_set_output(self, channel: int, on: int):
        p = self._send_cmd(0x04, channel, bytes([on & 0xFF]))
        return p[0] if p else 0

    def req_set_pwm(self, channel: int, duty: int):
        p = self._send_cmd(0x05, channel, bytes([duty & 0xFF]))
        return p[0] if p else 0

    def req_set_rgb(self, r: int, g: int, b: int):
        p = self._send_cmd(0x05, 0x05, bytes([r & 0xFF, g & 0xFF, b & 0xFF]))
        if not p:
            return [0, 0, 0]
        return [p[0], p[1], p[2]]

    def req_set_buzzer(self, freq: int, volume: int):
        pl = bytes([(freq >> 8) & 0xFF, freq & 0xFF, volume & 0xFF])
        p = self._send_cmd(0x05, 0x06, pl)
        return self.u16_from_be(p, 0) if p else 0

    def req_rtc_read(self):
        p = self._send_cmd(0x06, 0x00)
        if not p:
            return ""
        return "20{:02d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z".format(
            p[0], p[1], p[2], p[4], p[5], p[6]
        )

    def req_ina_read(self):
        p = self._send_cmd(0x07, 0x00)
        if not p:
            return [0.0, 0.0, 0.0, 0.0, 0.0]
        bus_voltage = self.u32_from_be(p, 0) / 1_000_000.0
        shunt_voltage = self.i32_from_be(p, 4) / 1_000_000.0
        current = self.i32_from_be(p, 8) / 1_000_000.0
        power = self.u32_from_be(p, 12) / 1_000_000.0
        id = self.u16_from_be(p, 16)
        return [bus_voltage, shunt_voltage, current, power, id]

    def req_rtc_write(
        self, yy: int, mo: int, dd: int, wd: int, hh: int, mi: int, ss: int
    ):
        pl = bytes(
            [
                yy & 0xFF,
                mo & 0xFF,
                dd & 0xFF,
                wd & 0xFF,
                hh & 0xFF,
                mi & 0xFF,
                ss & 0xFF,
            ]
        )
        p = self._send_cmd(0x06, 0x01, pl)
        if not p:
            return ""
        return "20{:02d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}".format(
            p[0], p[1], p[2], p[4], p[5], p[6]
        )

    def req_rtc_wakeup(self, seconds: int):
        pl = bytes([(seconds >> 8) & 0xFF, seconds & 0xFF])
        p = self._send_cmd(0x06, 0x02, pl)
        return p[0] if p else 0

    def req_alarm_set(self, hh: int, mi: int, ss: int, daily: int = 1):
        pl = bytes([hh & 0xFF, mi & 0xFF, ss & 0xFF, daily & 0xFF])
        p = self._send_cmd(0x06, 0x03, pl)
        return p[0] if p else 0

    def req_alarm_off(self):
        p = self._send_cmd(0x06, 0x04)
        return p[0] if p else 0

    def req_timestamp(self):
        p = self._send_cmd(0x06, 0x05)
        if not p:
            return ""
        return "20{:02d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}".format(
            p[0], p[1], p[2], p[4], p[5], p[6]
        )
