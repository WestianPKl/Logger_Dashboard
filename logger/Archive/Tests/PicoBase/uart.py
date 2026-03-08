import time
from machine import Pin, UART


class UARTDevice:
    def __init__(
        self,
        uart_id=1,
        tx_pin=None,
        rx_pin=None,
        baudrate=9600,
        bits=8,
        parity=None,
        stop=1,
    ):
        self.uart = UART(
            uart_id,
            baudrate=baudrate,
            bits=bits,
            parity=parity,
            stop=stop,
            tx=Pin(tx_pin),
            rx=Pin(rx_pin),
        )

    def write(self, data):
        if isinstance(data, str):
            data = data.encode("utf-8")
        self.uart.write(data)

    def read(self, nbytes):
        return self.uart.read(nbytes)

    def any(self):
        return self.uart.any()

    def deinit(self):
        self.uart.deinit()
        self.uart = None


"""Example usage:

Raspberry Pi Pico UART Pins:
UART0: GPIO0 (TX), GPIO1 (RX)
UART1: GPIO4 (TX), GPIO5 (RX)

def main():
    uart_device = UARTDevice(uart_id=0, tx_pin=16, rx_pin=17, baudrate=115200)

    i = 0
    while True:
        try:
            test_message = f"Hello, UART! Count: {i}"
            print(f"Sending: {test_message}")
            uart_device.write(test_message)

            time.sleep(1)

            if uart_device.any():
                received_data = uart_device.read(64)
                if received_data:
                    print(f"Received: {received_data.decode('utf-8')}")
            i += 1
        except Exception as e:
            print(f"An error occurred: {e}")
            break


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Program interrupted by user.")
        
"""
