import network, time


class WiFi:
    def __init__(self, ssid, password):
        self.ssid = ssid
        self.password = password
        self.wlan = network.WLAN(network.STA_IF)
        self.wlan.active(True)

    def connect(self, timeout=30):
        if self.wlan.isconnected():
            return self.wlan.ifconfig()

        self.wlan.active(True)
        self.wlan.connect(self.ssid, self.password)

        start = time.ticks_ms()
        while not self.wlan.isconnected():
            if time.ticks_diff(time.ticks_ms(), start) > timeout * 1000:
                st = self.wlan.status()
                raise Exception("WiFi timeout, status={}".format(st))
            time.sleep(0.2)

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
