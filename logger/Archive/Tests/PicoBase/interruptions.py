import time
from machine import Pin


class InterruptPin:
    def __init__(self, pin_number, trigger=Pin.IRQ_FALLING, pull=Pin.PULL_UP):
        self.pin = Pin(pin_number, Pin.IN, pull)
        self.callback = None
        self.pin.irq(trigger=trigger, handler=self._handle_interrupt)

    def _handle_interrupt(self, pin):
        if self.callback:
            self.callback(pin)

    def set_callback(self, callback):
        self.callback = callback

    def enable(self):
        self.pin.irq(handler=self._handle_interrupt)

    def disable(self):
        self.pin.irq(handler=None)

    def read(self):
        return self.pin.value()

    def set_pull(self, pull):
        self.pin.init(Pin.IN, pull)

    def set_mode(self, mode, pull=None):
        self.pin.init(mode, pull)

    def pulse(self, duration_ms):
        self.pin.init(Pin.OUT)
        self.pin.value(1)
        time.sleep_ms(duration_ms)
        self.pin.value(0)
        self.pin.init(Pin.IN)

    def toggle(self):
        self.pin.init(Pin.OUT)
        self.pin.value(not self.pin.value())
        self.pin.init(Pin.IN)

    def set_high(self):
        self.pin.init(Pin.OUT)
        self.pin.value(1)
        self.pin.init(Pin.IN)

    def set_low(self):
        self.pin.init(Pin.OUT)
        self.pin.value(0)
        self.pin.init(Pin.IN)

    def deinit(self):
        self.disable()
        self.pin = None


"""Example usage:

Raspberry Pi Pico GPIO Input:
GPIO0 - GPIO29: General-purpose digital input/output pins

def interrupt_handler(pin):
    print(f"Interrupt detected on pin {pin.id()} with value {pin.value()}")

def main():
    interrupt_pin = InterruptPin(pin_number=14, trigger=Pin.IRQ_FALLING)

    interrupt_pin.set_callback(interrupt_handler)

    print("Interrupt pin set up. Press the button connected to pin 14 to trigger an interrupt.")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Program interrupted by user.")
    finally:
        interrupt_pin.deinit()
        print("Interrupt pin deinitialized.")

if __name__ == "__main__":
    main()

"""
