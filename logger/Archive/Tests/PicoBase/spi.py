import time
from machine import Pin, SPI


# MOSI -> TX
# MISO -> RX


class SPIDevice:
    def __init__(
        self,
        channel,
        sck_pin,
        mosi_pin,
        miso_pin,
        cs_pin,
        baudrate=1000000,
        polarity=0,
        phase=0,
    ):
        self.spi = SPI(
            channel,
            baudrate=baudrate,
            polarity=polarity,
            phase=phase,
            sck=Pin(sck_pin),
            mosi=Pin(mosi_pin),
            miso=Pin(miso_pin),
        )

        self.cs = Pin(cs_pin, Pin.OUT)
        self._cs_high()

    def _cs_low(self):
        self.cs.value(0)

    def _cs_high(self):
        self.cs.value(1)

    def write(self, data):
        self._cs_low()
        if isinstance(data, str):
            data = data.encode("utf-8")
        self.spi.write(data)
        self._cs_high()

    def read(self, nbytes):
        self._cs_low()
        buf = self.spi.read(nbytes)
        self._cs_high()
        return buf

    def write_read(self, write_data, nbytes):
        self._cs_low()
        if isinstance(write_data, str):
            write_data = write_data.encode("utf-8")
        buf = bytearray(nbytes)
        result = self.spi.write_readinto(write_data, buf)
        self._cs_high()
        return bytes(buf)

    def deinit(self):
        self._cs_high()
        self.spi.deinit()
        self.spi = None


"""Example usage:

Raspberry Pi Pico SPI Pins:
SPI0: GPIO16 (MISO), GPIO19 (MOSI), GPIO18 (SCK), GPIO17 (CS)
SPI1: GPIO12 (MISO), GPIO11 (MOSI), GPIO10 (SCK), GPIO13 (CS)

def main():

    spi_device = SPIDevice(channel=0, sck_pin=18, mosi_pin=19, miso_pin=16, cs_pin=17, baudrate=1000000)

    try:
        # Write data
        spi_device.write("Hello SPI")

        # Read data
        data = spi_device.read(10)
        print("Received:", data)

        # Write and read data
        response = spi_device.write_read("Ping", 4)
        print("Response:", response)

    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        spi_device.deinit()

if __name__ == "__main__":
    main()

"""
