from machine import Pin, SPI
import utime
from can_mcp2515 import CAN_MCP2515

can_on = Pin(21, Pin.OUT)
can_int = Pin(20, Pin.IN, Pin.PULL_UP)
MODE_NORMAL = 0x00

# ID 0x010 Serial number and counter
# ID 0x011 Firmware version and hardware version
# ID 0x012 Build date
# ID 0x013 Production date
# ID 0x020 ADC values
# ID 0x030 SHT40 temperature and humidity
# ID 0x031 BME280 temperature, humidity and pressure
# ID 0x060 Datetime (year, month, day, weekday, hour, minute, second)


def decode_data(rid, rdata):
    if rdata[0] == 0x7F:
        return "Error: {}".format(rdata[1])
    if rid == 0x010:
        serial_number = rdata[0:4]
        counter = rdata[4]
        return "Serial: {}, Counter: {}".format(
            int.from_bytes(serial_number, "big"), counter
        )
    elif rid == 0x011:
        firmware_version = rdata[0:3]
        hardware_version = rdata[3:5]
        return "Firmware Version: {}.{}.{} | Hardware Version: {}.{}".format(
            *firmware_version, *hardware_version
        )
    elif rid == 0x012:
        build_date = rdata.decode("ascii").rstrip("\x00")
        return "Build Date: {}".format(build_date)
    elif rid == 0x013:
        production_date = rdata.decode("ascii").rstrip("\x00")
        return "Production Date: {}".format(production_date)
    elif rid == 0x020:
        adc_ch1 = int.from_bytes(rdata[0:2], "big")
        adc_ch2 = int.from_bytes(rdata[2:4], "big")
        return "ADC Values: Ch1={}, Ch2={}".format(adc_ch1, adc_ch2)
    elif rid == 0x030:
        temperature = int.from_bytes(rdata[0:2], "big") / 100
        humidity = int.from_bytes(rdata[2:4], "big") / 100
        return "SHT40 - Temp: {:.2f}°C, Humidity: {:.2f}%".format(temperature, humidity)
    elif rid == 0x031:
        temperature = int.from_bytes(rdata[0:2], "big") / 100
        humidity = int.from_bytes(rdata[2:4], "big") / 100
        pressure = int.from_bytes(rdata[4:8], "big") / 100
        return (
            "BME280 - Temp: {:.2f}°C, Humidity: {:.2f}%, Pressure: {:.2f} hPa".format(
                temperature, humidity, pressure
            )
        )
    elif rid == 0x060:
        year = 2000 + rdata[0]
        month = rdata[1]
        day = rdata[2]
        weekday = rdata[3]
        hour = rdata[4]
        minute = rdata[5]
        second = rdata[6]
        return (
            "Datetime: {:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d} (Weekday: {})".format(
                year, month, day, hour, minute, second, weekday
            )
        )
    else:
        return "Unknown ID"


def main():
    print("=== SNIFFER: moduł, który był nadajnikiem ===")
    can_on.value(1)
    utime.sleep(0.5)

    spi = SPI(
        0,
        baudrate=1_000_000,
        polarity=0,
        phase=0,
        sck=Pin(18),
        mosi=Pin(19),
        miso=Pin(16),
    )
    cs_pin = Pin(17, Pin.OUT, value=1)

    can = CAN_MCP2515(spi, cs_pin, can_int)
    can.set_mode(MODE_NORMAL)

    print("Czekam na jakiekolwiek ramki...")

    while True:
        msg = can.recv_std()
        if msg is not None:
            rid, rdata = msg
            print("RX: ID=0x{:03X}, data={}".format(rid, [hex(b) for b in rdata]))
            try:
                print("    decoded =", decode_data(rid, rdata))
            except:
                pass
        utime.sleep_ms(1)


if __name__ == "__main__":
    main()
