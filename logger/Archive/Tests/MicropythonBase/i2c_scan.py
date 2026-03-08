from machine import Pin, I2C
import time

i2c = I2C(0, scl=Pin(5), sda=Pin(4), freq=100_000)

while True:
    devices = i2c.scan()
    print("I2C devices:", [hex(d) for d in devices])
    time.sleep(3)
