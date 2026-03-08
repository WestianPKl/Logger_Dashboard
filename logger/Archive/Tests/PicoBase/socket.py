import network
import socket
import time


class PicoSocket:
    def __init__(self, ssid, password):
        self.ssid = ssid
        self.password = password
        self.station = network.WLAN(network.STA_IF)
        self.station.active(True)
        self.station.connect(self.ssid, self.password)
        while not self.station.isconnected():
            time.sleep(1)
        print("Connection successful")
        print(self.station.ifconfig())

    def create_socket(self, host="", port=80):
        addr = socket.getaddrinfo(host, port)[0][-1]
        s = socket.socket()
        s.bind(addr)
        s.listen(1)
        print("Listening on", addr)
        return s

    def accept_connection(self, server_socket):
        conn, addr = server_socket.accept()
        print("Client connected from", addr)
        return conn, addr

    def close_socket(self, sock):
        sock.close()

    def deinit(self):
        self.station.active(False)
        self.station = None

    def get_ip(self):
        if self.station.isconnected():
            return self.station.ifconfig()[0]
        else:
            return None

    def is_connected(self):
        return self.station.isconnected()

    def disconnect(self):
        if self.station.isconnected():
            self.station.disconnect()

    def connect(self, timeout=10):
        if not self.station.isconnected():
            self.station.connect(self.ssid, self.password)
            start_time = time.time()
            while not self.station.isconnected():
                if time.time() - start_time > timeout:
                    raise Exception("Connection timed out")
                time.sleep(1)
        return self.station.ifconfig()

    def send_data(self, conn, data):
        if isinstance(data, str):
            data = data.encode("utf-8")
        conn.send(data)

    def receive_data(self, conn, bufsize=1024):
        return conn.recv(bufsize)

    def set_timeout(self, sock, timeout):
        sock.settimeout(timeout)

    def get_timeout(self, sock):
        return sock.gettimeout()

    def setblocking(self, sock, flag):
        sock.setblocking(flag)

    def getblocking(self, sock):
        return sock.getblocking()

    def getpeername(self, sock):
        return sock.getpeername()

    def getsockname(self, sock):
        return sock.getsockname()

    def fileno(self, sock):
        return sock.fileno()

    def shutdown(self, sock, how):
        sock.shutdown(how)

    def setsockopt(self, sock, level, optname, value):
        sock.setsockopt(level, optname, value)

    def getsockopt(self, sock, level, optname, buflen=0):
        return sock.getsockopt(level, optname, buflen)

    def listen(self, sock, backlog=1):
        sock.listen(backlog)

    def bind(self, sock, addr):
        sock.bind(addr)

    def connect_socket(self, sock, addr):
        sock.connect(addr)

    def recvfrom(self, sock, bufsize):
        return sock.recvfrom(bufsize)

    def sendto(self, sock, data, addr):
        if isinstance(data, str):
            data = data.encode("utf-8")
        sock.sendto(data, addr)

    def deinit_socket(self, sock):
        sock.close()


"""Example usage:

Raspberry Pi Pico Network:
Use your Wi-Fi network credentials

def main():
    ssid = "your_SSID"
    password = "your_PASSWORD"
    pico_socket = PicoSocket(ssid, password)

    try:
        server_socket = pico_socket.create_socket(host='', port=80)

        while True:
            conn, addr = pico_socket.accept_connection(server_socket)
            request = pico_socket.receive_data(conn)
            print("Request:", request)

            response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!"
            pico_socket.send_data(conn, response)
            pico_socket.close_socket(conn)

    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        pico_socket.deinit()

if __name__ == "__main__":
    main()

"""
