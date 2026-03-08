from machine import Pin
import time

DATA  = Pin(3, Pin.OUT)
CLOCK = Pin(2, Pin.OUT)
LATCH = Pin(4, Pin.OUT)

digit_pins = [
    Pin(10, Pin.OUT),
    Pin(11, Pin.OUT),
    Pin(12, Pin.OUT),
    Pin(13, Pin.OUT),
]

digits = bytes([0x03, 0x9F, 0x25, 0x0D, 0x99, 0x49, 0x41, 0x1B, 0x01, 0x09])
letters = bytes([0x25, 0x83 , 0x25, 0x11])

buffer = [0, 1, 2, 3]

def shift_out(byte):
    for i in range(8):
        DATA.value((byte >> i) & 1)
        CLOCK.value(1)
        CLOCK.value(0)

def set_segments(pattern):
    LATCH.value(0)
    shift_out(pattern)
    LATCH.value(1)

def all_digits_off():
    for p in digit_pins:
        p.value(0)

def show_digit(pos, val):
    set_segments(digits[val])
    all_digits_off()
    digit_pins[pos].value(1)
    time.sleep_us(500)
    digit_pins[pos].value(0)
    
def show_letters(pos, val):
    set_segments(letters[val])
    all_digits_off()
    digit_pins[pos].value(1)
    time.sleep_us(500)
    digit_pins[pos].value(0)

def main():
    while True:
        for pos in range(4):
            show_letters(pos, buffer[pos])
        
if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Program interrupted by user.")