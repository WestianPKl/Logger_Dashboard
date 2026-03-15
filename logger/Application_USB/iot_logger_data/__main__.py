from pathlib import Path
from PySide6.QtCore import QStringListModel, QTimer, Signal, Qt
from PySide6.QtGui import QIcon, QStandardItemModel, QStandardItem
from PySide6.QtWidgets import QApplication, QMainWindow, QFileDialog, QProgressBar
from .ui.window import Ui_MainWindow
from .device.device_manager import DeviceManager
from .config.config import ConfigInformation
from serial.tools import list_ports
from typing import Callable, Optional

cfg = ConfigInformation


def get_application_icon():
    if cfg.ICON_FILE.exists():
        return QIcon(str(cfg.ICON_FILE))
    return QIcon()


class IoTLoggerApp(QMainWindow):
    _sig_log = Signal(str)
    _sig_error = Signal(str)
    _sig_data = Signal(str)
    _sig_connect = Signal(str, int)
    _sig_disconnect = Signal()

    def __init__(self, parent=None):
        super().__init__(parent)

        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        self.ui.set_frequency_slider.setRange(0, 15000)
        self.ui.set_volume_slider.setRange(0, 100)
        self.ui.set_red_slider.setRange(0, 255)
        self.ui.set_green_slider.setRange(0, 255)
        self.ui.set_blue_slider.setRange(0, 255)
        self.ui.set_brightness_slider.setRange(0, 100)

        for s in (
            self.ui.set_red_slider,
            self.ui.set_green_slider,
            self.ui.set_blue_slider,
            self.ui.set_brightness_slider,
            self.ui.set_volume_slider,
        ):
            s.setSingleStep(20)
            s.setPageStep(20)
        self.ui.set_frequency_slider.setSingleStep(1000)
        self.ui.set_frequency_slider.setPageStep(1000)

        self._rgb_timer = QTimer(self)
        self._rgb_timer.setSingleShot(True)
        self._rgb_timer.setInterval(100)
        self._rgb_timer.timeout.connect(self._send_rgb)

        self._buzzer_timer = QTimer(self)
        self._buzzer_timer.setSingleShot(True)
        self._buzzer_timer.setInterval(100)
        self._buzzer_timer.timeout.connect(self._send_buzzer)

        self.ui.set_red_slider.valueChanged.connect(self._on_rgb_changed)
        self.ui.set_green_slider.valueChanged.connect(self._on_rgb_changed)
        self.ui.set_blue_slider.valueChanged.connect(self._on_rgb_changed)
        self.ui.set_brightness_slider.valueChanged.connect(self._on_rgb_changed)
        self._update_color_preview()

        self.ui.set_frequency_slider.valueChanged.connect(self._on_buzzer_changed)
        self.ui.set_volume_slider.valueChanged.connect(self._on_buzzer_changed)

        self.setWindowIcon(get_application_icon())
        self.setWindowTitle(f"{cfg.APP_NAME} {cfg.VERSION}")
        self.setFixedSize(self.size())

        self.log_model = QStringListModel()
        self.ui.log_view.setModel(self.log_model)
        self.log_entries = []

        self.status_model = QStandardItemModel(0, 2)
        self.status_model.setHorizontalHeaderLabels(["Parameter", "Value"])
        self.ui.loaded_data_table_view.setModel(self.status_model)
        self.ui.loaded_data_table_view.horizontalHeader().setStretchLastSection(True)

        self._progress_bar = QProgressBar(self)
        self._progress_bar.setRange(0, 0)
        self._progress_bar.setTextVisible(False)
        self._progress_bar.setFixedHeight(3)
        self._progress_bar.hide()
        self.statusBar().addPermanentWidget(self._progress_bar, 1)
        self.statusBar().showMessage("Ready")

        self._startup_queue = []
        self._pending_startup = set()
        self._startup_timer = QTimer(self)
        self._startup_timer.setInterval(50)
        self._startup_timer.timeout.connect(self._send_next_startup_cmd)

        self._sig_log.connect(lambda msg: self.log_message(msg))
        self._sig_error.connect(lambda msg: self.log_message(msg, level="ERROR"))
        self._sig_data.connect(self._on_data_received)
        self._sig_connect.connect(self._on_connected_ui)
        self._sig_disconnect.connect(self._on_disconnected_ui)

        self.device_manager = DeviceManager()
        self.device_manager.on_log = lambda msg: self._sig_log.emit(msg)
        self.device_manager.on_data = lambda data: self._sig_data.emit(data)
        self.device_manager.on_connect = lambda p, b: self._sig_connect.emit(p, b)
        self.device_manager.on_disconnect = lambda: self._sig_disconnect.emit()
        self.device_manager.on_error = lambda msg: self._sig_error.emit(msg)

        self.received_callback: Optional[Callable[[str], None]] = None

        self.connect_signals()
        self.load_ports()
        self._set_connected_state(False)

    def _on_data_received(self, data: str):
        self._handle_response(data)
        if self.received_callback:
            try:
                self.received_callback(data)
            except Exception as e:
                self.log_message(f"Error in receive callback: {e}", level="ERROR")

    def _on_connected_ui(self, port: str, baudrate: int):
        self._set_connected_state(True)
        self.log_message(f"Active connection: {port} @ {baudrate}")
        self._request_startup_data()
        self._show_loading(True)

    def _on_disconnected_ui(self):
        self._set_connected_state(False)
        self._startup_timer.stop()
        self._pending_startup.clear()
        self._show_loading(False)
        self.log_message("Connection closed.")

    def set_received_callback(self, callback: Callable[[str], None]):
        self.received_callback = callback
        self.log_message("Callbacks connected for received data.")

    def connect_signals(self):
        self.ui.connect_button.clicked.connect(self.on_connect)
        self.ui.disconnect_button.clicked.connect(self.on_disconnect)
        self.ui.refresh_port_button.clicked.connect(self.on_refresh_ports)
        self.ui.ping_button.clicked.connect(lambda: self._send("PING"))
        self.ui.reset_button.clicked.connect(lambda: self._send("RESET"))
        self.ui.sync_time_button.clicked.connect(lambda: self._send("SYNC_TIME"))
        self.ui.read_time_button.clicked.connect(lambda: self._send("READ_RTC"))
        self.ui.read_data_button.clicked.connect(lambda: self._send("READ_CONFIG"))
        self.ui.read_flags_button.clicked.connect(lambda: self._send("READ_FLAGS"))
        self.ui.service_enable_button.clicked.connect(self.on_service_enable)
        self.ui.service_disable_button.clicked.connect(self.on_service_disable)

        self.ui.read_sht_button.clicked.connect(lambda: self._send("READ_SHT40"))
        self.ui.read_bme_button.clicked.connect(lambda: self._send("READ_BME280"))
        self.ui.set_pwm_button.clicked.connect(self.on_set_pwm)
        self.ui.output_on_button.clicked.connect(self._on_output_on)
        self.ui.output_off_button.clicked.connect(self._on_output_off)

        self.ui.save_connection_data_button.clicked.connect(
            self.on_save_connection_data
        )
        self.ui.restore_connection_data_button.clicked.connect(
            lambda: self._send("READ_CONFIG")
        )
        self.ui.save_flags_button.clicked.connect(self.on_save_flags)
        self.ui.restore_flags_button.clicked.connect(lambda: self._send("READ_FLAGS"))

        self.ui.save_log_button.clicked.connect(self.on_save_log)
        self.ui.clear_log_button.clicked.connect(self.on_clear_log)

    def _update_color_preview(self):
        r = self.ui.set_red_slider.value()
        g = self.ui.set_green_slider.value()
        b = self.ui.set_blue_slider.value()
        self.ui.current_color.setStyleSheet(f"background-color: rgb({r},{g},{b});")

    def _on_rgb_changed(self):
        self._update_color_preview()
        self._rgb_timer.start()

    def _send_rgb(self):
        r = self.ui.set_red_slider.value()
        g = self.ui.set_green_slider.value()
        b = self.ui.set_blue_slider.value()
        br = self.ui.set_brightness_slider.value()
        self._send("RGB", r, g, b, br)

    def _on_buzzer_changed(self):
        self._buzzer_timer.start()

    def _send_buzzer(self):
        freq = self.ui.set_frequency_slider.value()
        vol = self.ui.set_volume_slider.value()
        self._send("BUZZER", freq, vol)

    def _handle_response(self, data: str):
        try:
            self.log_message(f"Received: {data}")
            if data.startswith("OK:"):
                payload = data[3:]
                comma_idx = payload.find(",")
                if comma_idx == -1:
                    return
                cmd = payload[:comma_idx]
                rest = payload[comma_idx + 1 :]

                self._pending_startup.discard(cmd)
                self._check_startup_done()

                if cmd == "READ_SHT40":
                    parts = rest.split(",")
                    if len(parts) >= 2:
                        self.ui.sht_temperature.display(float(parts[0]))
                        self.ui.sht_humidity.display(float(parts[1]))

                elif cmd == "READ_BME280":
                    parts = rest.split(",")
                    if len(parts) >= 3:
                        self.ui.bme_temperature.display(float(parts[0]))
                        self.ui.bme_humidity.display(float(parts[1]))
                        self.ui.bme_pressure.display(float(parts[2]))

                elif cmd == "READ_RTC":
                    self.ui.current_time_text.setText(
                        rest.replace("T", " ").replace("Z", "")
                    )

                elif cmd == "READ_FLAGS":
                    self._apply_flags(rest)

                elif cmd == "READ_CONFIG":
                    self._apply_config(rest)

                elif cmd == "OUTPUT_CHANNELS":
                    self._apply_channels(self.ui.select_output_box, rest)

                elif cmd == "PWM_CHANNELS":
                    self._apply_channels(self.ui.select_pwm_box, rest)

                elif cmd == "INPUT_CHANNELS":
                    self.log_message(f"Input channels: {rest}")

                elif cmd == "STATUS":
                    self._apply_status(rest)

            elif data.startswith("STATUS:"):
                parts = data[7:].split(",")
                if parts[0] == "ALIVE" and len(parts) >= 2:
                    version = parts[1]
                    build = parts[2] if len(parts) >= 3 else ""
                    self.log_message(
                        f"Device alive - FW: {version}"
                        + (f" Build: {build}" if build else "")
                    )

            elif data.startswith("ERR:"):
                self.log_message(f"Device error: {data[4:]}", level="ERROR")

        except Exception as e:
            self.log_message(f"Response parse error: {e}", level="ERROR")

    def _apply_flags(self, raw: str):
        import ast

        flags = ast.literal_eval(raw)
        self.ui.ext_rtc_flag.setChecked(bool(flags.get("FRAM_FLAG_EXT_RTC_PRESENT", 0)))
        self.ui.flash_memory_flag.setChecked(
            bool(flags.get("FRAM_FLAG_FLASH_PRESENT", 0))
        )
        self.ui.lcd_display_flag.setChecked(
            bool(flags.get("FRAM_FLAG_DISPLAY_PRESENT", 0))
        )
        self.ui.sht_sensor_flag.setChecked(
            bool(flags.get("FRAM_FLAG_SHT40_PRESENT", 0))
        )
        self.ui.bme_sensor_flag.setChecked(
            bool(flags.get("FRAM_FLAG_BME280_PRESENT", 0))
        )
        self.ui.ina226_flag.setChecked(bool(flags.get("FRAM_FLAG_INA226_PRESENT", 0)))
        self.ui.adc_measurement_flag.setChecked(
            bool(flags.get("FRAM_FLAG_ADC_PRESENT", 0))
        )
        self.ui.network_enable_flag.setChecked(bool(flags.get("WIFI_ENABLED", 0)))

    def _apply_channels(self, combo_box, raw: str):
        import ast

        channels = ast.literal_eval(raw)
        combo_box.clear()
        for ch in channels:
            combo_box.addItem(str(ch))

    def _apply_status(self, raw: str):
        import ast

        status = ast.literal_eval(raw)
        self.status_model.removeRows(0, self.status_model.rowCount())
        labels = {
            "controller_sw": "Controller SW",
            "controller_hw": "Controller HW",
            "controller_build_date": "Controller Build Date",
            "controller_prod_date": "Controller Prod Date",
            "communication_sw": "Communication SW",
            "communication_build": "Communication Build",
            "logger_id": "Logger ID",
            "sensor_id": "Sensor ID",
            "ip_address": "IP Address",
            "service_mode": "Service Mode",
        }
        for key, label in labels.items():
            val = str(status.get(key, ""))
            row = [QStandardItem(label), QStandardItem(val)]
            row[0].setFlags(row[0].flags() & ~Qt.ItemFlag.ItemIsEditable)
            row[1].setFlags(row[1].flags() & ~Qt.ItemFlag.ItemIsEditable)
            self.status_model.appendRow(row)

    def _apply_config(self, raw: str):
        import ast

        config = ast.literal_eval(raw)
        self.ui.set_logger_id_line.setText(str(config.get("logger_id", "")))
        self.ui.set_sensor_id_line.setText(str(config.get("sensor_id", "")))
        self.ui.set_wifi_ssid_line.setText(str(config.get("wifi_ssid", "")))
        self.ui.set_wifi_password_line.setText(str(config.get("wifi_password", "")))
        self.ui.set_mqtt_server_line.setText(str(config.get("mqtt_server", "")))
        self.ui.set_ntp_server_line.setText(str(config.get("ntp_server", "")))
        self.ui.set_mqtt_user_line.setText(str(config.get("mqtt_user", "")))
        self.ui.set_mqtt_password_line.setText(str(config.get("mqtt_password", "")))

    def _send(self, command: str, *args):
        if self.device_manager.is_connected:
            self.log_message(f"Sending command: {command} {' '.join(map(str, args))}")
            self.device_manager.send_command(command, *args)
        else:
            self.log_message("Not connected. Connect first.")

    def on_connect(self):
        port_text = self.ui.port_box.currentText()
        if port_text and "No available ports" not in port_text:
            port = port_text.split()[0].strip()
            try:
                self.device_manager.connect(port, baudrate=cfg.DEFAULT_BAUDRATE)
            except Exception as e:
                self.log_message(f"Serial port error: {e}")
        else:
            self.log_message("Please select a port")

    def on_disconnect(self):
        self.device_manager.disconnect()

    def _set_connected_state(self, connected: bool):
        self.ui.port_box.setEnabled(not connected)
        self.ui.connect_button.setEnabled(not connected)
        self.ui.refresh_port_button.setEnabled(not connected)
        self.ui.disconnect_button.setEnabled(connected)

    def on_refresh_ports(self):
        self.log_message("Refreshing ports...")
        self.load_ports()

    def load_ports(self):
        self.ui.port_box.clear()
        ports = list_ports.comports()
        if ports:
            for port in ports:
                self.ui.port_box.addItem(f"{port.device} ({port.description})")
            self.log_message(f"Ports found: {len(ports)}")
        else:
            self.ui.port_box.addItem("No available ports")
            self.log_message("No available ports")

    def on_service_enable(self):
        password = self.ui.service_password.text()
        if not password:
            self.log_message("Enter service mode password")
            return
        self._send("SERVICE_MODE_ENABLE", password)
        self._send("READ_CONFIG")
        self._send("READ_FLAGS")
        self.ui.service_password.clear()

    def on_service_disable(self):
        self._send("SERVICE_MODE_DISABLE")
        self._send("READ_CONFIG")
        self._send("READ_FLAGS")
        self.ui.service_password.clear()
        self.log_entries.clear()
        self.log_model.setStringList(self.log_entries)
        self.log_message("Log cleared")

    def _on_output_on(self):
        ch = self.ui.select_output_box.currentText()
        if ch:
            self._send("SET_OUTPUT", ch, 1)
        else:
            self.log_message("Select an output channel first")

    def _on_output_off(self):
        ch = self.ui.select_output_box.currentText()
        if ch:
            self._send("SET_OUTPUT", ch, 0)
        else:
            self.log_message("Select an output channel first")

    def on_set_pwm(self):
        ch = self.ui.select_pwm_box.currentText()
        duty = self.ui.set_duty_cycle_slider.value()
        if ch:
            self._send("SET_PWM", ch, duty)
        else:
            self.log_message("Select a PWM channel first")

    def _request_startup_data(self):
        self._startup_queue = [
            "GET_STATUS",
            "GET_OUTPUT_CHANNELS",
            "GET_PWM_CHANNELS",
            "GET_INPUT_CHANNELS",
            "READ_FLAGS",
            "READ_CONFIG",
            "READ_RTC",
        ]
        self._pending_startup = {
            "STATUS",
            "OUTPUT_CHANNELS",
            "PWM_CHANNELS",
            "INPUT_CHANNELS",
            "READ_FLAGS",
            "READ_CONFIG",
            "READ_RTC",
        }
        self._startup_timer.start()

    def _send_next_startup_cmd(self):
        if self._startup_queue:
            cmd = self._startup_queue.pop(0)
            self.device_manager.send_command(cmd)
        else:
            self._startup_timer.stop()

    def _show_loading(self, loading: bool):
        if loading:
            self._progress_bar.show()
            self.statusBar().showMessage("Loading data...")
            self.ui.tabWidget.setEnabled(False)
            self.setCursor(Qt.CursorShape.WaitCursor)
        else:
            self._progress_bar.hide()
            self.statusBar().showMessage("Ready")
            self.ui.tabWidget.setEnabled(True)
            self.unsetCursor()

    def _check_startup_done(self):
        if not self._pending_startup:
            self._show_loading(False)

    def on_save_connection_data(self):
        if not self.device_manager.is_connected:
            self.log_message("Not connected. Connect first.")
            return
        ssid = self.ui.set_wifi_ssid_line.text().strip()
        password = self.ui.set_wifi_password_line.text().strip()
        mqtt_server = self.ui.set_mqtt_server_line.text().strip()
        ntp_server = self.ui.set_ntp_server_line.text().strip()
        mqtt_user = self.ui.set_mqtt_user_line.text().strip()
        mqtt_pass = self.ui.set_mqtt_password_line.text().strip()
        self._send("WRITE_WIFI", ssid, password)
        self._send("WRITE_SERVERS", mqtt_server, ntp_server)
        self._send("WRITE_MQTT_CREDENTIALS", mqtt_user, mqtt_pass)

    def on_save_flags(self):
        flags = 0
        if self.ui.ext_rtc_flag.isChecked():
            flags |= 1 << 0
        if self.ui.flash_memory_flag.isChecked():
            flags |= 1 << 1
        if self.ui.lcd_display_flag.isChecked():
            flags |= 1 << 2
        if self.ui.sht_sensor_flag.isChecked():
            flags |= 1 << 3
        if self.ui.bme_sensor_flag.isChecked():
            flags |= 1 << 4
        if self.ui.ina226_flag.isChecked():
            flags |= 1 << 5
        if self.ui.adc_measurement_flag.isChecked():
            flags |= 1 << 6
        network = 1 if self.ui.network_enable_flag.isChecked() else 0
        self._send("WRITE_FLAGS", hex(flags), network)

    def on_save_log(self):
        file_name, _ = QFileDialog.getSaveFileName(
            self, "Save Log", "", "Text Files (*.txt);;All Files (*)"
        )
        if file_name:
            try:
                with open(file_name, "w") as file:
                    for entry in self.log_entries:
                        file.write(entry + "\n")
                self.log_message(f"Log saved to {file_name}")
            except Exception as e:
                self.log_message(f"Error saving log: {e}")

    def on_clear_log(self):
        self.log_entries.clear()
        self.log_model.setStringList(self.log_entries)
        self.log_message("Log cleared")

    def log_message(self, message, level="INFO"):
        from datetime import datetime

        timestamp = datetime.now().strftime("%H:%M:%S")
        log_entry = f"[{timestamp}] [{level}] {message}"
        self.log_entries.append(log_entry)
        self.log_model.setStringList(self.log_entries)

    def closeEvent(self, event):
        try:
            if self.device_manager.is_connected:
                self.device_manager.disconnect()
        except Exception:
            pass
        super().closeEvent(event)


if __name__ == "__main__":
    app = QApplication([])
    window = IoTLoggerApp()
    window.show()
    app.exec()
