from pathlib import Path


class ConfigInformation:
    VERSION = "1.0.0"
    BUILD_DATE = "2026-03-15"
    APP_NAME = "IoT Logger"

    DEFAULT_BAUDRATE = 115200
    DEFAULT_TIMEOUT = 1.0
    SERIAL_READ_TIMEOUT = 0.1

    WINDOW_WIDTH = 562
    WINDOW_HEIGHT = 498
    LOG_MAX_ENTRIES = 10000

    ICONS_DIR = Path(__file__).parent.parent / "icons"
    ICON_FILE = ICONS_DIR / "icon.png"
    DATA_DIR = Path(__file__).parent.parent / "data"
    CONFIG_FILE = DATA_DIR / "config.cfg"
    DATABASE_FILE = DATA_DIR / "db.sql"
