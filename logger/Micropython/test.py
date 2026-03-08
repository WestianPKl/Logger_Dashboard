from machine import Pin, I2C
import time
import network
import ntptime
from rtc_clock import RTC_Clock
from i2c_lcd import I2cLcd
from sht30 import SHT30
from sht40 import SHT40
from bme280_i2c import BME280


sda_pin = 0
scl_pin = 1
total_rows = 2
total_columns = 16
sensor_type = 30  # 30 for SHT30, 40 for SHT40, 0 for BME280
pressure_measurement = 0


def network_connection():
    ssid = ""
    password = ""

    station = network.WLAN(network.STA_IF)
    station.active(True)
    station.connect(ssid, password)

    timeout = 0
    while station.isconnected() == False and timeout < 20:
        time.sleep(1)
        timeout += 1

    if station.isconnected():
        try:
            ntptime.settime()
        except Exception as e:
            print("NTP time sync error:", e)
    else:
        print("WiFi connection failed!")


def set_time(clock):
    try:
        aT = time.localtime(time.time() + 2 * 60 * 60)
        year = aT[0]
        month = aT[1]
        day = aT[2]
        hour = aT[3]
        minute = aT[4]
        second = aT[5]
        clock.set_time((year, month, day, 0, hour, minute, second))
    except Exception as e:
        print("Set time error:", e)


## SMD RGD LED control ##
def map_color(color):
    return int(color * 65535 / 255)


def set_color(leds, red, green, blue):
    try:
        leds[0].duty_u16(map_color(red))
        leds[1].duty_u16(map_color(green))
        leds[2].duty_u16(map_color(blue))
    except Exception as e:
        print("Cannot set LED:", e)


##################################


def button1_handler():
    print("Button 1 pressed")


def button2_handler():
    print("Button 2 pressed")


def main():
    i2c = I2C(0, scl=Pin(scl_pin), sda=Pin(sda_pin), freq=100000)
    lcd = I2cLcd(i2c, 0x27, total_rows, total_columns)
    lcd.move_to(0, 0)
    lcd.putstr("Logger ready")
    network_connection()
    clock = RTC_Clock(i2c)
    set_time(clock)
    print(clock.read_time())
    if sensor_type == 30:
        sensor = SHT30(i2c)
    elif sensor_type == 40:
        sensor = SHT40(i2c)
    elif sensor_type == 0:
        sensor = BME280(i2c)

    print(sensor.temperature())
    print(sensor.relative_humidity())
    if pressure_measurement == 1 and sensor_type == 0:
        print(sensor.pressure())

    switch1 = Pin(17, Pin.IN, Pin.PULL_UP)
    switch2 = Pin(16, Pin.IN, Pin.PULL_UP)
    switch1.irq(
        trigger=Pin.IRQ_FALLING,
        handler=button1_handler,
    )
    switch2.irq(
        trigger=Pin.IRQ_FALLING,
        handler=button2_handler,
    )
    led_pin = Pin(15, Pin.OUT)

    while True:
        led_pin.value(1)
        time.sleep(1)
        led_pin.value(0)
        time.sleep(1)


if __name__ == "__main__":
    main()
