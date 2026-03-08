from machine import UART, Pin
import utime
from modbus_max3485 import ModbusRTUSlave

SLAVE_ADDR = 1


def main():
    print("=== MODBUS SLAVE (Pico #2) ===")

    modbus_on = Pin(11, Pin.OUT, value=1)
    utime.sleep(0.5)

    uart = UART(
        0,
        baudrate=9600,
        bits=8,
        parity=None,
        stop=1,
        tx=Pin(0),
        rx=Pin(1),
    )

    de_re = Pin(13, Pin.OUT)

    slave = ModbusRTUSlave(uart, de_re, slave_addr=SLAVE_ADDR, num_regs=16)

    slave.regs[1] = 0x1111
    slave.regs[2] = 0x2222
    slave.regs[3] = 0x3333

    last_print = utime.ticks_ms()
    while True:
        slave.poll()
        if utime.ticks_diff(utime.ticks_ms(), last_print) > 2000:
            print("Rejestry SLAVE[0..3]:", slave.regs[0:4])
            last_print = utime.ticks_ms()


if __name__ == "__main__":
    main()
