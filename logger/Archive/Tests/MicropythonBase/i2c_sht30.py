from machine import Pin, I2C
import time

SHT30_ADDR = 0x44

i2c = I2C(0, scl=Pin(5), sda=Pin(4), freq=100_000)


def sht30_read():
    # komenda pomiaru: single shot, high repeatability, clock stretching
    i2c.writeto(SHT30_ADDR, bytes([0x2C, 0x06]))
    time.sleep_ms(15)

    data = i2c.readfrom(SHT30_ADDR, 6)
    t_raw = (data[0] << 8) | data[1]
    rh_raw = (data[3] << 8) | data[4]
    # data[2], data[5] = CRC (pomijamy)

    t_c = -45.0 + 175.0 * (t_raw / 65535.0)
    rh = 100.0 * (rh_raw / 65535.0)
    return t_c, rh


while True:
    try:
        t, h = sht30_read()
        print("T=%.2f C  RH=%.1f %%" % (t, h))
    except OSError as e:
        print("SHT30 error:", e)
    time.sleep(1)
