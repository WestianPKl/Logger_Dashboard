import ast
import hashlib
import os
from datetime import date, datetime
from pathlib import Path
from PySide6.QtCore import QStringListModel, QTimer, Signal, Qt
from PySide6.QtGui import QIcon, QStandardItemModel, QStandardItem
from PySide6.QtWidgets import (
    QApplication,
    QMainWindow,
    QFileDialog,
    QProgressBar,
    QMessageBox,
)
from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qtagg import NavigationToolbar2QT as NavigationToolbar
from matplotlib.figure import Figure
from .ui.window import Ui_MainWindow
from .device.device_manager import DeviceManager
from .config.config import ConfigInformation
from .config.database import DatabaseConnection
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

        self._sig_log.connect(self._on_device_log)
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

        self._data_logs_model = QStandardItemModel(0, 6)
        self._data_logs_model.setHorizontalHeaderLabels(
            ["#", "Timestamp", "Temperature", "Humidity", "Pressure [hPa]", "CRC32"]
        )
        self.ui.data_logs_table.setModel(self._data_logs_model)
        self.ui.data_logs_table.horizontalHeader().setStretchLastSection(True)

        self._log_total = 0
        self._log_index = 0
        self._log_reading = False

        self._log_timeout_timer = QTimer(self)
        self._log_timeout_timer.setSingleShot(True)
        self._log_timeout_timer.setInterval(3000)
        self._log_timeout_timer.timeout.connect(self._on_log_timeout)

        self._figure = Figure(figsize=(5, 4), tight_layout=True)
        self._canvas = FigureCanvas(self._figure)
        self._nav_toolbar = NavigationToolbar(self._canvas, self)
        self.ui.chart_layout.addWidget(self._nav_toolbar)
        self.ui.chart_layout.addWidget(self._canvas)

        self._service_logged_in = False
        self._load_env()

        self.connect_signals()
        self.load_ports()
        self._set_connected_state(False)
        self._set_service_logged_in(False)
        self._check_first_user()

    def _on_device_log(self, msg: str):
        if self._log_reading and ("READ_LOG" in msg or "TX >" in msg or "RX <" in msg):
            return
        self.log_message(msg)

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
        self.ui.output_on_button.clicked.connect(lambda: self._on_set_output(1))
        self.ui.output_off_button.clicked.connect(lambda: self._on_set_output(0))

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

        self.ui.read_all_logs_button.clicked.connect(self.on_read_all_logs)
        self.ui.export_logs_button.clicked.connect(self.on_export_logs_csv)
        self.ui.clear_logs_table_button.clicked.connect(self.on_clear_logs_table)

        self.ui.service_login_button.clicked.connect(self.on_service_login)
        self.ui.service_logout_button.clicked.connect(self.on_service_logout)
        self.ui.keygen_generate_button.clicked.connect(self.on_keygen_generate)

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
            if not self._log_reading:
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

                elif cmd == "READ_LOG_COUNT":
                    self._on_log_count_received(rest)

                elif cmd == "READ_LOG":
                    self._on_log_entry_received(rest)

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
                if self._log_reading:
                    self._log_index += 1
                    self.ui.data_logs_progress.setValue(self._log_index)
                    self._request_next_log()

        except Exception as e:
            self.log_message(f"Response parse error: {e}", level="ERROR")
            if self._log_reading:
                self._log_index += 1
                self._request_next_log()

    _FLAG_MAP = [
        ("ext_rtc_flag", "FRAM_FLAG_EXT_RTC_PRESENT"),
        ("flash_memory_flag", "FRAM_FLAG_FLASH_PRESENT"),
        ("lcd_display_flag", "FRAM_FLAG_DISPLAY_PRESENT"),
        ("sht_sensor_flag", "FRAM_FLAG_SHT40_PRESENT"),
        ("bme_sensor_flag", "FRAM_FLAG_BME280_PRESENT"),
        ("ina226_flag", "FRAM_FLAG_INA226_PRESENT"),
        ("adc_measurement_flag", "FRAM_FLAG_ADC_PRESENT"),
        ("can_flag", "FRAM_FLAG_CAN_PRESENT"),
        ("network_enable_flag", "WIFI_ENABLED"),
    ]

    def _apply_flags(self, raw: str):
        flags = ast.literal_eval(raw)
        for attr, key in self._FLAG_MAP:
            getattr(self.ui, attr).setChecked(bool(flags.get(key, 0)))

    def _apply_channels(self, combo_box, raw: str):
        channels = ast.literal_eval(raw)
        combo_box.clear()
        for ch in channels:
            combo_box.addItem(str(ch))

    def _apply_status(self, raw: str):
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

    _CONFIG_MAP = [
        ("set_logger_id_line", "logger_id"),
        ("set_sensor_id_line", "sensor_id"),
        ("set_wifi_ssid_line", "wifi_ssid"),
        ("set_wifi_password_line", "wifi_password"),
        ("set_mqtt_server_line", "mqtt_server"),
        ("set_ntp_server_line", "ntp_server"),
        ("set_mqtt_user_line", "mqtt_user"),
        ("set_mqtt_password_line", "mqtt_password"),
    ]

    def _apply_config(self, raw: str):
        config = ast.literal_eval(raw)
        for attr, key in self._CONFIG_MAP:
            getattr(self.ui, attr).setText(str(config.get(key, "")))

    def _send(self, command: str, *args):
        if self.device_manager.is_connected:
            if not self._log_reading or command != "READ_LOG":
                self.log_message(
                    f"Sending command: {command} {' '.join(map(str, args))}"
                )
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
        self._set_service_controls_enabled(False)

    def _on_set_output(self, value: int):
        ch = self.ui.select_output_box.currentText()
        if ch:
            self._send("SET_OUTPUT", ch, value)
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

    _FLAG_BITS = [
        "ext_rtc_flag",
        "flash_memory_flag",
        "lcd_display_flag",
        "sht_sensor_flag",
        "bme_sensor_flag",
        "ina226_flag",
        "adc_measurement_flag",
        "can_flag",
    ]

    def on_save_flags(self):
        flags = sum(
            (1 << i)
            for i, attr in enumerate(self._FLAG_BITS)
            if getattr(self.ui, attr).isChecked()
        )
        network = int(self.ui.network_enable_flag.isChecked())
        self._send("WRITE_FLAGS", hex(flags), network)

    def on_read_all_logs(self):
        if not self.device_manager.is_connected:
            self.log_message("Not connected. Connect first.")
            return
        if self._log_reading:
            self.log_message("Log reading already in progress.")
            return
        self._data_logs_model.removeRows(0, self._data_logs_model.rowCount())
        self._send("READ_LOG_COUNT")

    def _on_log_count_received(self, rest: str):
        try:
            self._log_total = int(rest.strip())
        except ValueError:
            self.log_message(f"Invalid log count: {rest}", level="ERROR")
            return
        self.ui.log_count_label.setText(f"Logs: {self._log_total}")
        if self._log_total == 0:
            self.log_message("No logs on device.")
            return
        self._log_index = 0
        self._log_reading = True
        self.ui.data_logs_progress.setRange(0, self._log_total)
        self.ui.data_logs_progress.setValue(0)
        self.ui.data_logs_progress.show()
        self.ui.read_all_logs_button.setEnabled(False)
        self._request_next_log()

    def _request_next_log(self):
        if self._log_index < self._log_total:
            self._log_timeout_timer.start()
            self._send("READ_LOG", self._log_index, 0)
        else:
            self._log_timeout_timer.stop()
            self._log_reading = False
            self.ui.data_logs_progress.hide()
            self.ui.read_all_logs_button.setEnabled(True)
            self.log_message(f"All {self._log_total} logs loaded.")
            self._update_chart()

    def _on_log_timeout(self):
        if not self._log_reading:
            return
        self.log_message(
            f"Log read timeout at index {self._log_index}, skipping.",
            level="ERROR",
        )
        self._log_index += 1
        self.ui.data_logs_progress.setValue(self._log_index)
        self._request_next_log()

    def _on_log_entry_received(self, rest: str):
        self._log_timeout_timer.stop()
        try:
            entry = ast.literal_eval(rest)
            dt = entry.get("datetime", {})
            iso = dt.get("iso", "") if isinstance(dt, dict) else str(dt)
            timestamp = (
                iso.replace("T", " ") if iso else str(entry.get("timestamp", ""))
            )

            row = [
                QStandardItem(str(entry.get("sequence", self._log_index))),
                QStandardItem(timestamp),
                QStandardItem(str(entry.get("temperature", ""))),
                QStandardItem(str(entry.get("humidity", ""))),
                QStandardItem(str(entry.get("pressure_hpa", ""))),
                QStandardItem(str(entry.get("crc32", ""))),
            ]
            for item in row:
                item.setFlags(item.flags() & ~Qt.ItemFlag.ItemIsEditable)
            self._data_logs_model.appendRow(row)

            self._log_index += 1
            self.ui.data_logs_progress.setValue(self._log_index)
        except Exception as e:
            self.log_message(f"Log parse error: {e}", level="ERROR")
            self._log_index += 1
        self._request_next_log()

    def on_export_logs_csv(self):
        if self._data_logs_model.rowCount() == 0:
            self.log_message("No data to export.")
            return
        file_name, _ = QFileDialog.getSaveFileName(
            self, "Export Logs", "", "CSV Files (*.csv);;All Files (*)"
        )
        if file_name:
            try:
                with open(file_name, "w", encoding="utf-8") as f:
                    headers = []
                    for col in range(self._data_logs_model.columnCount()):
                        headers.append(
                            self._data_logs_model.headerData(
                                col, Qt.Orientation.Horizontal
                            )
                        )
                    f.write(",".join(headers) + "\n")
                    for row_idx in range(self._data_logs_model.rowCount()):
                        cells = []
                        for col in range(self._data_logs_model.columnCount()):
                            item = self._data_logs_model.item(row_idx, col)
                            cells.append(item.text() if item else "")
                        f.write(",".join(cells) + "\n")
                self.log_message(f"Logs exported to {file_name}")
            except Exception as e:
                self.log_message(f"Error exporting logs: {e}", level="ERROR")

    def on_clear_logs_table(self):
        self._data_logs_model.removeRows(0, self._data_logs_model.rowCount())
        self.ui.log_count_label.setText("Logs: -")
        self._clear_chart()
        self.log_message("Logs table cleared.")

    def _update_chart(self):
        timestamps = []
        temperatures = []
        humidities = []
        pressures = []
        for row_idx in range(self._data_logs_model.rowCount()):
            ts_item = self._data_logs_model.item(row_idx, 1)
            temp_item = self._data_logs_model.item(row_idx, 2)
            hum_item = self._data_logs_model.item(row_idx, 3)
            pres_item = self._data_logs_model.item(row_idx, 4)
            try:
                timestamps.append(
                    datetime.fromisoformat(ts_item.text()) if ts_item else None
                )
            except (ValueError, AttributeError):
                timestamps.append(row_idx)
            try:
                temperatures.append(
                    float(temp_item.text()) if temp_item and temp_item.text() else None
                )
            except ValueError:
                temperatures.append(None)
            try:
                humidities.append(
                    float(hum_item.text()) if hum_item and hum_item.text() else None
                )
            except ValueError:
                humidities.append(None)
            try:
                pressures.append(
                    float(pres_item.text()) if pres_item and pres_item.text() else None
                )
            except ValueError:
                pressures.append(None)

        self._figure.clear()
        has_pressure = any(p is not None and p > 0 for p in pressures)

        x = list(range(len(timestamps)))
        x_labels = [
            t.strftime("%H:%M:%S") if isinstance(t, datetime) else str(t)
            for t in timestamps
        ]

        ax1 = self._figure.add_subplot(111)
        ax1.plot(
            x,
            temperatures,
            color="tab:red",
            marker=".",
            markersize=3,
            linewidth=1,
            label="Temperature [°C]",
        )
        ax1.set_ylabel("Temperature [°C] / Humidity [%]")
        ax1.plot(
            x,
            humidities,
            color="tab:blue",
            marker=".",
            markersize=3,
            linewidth=1,
            label="Humidity [%]",
        )

        if has_pressure:
            ax2 = ax1.twinx()
            ax2.plot(
                x,
                pressures,
                color="tab:green",
                marker=".",
                markersize=3,
                linewidth=1,
                label="Pressure [hPa]",
            )
            ax2.set_ylabel("Pressure [hPa]")
            lines2, labels2 = ax2.get_legend_handles_labels()
        else:
            lines2, labels2 = [], []

        ax1.grid(True, alpha=0.3)
        lines1, labels1 = ax1.get_legend_handles_labels()
        ax1.legend(lines1 + lines2, labels1 + labels2, loc="upper left", fontsize=7)

        step = max(1, len(x) // 10)
        ax1.set_xticks(x[::step])
        ax1.set_xticklabels(x_labels[::step], rotation=45, ha="right", fontsize=7)
        ax1.set_xlabel("Time")

        self._canvas.draw()

    def _clear_chart(self):
        self._figure.clear()
        self._canvas.draw()

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

    def _load_env(self):
        env_path = Path(__file__).parent.parent / ".env"
        self._env_secret = ""
        if env_path.exists():
            for line in env_path.read_text(encoding="utf-8").splitlines():
                line = line.strip()
                if line.startswith("SECRET"):
                    _, _, val = line.partition("=")
                    self._env_secret = val.strip().strip('"').strip("'")

    def _generate_daily_password(self, day=None):
        if day is None:
            day = date.today()
        ymd = day.strftime("%Y%m%d")
        raw = f"{self._env_secret}:{ymd}"
        return hashlib.sha256(raw.encode("utf-8")).hexdigest().upper()[:10]

    def on_keygen_generate(self):
        text = self.ui.keygen_date_input.text().strip()
        try:
            day = datetime.strptime(text, "%Y-%m-%d").date() if text else date.today()
        except ValueError:
            self.log_message("Invalid date format. Use YYYY-MM-DD.", level="ERROR")
            return
        key = self._generate_daily_password(day)
        self.ui.keygen_result_output.setText(key)
        self.log_message(f"Key generated for {day}")

    def _check_first_user(self):
        try:
            if not DatabaseConnection.has_users():
                QMessageBox.information(
                    self,
                    "First Run",
                    "No service users found. Please create the first user in the Service tab.",
                )
                self.ui.tabWidget.setCurrentWidget(self.ui.tab_service)
        except Exception as e:
            self.log_message(f"Database error: {e}", level="ERROR")

    def on_service_login(self):
        username = self.ui.service_username_input.text().strip()
        password = self.ui.service_password_input.text().strip()
        if not username or not password:
            self.log_message("Enter username and password.")
            return

        try:
            if not DatabaseConnection.has_users():
                DatabaseConnection.create_user(username, password)
                self.log_message(f"User '{username}' created.")
                self._set_service_logged_in(True)
                return

            if DatabaseConnection.verify_user(username, password):
                self.log_message(f"User '{username}' logged in.")
                self._set_service_logged_in(True)
            else:
                self.log_message("Invalid credentials.", level="ERROR")
        except Exception as e:
            self.log_message(f"Login error: {e}", level="ERROR")
        finally:
            self.ui.service_password_input.clear()

    def on_service_logout(self):
        self._set_service_logged_in(False)
        self.ui.service_username_input.clear()
        self.ui.keygen_date_input.clear()
        self.ui.keygen_result_output.clear()
        self.log_message("Service logout.")

    def _set_service_logged_in(self, logged_in: bool):
        self._service_logged_in = logged_in

        self.ui.service_status_label.setText(
            "Logged in" if logged_in else "Not logged in"
        )
        self.ui.service_login_button.setEnabled(not logged_in)
        self.ui.service_username_input.setEnabled(not logged_in)
        self.ui.service_password_input.setEnabled(not logged_in)
        self.ui.service_logout_button.setEnabled(logged_in)

        self.ui.keygen_date_input.setEnabled(logged_in)
        self.ui.keygen_generate_button.setEnabled(logged_in)
        self.ui.keygen_result_output.setEnabled(logged_in)

        self._set_service_controls_enabled(logged_in)

    _SERVICE_REQUIRED_BUTTONS = [
        "service_disable_button",
        "save_connection_data_button",
        "save_flags_button",
        "restore_flags_button",
        "restore_connection_data_button",
        "sync_time_button",
        "reset_button",
    ]

    _SERVICE_REQUIRED_INPUTS = [
        "set_logger_id_line",
        "set_sensor_id_line",
        "set_wifi_ssid_line",
        "set_wifi_password_line",
        "set_mqtt_server_line",
        "set_ntp_server_line",
        "set_mqtt_user_line",
        "set_mqtt_password_line",
    ]

    _SERVICE_REQUIRED_CHECKBOXES = [
        "ext_rtc_flag",
        "flash_memory_flag",
        "lcd_display_flag",
        "sht_sensor_flag",
        "bme_sensor_flag",
        "ina226_flag",
        "adc_measurement_flag",
        "can_flag",
        "network_enable_flag",
    ]

    def _set_service_controls_enabled(self, enabled: bool):
        for name in self._SERVICE_REQUIRED_BUTTONS:
            widget = getattr(self.ui, name, None)
            if widget:
                widget.setEnabled(enabled)
        for name in self._SERVICE_REQUIRED_INPUTS:
            widget = getattr(self.ui, name, None)
            if widget:
                widget.setEnabled(enabled)
        for name in self._SERVICE_REQUIRED_CHECKBOXES:
            widget = getattr(self.ui, name, None)
            if widget:
                widget.setEnabled(enabled)


if __name__ == "__main__":
    app = QApplication([])
    window = IoTLoggerApp()
    window.show()
    app.exec()
