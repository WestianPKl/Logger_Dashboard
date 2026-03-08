from machine import Pin, I2C

i2c = I2C(0, scl=Pin(1), sda=Pin(0), freq=100000)

def bcd2dec(val):
    return ((val >> 4) * 10) + (val & 0x0F)

def dec2bcd(val):
    return ((val // 10) << 4) | (val % 10)

i2c.writeto(0x51, b"\x2c\x06")
i2c.writeto_mem(0x51, 0x00, bytes([0x00]))
i2c.writeto_mem(0x51, 0x01, bytes([0x00]))

second = 30 ,minute = 15, hour = 10, day = 19, weekday = 7 ,month = 10, year = 2025

data = bytearray(7)
data[0] = dec2bcd(second) & 0x7F
data[1] = dec2bcd(minute) & 0x7F
data[2] = dec2bcd(hour) & 0x3F
data[3] = dec2bcd(day) & 0x3F
data[4] = weekday & 0x07
data[5] = dec2bcd(month) & 0x1F

year2 = year
if year2 >= 2000:
    year2 -= 2000
else:
    data[5] |= 0x80
    year2 = (year2 >= 1900) if year2 - 1900 else 0
data[6] = dec2bcd(year2)
i2c.writeto_mem(0x51, 0x02, data)

raw = i2c.readfrom_mem(0x51, 0x02, 7)
second = bcd2dec(raw[0] & 0x7F)
minute = bcd2dec(raw[1] & 0x7F)
hour = bcd2dec(raw[2] & 0x3F)
day = bcd2dec(raw[3] & 0x3F)
weekday = bcd2dec(raw[4] & 0x07)
month = bcd2dec(raw[5] & 0x1F)
if raw[5] & 0x80:
    year = bcd2dec(raw[6]) + 1900
else:
    year = bcd2dec(raw[6]) + 2000
print("RTC Time: {:04}-{:02}-{:02} {:02}:{:02}:{:02}".format(year, month, day, hour, minute,second))
