from machine import Pin
import time

led = Pin(25, Pin.OUT)
btn = Pin(15, Pin.IN, Pin.PULL_UP)

fired = False


def on_btn(pin):
    global fired
    fired = True  # ISR krótki: flaga


btn.irq(trigger=Pin.IRQ_FALLING, handler=on_btn)

while True:
    if fired:
        fired = False
        led.toggle()
        print("IRQ!")
    time.sleep(0.01)
