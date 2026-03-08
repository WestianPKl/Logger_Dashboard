import network
import socket
import time

html_template = """\
HTTP/1.0 200 OK\r
Content-Type: text/html\r
\r
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Pico W</title>
    <meta http-equiv="refresh" content="5">
  </head>
  <body>
    <h1>Raspberry Pi Pico W</h1>
    <ul>
      <li>Test: {counter}</li>
    </ul>

    <h2>Przycisk</h2>
    <form action="/button" method="get">
        <button type="submit">Przycisk</button>
    </form>
  </body>
</html>
"""


def main():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect("your-ssid", "your-password")

    while not wlan.isconnected():
        time.sleep(1)

    print("Network config:", wlan.ifconfig())

    addr = socket.getaddrinfo("0.0.0.0", 80)[0][-1]
    s = socket.socket()
    s.bind(addr)
    s.listen(1)
    print("Listening on", addr)
    counter = 0
    ip = wlan.ifconfig()[0]
    print(f"Access the web page at http://{ip}/")
    while True:
        cl, addr = s.accept()
        print("Client connected from", addr)
        request = cl.recv(1024)
        request_str = request.decode("utf-8")
        print("Request:", request_str)

        if "GET /button" in request_str:
            counter += 1
            cl.send("HTTP/1.1 302 Found\r\nLocation: /\r\n\r\n")
            cl.close()
            continue

        response = html_template.format(counter=counter)
        cl.send(response.encode("utf-8"))
        cl.close()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Program interrupted by user.")
