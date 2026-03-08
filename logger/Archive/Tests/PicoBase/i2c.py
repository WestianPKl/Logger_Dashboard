import time
from machine import Pin, I2C


class I2CDevice:
    def __init__(self, scl_pin, sda_pin, freq=400000, addr=0x3C):
        self.i2c = I2C(0, scl=Pin(scl_pin), sda=Pin(sda_pin), freq=freq)
        self.addr = addr

    def scan(self):
        return self.i2c.scan()

    def write(self, data):
        if isinstance(data, str):
            data = data.encode("utf-8")
        self.i2c.writeto(self.addr, data)

    def read(self, nbytes):
        return self.i2c.readfrom(self.addr, nbytes)

    def write_read(self, write_data, nbytes):
        if isinstance(write_data, str):
            write_data = write_data.encode("utf-8")
        return self.i2c.writeto_mem(self.addr, 0x00, write_data) or self.i2c.readfrom(
            self.addr, nbytes
        )

    def scan_devices(self):
        devices = self.i2c.scan()
        return [hex(device) for device in devices]

    def is_device_connected(self):
        return self.addr in self.i2c.scan()

    def read_from_mem(self, memaddr, nbytes):
        return self.i2c.readfrom_mem(self.addr, memaddr, nbytes)

    def write_to_mem(self, memaddr, data):
        if isinstance(data, str):
            data = data.encode("utf-8")
        self.i2c.writeto_mem(self.addr, memaddr, data)

    def deinit(self):
        self.i2c = None


"""Example usage:

Raspberry Pi Pico I2C Pins:
For I2C0: GPIO4 (SDA), GPIO5 (SCL)
For I2C1: GPIO2 (SDA), GPIO3 (SCL)

def main():
    i2c_device = I2CDevice(scl_pin=5, sda_pin=4, freq=400000, addr=0x3C)

    try:
        print("Scanning for I2C devices...")
        devices = i2c_device.scan_devices()
        print(f"Found I2C devices at addresses: {devices}")

        if i2c_device.is_device_connected():
            print(f"Device found at address {hex(i2c_device.addr)}")
            test_message = "Hello, I2C!"
            print(f"Sending: {test_message}")
            i2c_device.write(test_message)

            time.sleep(1)

            received_data = i2c_device.read(len(test_message))
            print(f"Received: {received_data.decode("utf-8")}")
        else:
            print(f"No device found at address {hex(i2c_device.addr)}")

    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        i2c_device.deinit()

if __name__ == "__main__":
    main()

"""
