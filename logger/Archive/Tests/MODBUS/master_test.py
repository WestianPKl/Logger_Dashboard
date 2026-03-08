from machine import UART, Pin
import utime
from modbus_max3485 import ModbusRTUMaster

SLAVE_ADDR = 1


def main():
    print("=== MODBUS MASTER (Pico #1) ===")
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

    master = ModbusRTUMaster(uart, de_re, timeout_ms=300)

    ok = master.write_single_register(SLAVE_ADDR, reg_addr=0, value=1234)
    print("Zapis rejestru 0:", "OK" if ok else "FAIL")

    while True:
        regs = master.read_holding_registers(SLAVE_ADDR, start_addr=0, count=4)
        if regs is None:
            print("Brak odpowiedzi / błąd odczytu")
        else:
            print("Rejestry[0..3]:", regs)
        utime.sleep(1)


if __name__ == "__main__":
    main()
