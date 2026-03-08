from machine import Pin, UART
import time

uart = UART(0, baudrate=115200, tx=Pin(0), rx=Pin(1))

uart.write(b"UART ready\r\n")

while True:
    if uart.any():
        b = uart.read(1)
        if b:
            uart.write(b)  # echo
    time.sleep_ms(10)
