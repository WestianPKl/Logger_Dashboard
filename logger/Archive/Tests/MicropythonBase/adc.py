from machine import ADC
import time

adc = ADC(26)  # GPIO26 = ADC0

while True:
    raw = adc.read_u16()  # 0..65535 (skalowane z 12-bit)
    print("ADC:", raw)
    time.sleep(0.2)
