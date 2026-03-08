import time
import rp2
from machine import Pin


class PIOProgram:
    def __init__(self, program, freq=2000000, sm_id=0, pin_number=0):
        self.program = program
        self.freq = freq
        self.sm_id = sm_id
        self.pin = Pin(pin_number, Pin.OUT)
        self.sm = rp2.StateMachine(sm_id, program, freq=freq, sideset_base=self.pin)
        self.sm.active(1)

    def write(self, data):
        if isinstance(data, str):
            data = data.encode("utf-8")
        for byte in data:
            self.sm.put(byte)

    def read(self, nbytes):
        result = bytearray()
        for _ in range(nbytes):
            result.append(self.sm.get() & 0xFF)
        return bytes(result)

    def deinit(self):
        self.sm.active(0)
        self.sm = None
        self.pin = None


"""Example usage:

Raspberry Pi Pico PIO:
GPIO0 - GPIO29: General-purpose digital input/output pins

def main():
    @rp2.asm_pio(sideset_init=rp2.PIO.OUT_LOW)
    def simple_program():
        pull()               .side(0)
        out(pins, 8)        .side(1)
        jmp(simple_program)

    pio_device = PIOProgram(program=simple_program, freq=2000000, sm_id=0, pin_number=15)

    try:
        test_message = "Hello, PIO!"
        print(f"Sending: {test_message}")
        pio_device.write(test_message)

        time.sleep(1)

        received_data = pio_device.read(len(test_message))
        print(f"Received: {received_data.decode('utf-8')}")

    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        pio_device.deinit()

if __name__ == "__main__":
    main()
    
"""
