import network
import time


class WiFi:
    def __init__(self, ssid, password):
        self.ssid = ssid
        self.password = password
        self.wlan = network.WLAN(network.STA_IF)
        self.wlan.active(True)

    def connect(self, timeout=10):
        if not self.wlan.isconnected():
            self.wlan.connect(self.ssid, self.password)
            start_time = time.time()
            while not self.wlan.isconnected():
                if time.time() - start_time > timeout:
                    raise Exception("Connection timed out")
                time.sleep(1)
        return self.wlan.ifconfig()

    def disconnect(self):
        if self.wlan.isconnected():
            self.wlan.disconnect()

    def is_connected(self):
        return self.wlan.isconnected()

    def get_ip(self):
        if self.wlan.isconnected():
            return self.wlan.ifconfig()[0]
        else:
            return None

    def deinit(self):
        self.wlan.active(False)
        self.wlan = None


"""Example usage:

Raspberry Pi Pico Network:
Use your Wi-Fi network credentials

def main():
    ssid = "your_SSID"
    password = "your_PASSWORD"
    wifi = WiFi(ssid, password)

    try:
        print("Connecting to WiFi...")
        ip_info = wifi.connect()
        print(f"Connected! IP Address: {ip_info[0]}")

        # Keep the connection for 30 seconds
        time.sleep(30)

    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        print("Disconnecting from WiFi...")
        wifi.disconnect()
        wifi.deinit()
        print("Disconnected.")

if __name__ == "__main__":
    main()

"""
