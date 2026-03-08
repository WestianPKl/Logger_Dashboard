import time
from machine import Pin


class GPIO:
    def __init__(self, pin_number, mode=Pin.OUT, pull=None):
        self.pin = Pin(pin_number, mode, pull)

        if mode == Pin.IN:
            self.pin.irq(
                trigger=Pin.IRQ_RISING | Pin.IRQ_FALLING, handler=self._irq_handler
            )

    def set_high(self):
        self.pin.value(1)

    def set_low(self):
        self.pin.value(0)

    def toggle(self):
        self.pin.value(not self.pin.value())

    def read(self):
        return self.pin.value()

    def pulse(self, duration_ms):
        self.set_high()
        time.sleep_ms(duration_ms)
        self.set_low()

    def _irq_handler(self, pin):
        print("Interrupt detected on pin:", pin)

    def set_mode(self, mode, pull=None):
        self.pin.init(mode, pull)

    def enable_interrupt(self, trigger=Pin.IRQ_RISING | Pin.IRQ_FALLING, callback=None):
        if callback:
            self.pin.irq(trigger=trigger, handler=callback)
        else:
            self.pin.irq(trigger=trigger, handler=self._irq_handler)

    def disable_interrupt(self):
        self.pin.irq(handler=None)

    def deinit(self):
        self.disable_interrupt()
        self.pin = None


"""Example usage:

Raspberry Pi Pico GPIO Input:
GPIO0 - GPIO29: General-purpose digital input/output pins

def main():
    gpio_pin = GPIO(pin_number=14, mode=Pin.IN, pull=Pin.PULL_UP)

    def custom_interrupt_handler(pin):
        print(f"Custom interrupt detected on pin {pin.id()} with value {pin.value()}")

    gpio_pin.enable_interrupt(trigger=Pin.IRQ_FALLING, callback=custom_interrupt_handler)

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Program interrupted by user.")
    finally:
        gpio_pin.deinit()

if __name__ == "__main__":
    main()
    
"""
