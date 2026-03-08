from machine import Pin, SPI
import time

cs = Pin(17, Pin.OUT, value=1)

spi = SPI(
    0,
    baudrate=1_000_000,
    polarity=0,
    phase=0,
    bits=8,
    firstbit=SPI.MSB,
    sck=Pin(18),
    mosi=Pin(19),
    miso=Pin(20),
)

tx = bytes([0x9F])
rx = bytearray(1)

while True:
    cs.value(0)
    spi.write_readinto(tx, rx)
    cs.value(1)

    print("RX:", hex(rx[0]))
    time.sleep(1)
