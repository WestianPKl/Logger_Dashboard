import time
from machine import Pin, PWM


class PWMOutput:
    def __init__(self, pin_number, freq=1000, duty=512):
        self.pin = Pin(pin_number, Pin.OUT)
        self.pwm = PWM(self.pin)
        self.pwm.freq(freq)
        self.pwm.duty(duty)

    def set_frequency(self, freq):
        self.pwm.freq(freq)

    def set_duty_cycle(self, duty):
        if 0 <= duty <= 1023:
            self.pwm.duty(duty)
        else:
            raise ValueError("Duty cycle must be between 0 and 1023")

    def stop(self):
        self.pwm.deinit()

    def deinit(self):
        self.pwm.deinit()
        self.pwm = None
        self.pin = None


"""Example usage:

Raspberry Pi Pico PWM Output:
GPIO0 - GPIO29: General-purpose digital input/output pins

def main():
    pwm_output = PWMOutput(pin_number=15, freq=1000, duty=512)

    try:
        print("Starting PWM output...")
        pwm_output.start()

        for duty in range(0, 1024, 128):
            print(f"Setting duty cycle to {duty}")
            pwm_output.set_duty_cycle(duty)
            time.sleep(1)

        for duty in range(1023, -1, -128):
            print(f"Setting duty cycle to {duty}")
            pwm_output.set_duty_cycle(duty)
            time.sleep(1)

    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        print("Stopping PWM output...")
        pwm_output.stop()
        pwm_output.deinit()

if __name__ == "__main__":
    main()
    
"""
