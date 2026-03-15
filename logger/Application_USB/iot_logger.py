import ctypes
import sys

from PySide6.QtWidgets import QApplication
from iot_logger_data.__main__ import IoTLoggerApp
from iot_logger_data.config.config import ConfigInformation as cfg

if __name__ == "__main__":
    if sys.platform == "win32":
        ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(
            f"iot_logger.{cfg.VERSION}"
        )
    app = QApplication([])
    window = IoTLoggerApp()
    window.show()
    app.exec()
