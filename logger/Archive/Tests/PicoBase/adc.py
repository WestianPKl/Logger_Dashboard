import time
from machine import Pin, ADC


class ADCInput:
    def __init__(
        self, pin_number, atten=ADC.ATTN_11DB, width=ADC.WIDTH_12BIT, v_ref=3.3
    ):
        self.pin = Pin(pin_number, Pin.IN)
        self.adc = ADC(self.pin)
        self.adc.atten(atten)
        self.adc.width(width)
        self.v_ref = v_ref

    def read(self):
        return self.adc.read()

    def read_voltage(self):
        raw_value = self.read()
        if self.adc.width() == ADC.WIDTH_8BIT:
            max_value = 255
        elif self.adc.width() == ADC.WIDTH_12BIT:
            max_value = 4095
        elif self.adc.width() == ADC.WIDTH_16BIT:
            max_value = 65535

        voltage = (raw_value / max_value) * self.v_ref
        return voltage

    def read_average(self, samples=10):
        total = 0
        for _ in range(samples):
            total += self.read()
            time.sleep_ms(10)
        return total // samples

    def deinit(self):
        self.adc = None
        self.pin = None


"""Example usage:

Raspberry Pi Pico ADC Input:
ADC2: GPIO28
ADC1: GPIO27
ADC0: GPIO26
AGND: GPIO29 (Ground reference for ADC)
ADCVREF: GPIO30 (Reference voltage input for ADC)

def main():
    adc_input = ADCInput(pin_number=32, atten=ADC.ATTN_11DB, width=ADC.WIDTH_12BIT, v_ref=3.3)

    try:
        while True:
            raw_value = adc_input.read()
            voltage = adc_input.read_voltage()
            avg_value = adc_input.read_average(samples=20)

            print(f"Raw ADC Value: {raw_value}")
            print(f"Voltage: {voltage:.2f} V")
            print(f"Average ADC Value (20 samples): {avg_value}")
            print("-----------------------------")

            time.sleep(2)

    except KeyboardInterrupt:
        print("Program interrupted by user.")
    finally:
        adc_input.deinit()

if __name__ == "__main__":
    main()

"""
