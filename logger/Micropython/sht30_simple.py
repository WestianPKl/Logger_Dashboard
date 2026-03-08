from machine import Pin, I2C
import time

sensor_addr = 0x44
i2c = I2C(0, scl=Pin(1), sda=Pin(0), freq=100000)

i2c.writeto(sensor_addr, b"\x2c\x06")
time.sleep_ms(100)
raw = i2c.readfrom(sensor_addr, 6)
temperature_raw = (raw[0] << 8) + raw[1]
humidity_raw = (raw[3] << 8) + raw[4]

temp = -45 + (175 * (temperature_raw / 65535))
hum = 100 * (humidity_raw / 65535)
print("Temperature:", temp, "C")
print("Humidity:", hum, "%")
