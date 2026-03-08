from machine import Pin, SPI
import time

EPD_WIDTH = 168
EPD_HEIGHT = 168

# Kolory (numery odpowiadają 2-bitowemu kodowaniu w buforze)
EPD_BLACK  = 0b00
EPD_WHITE  = 0b01
EPD_YELLOW = 0b10
EPD_RED    = 0b11

class EPD:
    def __init__(self, spi, cs, dc, rst, busy):
        self.reset_pin = rst
        self.dc_pin = dc
        self.busy_pin = busy
        self.cs_pin = cs
        self.spi = spi
        self.width = EPD_WIDTH
        self.height = EPD_HEIGHT

        self.reset_pin.init(Pin.OUT, value=1)
        self.dc_pin.init(Pin.OUT, value=0)
        self.cs_pin.init(Pin.OUT, value=1)
        self.busy_pin.init(Pin.IN)

    def _digital_write(self, pin, value):
        pin.value(value)

    def _digital_read(self, pin):
        return pin.value()

    def _delay_ms(self, ms):
        time.sleep_ms(ms)

    def send_command(self, command):
        self._digital_write(self.dc_pin, 0)
        self._digital_write(self.cs_pin, 0)
        self.spi.write(bytearray([command]))
        self._digital_write(self.cs_pin, 1)

    def send_data(self, data):
        self._digital_write(self.dc_pin, 1)
        self._digital_write(self.cs_pin, 0)
        if isinstance(data, int):
            self.spi.write(bytearray([data]))
        else:
            self.spi.write(data)
        self._digital_write(self.cs_pin, 1)

    def reset(self):
        self._digital_write(self.reset_pin, 1)
        self._delay_ms(200)
        self._digital_write(self.reset_pin, 0)
        self._delay_ms(2)
        self._digital_write(self.reset_pin, 1)
        self._delay_ms(200)

    def wait_until_idle(self):
        while self._digital_read(self.busy_pin) == 0:
            self._delay_ms(5)

    def wait_until_ready(self):
        while self._digital_read(self.busy_pin) == 1:
            self._delay_ms(5)

    def TurnOnDisplay(self):
        self.send_command(0x12) # DISPLAY_REFRESH
        self.send_data(0x01)
        self.wait_until_idle()

        self.send_command(0x02) # POWER_OFF
        self.send_data(0x00)
        self.wait_until_idle()

    def init(self):
        self.reset()
        # Sekwencja inicjalizacji z dokumentacji producenta:
        self.send_command(0x66)
        self.send_data(0x49)
        self.send_data(0x55)
        self.send_data(0x13)
        self.send_data(0x5D)

        self.send_command(0x66)
        self.send_data(0x49)
        self.send_data(0x55)

        self.send_command(0xB0)
        self.send_data(0x03)

        self.send_command(0x00)
        self.send_data(0x4F)
        self.send_data(0x6B)

        self.send_command(0x03)
        self.send_data(0x00)

        self.send_command(0xF0)
        self.send_data(0xF6)
        self.send_data(0x0D)
        self.send_data(0x00)
        self.send_data(0x00)
        self.send_data(0x00)

        self.send_command(0x06)
        self.send_data(0xCF)
        self.send_data(0xDF)
        self.send_data(0x0F)

        self.send_command(0x41)
        self.send_data(0x00)

        self.send_command(0x50)
        self.send_data(0x30)

        self.send_command(0x60)
        self.send_data(0x0C)
        self.send_data(0x05)

        self.send_command(0x61)
        self.send_data(0xA8)
        self.send_data(0x00)
        self.send_data(0xA8)

        self.send_command(0x84)
        self.send_data(0x01)
        return 0

    def display(self, buf):
        # buf - 2 bity na piksel, 4 piksele w bajcie
        if self.width % 4 == 0:
            Width = self.width // 4
        else:
            Width = self.width // 4 + 1
        Height = self.height

        self.send_command(0x68)
        self.send_data(0x01)

        self.send_command(0x04)
        self.wait_until_idle()

        self.send_command(0x10)
        idx = 0
        for j in range(Height):
            for i in range(Width):
                self.send_data(buf[idx])
                idx += 1

        self.send_command(0x68)
        self.send_data(0x00)
        self.TurnOnDisplay()

    def Clear(self, color=0x55):
        # color: bajt - 4 piksele (2 bity każdy, np. 0x55 to białe)
        if self.width % 4 == 0:
            Width = self.width // 4
        else:
            Width = self.width // 4 + 1
        Height = self.height

        self.send_command(0x68)
        self.send_data(0x01)

        self.send_command(0x04)
        self.wait_until_idle()

        self.send_command(0x10)
        for j in range(Height):
            for i in range(Width):
                self.send_data(color)

        self.send_command(0x68)
        self.send_data(0x00)
        self.TurnOnDisplay()

    def sleep(self):
        self.send_command(0x02) # POWER_OFF
        self.send_data(0x00)
        self.send_command(0x07) # DEEP_SLEEP
        self.send_data(0xA5)
        self._delay_ms(2000)