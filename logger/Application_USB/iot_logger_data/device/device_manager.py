import threading, queue, time, serial
from typing import Callable, Optional


class DeviceManager:
    def __init__(self):
        self.ser: Optional[serial.Serial] = None
        self._running = False
        self._thread: Optional[threading.Thread] = None
        self._tx_queue: queue.Queue[bytes] = queue.Queue()

        self.on_log: Optional[Callable[[str], None]] = None
        self.on_data: Optional[Callable[[str], None]] = None
        self.on_connect: Optional[Callable[[str, int], None]] = None
        self.on_disconnect: Optional[Callable[[], None]] = None
        self.on_error: Optional[Callable[[str], None]] = None

    @property
    def is_connected(self) -> bool:
        return self.ser is not None and self.ser.is_open

    def connect(self, port: str, baudrate: int = 115200, timeout: float = 0.1):
        if self.is_connected:
            self._emit_log("Connection already established. Please disconnect first.")
            return

        try:
            self.ser = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)
            self._running = True
            self._thread = threading.Thread(target=self._io_loop, daemon=True)
            self._thread.start()
            self._emit_log(f"Connected to {port} @ {baudrate}.")
            if self.on_connect:
                self.on_connect(port, baudrate)
        except Exception as e:
            self.ser = None
            self._running = False
            self._emit_error(f"Connection error: {e}")

    def disconnect(self):
        if not self.is_connected:
            self._emit_log("No active connection.")
            return

        self._running = False
        try:
            if self.ser:
                try:
                    self.ser.cancel_read()
                except Exception:
                    pass
                self.ser.close()
        except Exception as e:
            self._emit_error(f"Error during disconnect: {e}")
        finally:
            self.ser = None
            self._emit_log("Disconnected.")
            if self.on_disconnect:
                self.on_disconnect()

    def send_text(self, text: str, append_newline: bool = True):
        if not self.is_connected:
            self._emit_log("Cannot send: no connection.")
            return

        payload = text + ("\n" if append_newline else "")
        self._tx_queue.put(payload.encode("utf-8", errors="replace"))

    def send_bytes(self, data: bytes):
        if not self.is_connected:
            self._emit_log("Cannot send: no connection.")
            return
        self._tx_queue.put(data)

    def _io_loop(self):
        while self._running and self.ser:
            try:
                while not self._tx_queue.empty():
                    packet = self._tx_queue.get_nowait()
                    self.ser.write(packet)
                    try:
                        tx_preview = packet.decode("utf-8", errors="replace").rstrip()
                    except Exception:
                        tx_preview = repr(packet)
                    self._emit_log(f"TX > {tx_preview}")

                incoming = self.ser.readline()
                if incoming:
                    decoded = incoming.decode("utf-8", errors="replace").rstrip("\r\n")
                    if self.on_data:
                        self.on_data(decoded)
                    self._emit_log(f"RX < {decoded}")

            except serial.SerialException as e:
                self._emit_error(f"Serial port error: {e}")
                break
            except Exception as e:
                self._emit_error(f"IO error: {e}")
                break

            time.sleep(0.01)

        try:
            if self.ser:
                self.ser.close()
        except Exception:
            pass

        was_connected = self.ser is not None
        self.ser = None
        self._running = False
        if was_connected and self.on_disconnect:
            self.on_disconnect()

    def _emit_log(self, message: str):
        if self.on_log:
            self.on_log(message)

    def _emit_error(self, message: str):
        if self.on_error:
            self.on_error(message)
        self._emit_log(message)

    def send_command(self, command: str, *args):
        parts = [command.upper()] + [str(a) for a in args]
        self.send_text(",".join(parts))
