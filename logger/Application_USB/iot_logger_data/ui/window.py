from pathlib import Path
from PySide6.QtCore import (
    QCoreApplication,
    QDate,
    QDateTime,
    QLocale,
    QMetaObject,
    QObject,
    QPoint,
    QRect,
    QSize,
    QTime,
    QUrl,
    Qt,
)
from PySide6.QtGui import (
    QBrush,
    QColor,
    QConicalGradient,
    QCursor,
    QFont,
    QFontDatabase,
    QGradient,
    QIcon,
    QImage,
    QKeySequence,
    QLinearGradient,
    QPainter,
    QPalette,
    QPixmap,
    QRadialGradient,
    QTransform,
)
from PySide6.QtWidgets import (
    QAbstractScrollArea,
    QApplication,
    QCheckBox,
    QComboBox,
    QFrame,
    QGridLayout,
    QHeaderView,
    QLCDNumber,
    QLabel,
    QLineEdit,
    QListView,
    QMainWindow,
    QMenuBar,
    QProgressBar,
    QPushButton,
    QScrollArea,
    QSizePolicy,
    QSlider,
    QSpacerItem,
    QSplitter,
    QStatusBar,
    QTabWidget,
    QTableView,
    QTextBrowser,
    QToolBox,
    QVBoxLayout,
    QWidget,
)

_ICON_PATH = str(Path(__file__).parent.parent / "icons" / "icon.png")


class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName("MainWindow")
        MainWindow.resize(562, 498)
        icon = QIcon()
        if Path(_ICON_PATH).exists():
            icon.addFile(_ICON_PATH, QSize(), QIcon.Mode.Normal, QIcon.State.Off)
        MainWindow.setWindowIcon(icon)
        MainWindow.setAutoFillBackground(False)
        MainWindow.setDocumentMode(False)
        MainWindow.setDockNestingEnabled(False)
        MainWindow.setUnifiedTitleAndToolBarOnMac(False)
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout_3 = QGridLayout(self.centralwidget)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.tabWidget = QTabWidget(self.centralwidget)
        self.tabWidget.setObjectName("tabWidget")
        self.tab = QWidget()
        self.tab.setObjectName("tab")
        self.port_box = QComboBox(self.tab)
        self.port_box.setObjectName("port_box")
        self.port_box.setGeometry(QRect(21, 33, 171, 26))
        self.port_box.setEditable(False)
        self.connect_button = QPushButton(self.tab)
        self.connect_button.setObjectName("connect_button")
        self.connect_button.setGeometry(QRect(21, 65, 81, 26))
        self.disconnect_button = QPushButton(self.tab)
        self.disconnect_button.setObjectName("disconnect_button")
        self.disconnect_button.setGeometry(QRect(108, 65, 81, 26))
        self.refresh_port_button = QPushButton(self.tab)
        self.refresh_port_button.setObjectName("refresh_port_button")
        self.refresh_port_button.setGeometry(QRect(195, 33, 36, 26))
        icon1 = QIcon(QIcon.fromTheme(QIcon.ThemeIcon.ViewRefresh))
        self.refresh_port_button.setIcon(icon1)
        self.loaded_data_table_view = QTableView(self.tab)
        self.loaded_data_table_view.setObjectName("loaded_data_table_view")
        self.loaded_data_table_view.setGeometry(QRect(11, 207, 519, 183))
        self.line = QFrame(self.tab)
        self.line.setObjectName("line")
        self.line.setGeometry(QRect(0, 100, 531, 16))
        self.line.setFrameShape(QFrame.Shape.HLine)
        self.line.setFrameShadow(QFrame.Shadow.Sunken)
        self.ping_button = QPushButton(self.tab)
        self.ping_button.setObjectName("ping_button")
        self.ping_button.setGeometry(QRect(435, 11, 81, 26))
        self.reset_button = QPushButton(self.tab)
        self.reset_button.setObjectName("reset_button")
        self.reset_button.setGeometry(QRect(435, 43, 81, 26))
        self.sync_time_button = QPushButton(self.tab)
        self.sync_time_button.setObjectName("sync_time_button")
        self.sync_time_button.setGeometry(QRect(348, 11, 81, 26))
        self.read_time_button = QPushButton(self.tab)
        self.read_time_button.setObjectName("read_time_button")
        self.read_time_button.setGeometry(QRect(348, 43, 81, 26))
        self.current_time_text = QTextBrowser(self.tab)
        self.current_time_text.setObjectName("current_time_text")
        self.current_time_text.setEnabled(True)
        self.current_time_text.setGeometry(QRect(260, 80, 251, 21))
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(
            self.current_time_text.sizePolicy().hasHeightForWidth()
        )
        self.current_time_text.setSizePolicy(sizePolicy)
        self.current_time_text.setMouseTracking(True)
        self.current_time_text.setAcceptDrops(True)
        self.current_time_text.setFrameShape(QFrame.Shape.StyledPanel)
        self.current_time_text.setVerticalScrollBarPolicy(
            Qt.ScrollBarPolicy.ScrollBarAlwaysOff
        )
        self.current_time_text.setHorizontalScrollBarPolicy(
            Qt.ScrollBarPolicy.ScrollBarAlwaysOff
        )
        self.current_time_text.setSizeAdjustPolicy(
            QAbstractScrollArea.SizeAdjustPolicy.AdjustToContents
        )
        self.read_data_button = QPushButton(self.tab)
        self.read_data_button.setObjectName("read_data_button")
        self.read_data_button.setGeometry(QRect(261, 11, 81, 26))
        self.read_flags_button = QPushButton(self.tab)
        self.read_flags_button.setObjectName("read_flags_button")
        self.read_flags_button.setGeometry(QRect(261, 43, 81, 26))
        self.select_com_label = QLabel(self.tab)
        self.select_com_label.setObjectName("select_com_label")
        self.select_com_label.setGeometry(QRect(21, 11, 62, 16))
        self.service_enable_button = QPushButton(self.tab)
        self.service_enable_button.setObjectName("service_enable_button")
        self.service_enable_button.setGeometry(QRect(260, 150, 81, 26))
        self.service_disable_button = QPushButton(self.tab)
        self.service_disable_button.setObjectName("service_disable_button")
        self.service_disable_button.setGeometry(QRect(346, 150, 81, 26))
        self.service_password = QLineEdit(self.tab)
        self.service_password.setObjectName("service_password")
        self.service_password.setGeometry(QRect(20, 150, 231, 26))
        sizePolicy.setHeightForWidth(
            self.service_password.sizePolicy().hasHeightForWidth()
        )
        self.service_password.setSizePolicy(sizePolicy)
        self.service_model_label = QLabel(self.tab)
        self.service_model_label.setObjectName("service_model_label")
        self.service_model_label.setGeometry(QRect(11, 121, 104, 21))
        font = QFont()
        font.setPointSize(12)
        font.setBold(True)
        self.service_model_label.setFont(font)
        self.data_label = QLabel(self.tab)
        self.data_label.setObjectName("data_label")
        self.data_label.setGeometry(QRect(11, 180, 36, 21))
        self.data_label.setFont(font)
        self.tabWidget.addTab(self.tab, "")
        self.tab_4 = QWidget()
        self.tab_4.setObjectName("tab_4")
        self.gridLayout_5 = QGridLayout(self.tab_4)
        self.gridLayout_5.setObjectName("gridLayout_5")
        self.scrollArea = QScrollArea(self.tab_4)
        self.scrollArea.setObjectName("scrollArea")
        self.scrollArea.setWidgetResizable(True)
        self.scrollAreaWidgetContents = QWidget()
        self.scrollAreaWidgetContents.setObjectName("scrollAreaWidgetContents")
        self.scrollAreaWidgetContents.setGeometry(QRect(0, -278, 504, 653))
        self.gridLayout_6 = QGridLayout(self.scrollAreaWidgetContents)
        self.gridLayout_6.setObjectName("gridLayout_6")
        self.output_off_button = QPushButton(self.scrollAreaWidgetContents)
        self.output_off_button.setObjectName("output_off_button")

        self.gridLayout_6.addWidget(self.output_off_button, 13, 4, 1, 1)

        self.line_7 = QFrame(self.scrollAreaWidgetContents)
        self.line_7.setObjectName("line_7")
        self.line_7.setFrameShape(QFrame.Shape.HLine)
        self.line_7.setFrameShadow(QFrame.Shadow.Sunken)

        self.gridLayout_6.addWidget(self.line_7, 14, 0, 1, 6)

        self.outputs_label = QLabel(self.scrollAreaWidgetContents)
        self.outputs_label.setObjectName("outputs_label")
        self.outputs_label.setFont(font)

        self.gridLayout_6.addWidget(self.outputs_label, 11, 0, 1, 1)

        self.volume_label = QLabel(self.scrollAreaWidgetContents)
        self.volume_label.setObjectName("volume_label")
        sizePolicy.setHeightForWidth(self.volume_label.sizePolicy().hasHeightForWidth())
        self.volume_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.volume_label, 27, 2, 1, 1)

        self.set_pwm_button = QPushButton(self.scrollAreaWidgetContents)
        self.set_pwm_button.setObjectName("set_pwm_button")

        self.gridLayout_6.addWidget(self.set_pwm_button, 18, 4, 1, 1)

        self.duty_cycle_label = QLabel(self.scrollAreaWidgetContents)
        self.duty_cycle_label.setObjectName("duty_cycle_label")
        sizePolicy.setHeightForWidth(
            self.duty_cycle_label.sizePolicy().hasHeightForWidth()
        )
        self.duty_cycle_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.duty_cycle_label, 17, 3, 1, 1)

        self.rgb_label = QLabel(self.scrollAreaWidgetContents)
        self.rgb_label.setObjectName("rgb_label")
        sizePolicy.setHeightForWidth(self.rgb_label.sizePolicy().hasHeightForWidth())
        self.rgb_label.setSizePolicy(sizePolicy)
        self.rgb_label.setFont(font)

        self.gridLayout_6.addWidget(self.rgb_label, 21, 0, 1, 1)

        self.read_sht_button = QPushButton(self.scrollAreaWidgetContents)
        self.read_sht_button.setObjectName("read_sht_button")

        self.gridLayout_6.addWidget(self.read_sht_button, 2, 4, 1, 1)

        self.set_duty_cycle_slider = QSlider(self.scrollAreaWidgetContents)
        self.set_duty_cycle_slider.setObjectName("set_duty_cycle_slider")
        sizePolicy.setHeightForWidth(
            self.set_duty_cycle_slider.sizePolicy().hasHeightForWidth()
        )
        self.set_duty_cycle_slider.setSizePolicy(sizePolicy)
        self.set_duty_cycle_slider.setOrientation(Qt.Orientation.Horizontal)

        self.gridLayout_6.addWidget(self.set_duty_cycle_slider, 18, 3, 1, 1)

        self.select_output_box = QComboBox(self.scrollAreaWidgetContents)
        self.select_output_box.setObjectName("select_output_box")
        sizePolicy.setHeightForWidth(
            self.select_output_box.sizePolicy().hasHeightForWidth()
        )
        self.select_output_box.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.select_output_box, 13, 0, 1, 1)

        self.horizontalSpacer_9 = QSpacerItem(
            526, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum
        )

        self.gridLayout_6.addItem(self.horizontalSpacer_9, 15, 0, 1, 6)

        self.bme_temperature = QLCDNumber(self.scrollAreaWidgetContents)
        self.bme_temperature.setObjectName("bme_temperature")
        sizePolicy.setHeightForWidth(
            self.bme_temperature.sizePolicy().hasHeightForWidth()
        )
        self.bme_temperature.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.bme_temperature, 7, 0, 1, 1)

        self.sht_humidity_label = QLabel(self.scrollAreaWidgetContents)
        self.sht_humidity_label.setObjectName("sht_humidity_label")
        sizePolicy.setHeightForWidth(
            self.sht_humidity_label.sizePolicy().hasHeightForWidth()
        )
        self.sht_humidity_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.sht_humidity_label, 1, 1, 1, 2)

        self.horizontalSpacer_11 = QSpacerItem(
            476, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum
        )

        self.gridLayout_6.addItem(self.horizontalSpacer_11, 25, 0, 1, 6)

        self.line_6 = QFrame(self.scrollAreaWidgetContents)
        self.line_6.setObjectName("line_6")
        self.line_6.setFrameShape(QFrame.Shape.HLine)
        self.line_6.setFrameShadow(QFrame.Shadow.Sunken)

        self.gridLayout_6.addWidget(self.line_6, 9, 0, 1, 6)

        self.current_color = QLabel(self.scrollAreaWidgetContents)
        self.current_color.setObjectName("current_color")

        self.gridLayout_6.addWidget(self.current_color, 23, 4, 1, 1)

        self.bme_humidity_label = QLabel(self.scrollAreaWidgetContents)
        self.bme_humidity_label.setObjectName("bme_humidity_label")
        sizePolicy.setHeightForWidth(
            self.bme_humidity_label.sizePolicy().hasHeightForWidth()
        )
        self.bme_humidity_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.bme_humidity_label, 6, 1, 1, 1)

        self.red_label = QLabel(self.scrollAreaWidgetContents)
        self.red_label.setObjectName("red_label")
        sizePolicy.setHeightForWidth(self.red_label.sizePolicy().hasHeightForWidth())
        self.red_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.red_label, 22, 0, 1, 1)

        self.green_label = QLabel(self.scrollAreaWidgetContents)
        self.green_label.setObjectName("green_label")
        sizePolicy.setHeightForWidth(self.green_label.sizePolicy().hasHeightForWidth())
        self.green_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.green_label, 22, 1, 1, 1)

        self.output_select_label = QLabel(self.scrollAreaWidgetContents)
        self.output_select_label.setObjectName("output_select_label")
        sizePolicy.setHeightForWidth(
            self.output_select_label.sizePolicy().hasHeightForWidth()
        )
        self.output_select_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.output_select_label, 12, 0, 1, 1)

        self.line_8 = QFrame(self.scrollAreaWidgetContents)
        self.line_8.setObjectName("line_8")
        self.line_8.setFrameShape(QFrame.Shape.HLine)
        self.line_8.setFrameShadow(QFrame.Shadow.Sunken)

        self.gridLayout_6.addWidget(self.line_8, 19, 0, 1, 6)

        self.select_pwm_box = QComboBox(self.scrollAreaWidgetContents)
        self.select_pwm_box.setObjectName("select_pwm_box")
        sizePolicy.setHeightForWidth(
            self.select_pwm_box.sizePolicy().hasHeightForWidth()
        )
        self.select_pwm_box.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.select_pwm_box, 18, 0, 1, 1)

        self.set_volume_slider = QSlider(self.scrollAreaWidgetContents)
        self.set_volume_slider.setObjectName("set_volume_slider")
        sizePolicy.setHeightForWidth(
            self.set_volume_slider.sizePolicy().hasHeightForWidth()
        )
        self.set_volume_slider.setSizePolicy(sizePolicy)
        self.set_volume_slider.setOrientation(Qt.Orientation.Horizontal)

        self.gridLayout_6.addWidget(self.set_volume_slider, 28, 2, 1, 1)

        self.line_2 = QFrame(self.scrollAreaWidgetContents)
        self.line_2.setObjectName("line_2")
        self.line_2.setFrameShape(QFrame.Shape.HLine)
        self.line_2.setFrameShadow(QFrame.Shadow.Sunken)

        self.gridLayout_6.addWidget(self.line_2, 3, 0, 1, 6)

        self.horizontalSpacer_2 = QSpacerItem(
            337, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum
        )

        self.gridLayout_6.addItem(self.horizontalSpacer_2, 10, 0, 1, 6)

        self.frequency_label = QLabel(self.scrollAreaWidgetContents)
        self.frequency_label.setObjectName("frequency_label")
        sizePolicy.setHeightForWidth(
            self.frequency_label.sizePolicy().hasHeightForWidth()
        )
        self.frequency_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.frequency_label, 27, 0, 1, 1)

        self.bme_label = QLabel(self.scrollAreaWidgetContents)
        self.bme_label.setObjectName("bme_label")
        self.bme_label.setFont(font)

        self.gridLayout_6.addWidget(self.bme_label, 5, 0, 1, 2)

        self.bme_temperature_label = QLabel(self.scrollAreaWidgetContents)
        self.bme_temperature_label.setObjectName("bme_temperature_label")
        sizePolicy.setHeightForWidth(
            self.bme_temperature_label.sizePolicy().hasHeightForWidth()
        )
        self.bme_temperature_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.bme_temperature_label, 6, 0, 1, 1)

        self.sht_temperature = QLCDNumber(self.scrollAreaWidgetContents)
        self.sht_temperature.setObjectName("sht_temperature")
        sizePolicy.setHeightForWidth(
            self.sht_temperature.sizePolicy().hasHeightForWidth()
        )
        self.sht_temperature.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.sht_temperature, 2, 0, 1, 1)

        self.set_red_slider = QSlider(self.scrollAreaWidgetContents)
        self.set_red_slider.setObjectName("set_red_slider")
        sizePolicy.setHeightForWidth(
            self.set_red_slider.sizePolicy().hasHeightForWidth()
        )
        self.set_red_slider.setSizePolicy(sizePolicy)
        self.set_red_slider.setOrientation(Qt.Orientation.Horizontal)

        self.gridLayout_6.addWidget(self.set_red_slider, 23, 0, 1, 1)

        self.set_frequency_slider = QSlider(self.scrollAreaWidgetContents)
        self.set_frequency_slider.setObjectName("set_frequency_slider")
        sizePolicy.setHeightForWidth(
            self.set_frequency_slider.sizePolicy().hasHeightForWidth()
        )
        self.set_frequency_slider.setSizePolicy(sizePolicy)
        self.set_frequency_slider.setOrientation(Qt.Orientation.Horizontal)

        self.gridLayout_6.addWidget(self.set_frequency_slider, 28, 0, 1, 1)

        self.set_blue_slider = QSlider(self.scrollAreaWidgetContents)
        self.set_blue_slider.setObjectName("set_blue_slider")
        sizePolicy.setHeightForWidth(
            self.set_blue_slider.sizePolicy().hasHeightForWidth()
        )
        self.set_blue_slider.setSizePolicy(sizePolicy)
        self.set_blue_slider.setOrientation(Qt.Orientation.Horizontal)

        self.gridLayout_6.addWidget(self.set_blue_slider, 23, 2, 1, 1)

        self.sht_humidity = QLCDNumber(self.scrollAreaWidgetContents)
        self.sht_humidity.setObjectName("sht_humidity")
        sizePolicy.setHeightForWidth(self.sht_humidity.sizePolicy().hasHeightForWidth())
        self.sht_humidity.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.sht_humidity, 2, 1, 1, 2)

        self.set_green_slider = QSlider(self.scrollAreaWidgetContents)
        self.set_green_slider.setObjectName("set_green_slider")
        sizePolicy.setHeightForWidth(
            self.set_green_slider.sizePolicy().hasHeightForWidth()
        )
        self.set_green_slider.setSizePolicy(sizePolicy)
        self.set_green_slider.setOrientation(Qt.Orientation.Horizontal)

        self.gridLayout_6.addWidget(self.set_green_slider, 23, 1, 1, 1)

        self.pwm_select_label = QLabel(self.scrollAreaWidgetContents)
        self.pwm_select_label.setObjectName("pwm_select_label")

        self.gridLayout_6.addWidget(self.pwm_select_label, 17, 0, 1, 1)

        self.line_9 = QFrame(self.scrollAreaWidgetContents)
        self.line_9.setObjectName("line_9")
        self.line_9.setFrameShape(QFrame.Shape.HLine)
        self.line_9.setFrameShadow(QFrame.Shadow.Sunken)

        self.gridLayout_6.addWidget(self.line_9, 24, 0, 1, 6)

        self.output_on_button = QPushButton(self.scrollAreaWidgetContents)
        self.output_on_button.setObjectName("output_on_button")

        self.gridLayout_6.addWidget(self.output_on_button, 13, 3, 1, 1)

        self.horizontalSpacer_10 = QSpacerItem(
            526, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum
        )

        self.gridLayout_6.addItem(self.horizontalSpacer_10, 20, 0, 1, 6)

        self.blue_label = QLabel(self.scrollAreaWidgetContents)
        self.blue_label.setObjectName("blue_label")
        sizePolicy.setHeightForWidth(self.blue_label.sizePolicy().hasHeightForWidth())
        self.blue_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.blue_label, 22, 2, 1, 1)

        self.color_label = QLabel(self.scrollAreaWidgetContents)
        self.color_label.setObjectName("color_label")
        sizePolicy.setHeightForWidth(self.color_label.sizePolicy().hasHeightForWidth())
        self.color_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.color_label, 22, 4, 1, 1)

        self.pwm_label = QLabel(self.scrollAreaWidgetContents)
        self.pwm_label.setObjectName("pwm_label")
        self.pwm_label.setFont(font)

        self.gridLayout_6.addWidget(self.pwm_label, 16, 0, 1, 1)

        self.sht_label = QLabel(self.scrollAreaWidgetContents)
        self.sht_label.setObjectName("sht_label")
        self.sht_label.setFont(font)

        self.gridLayout_6.addWidget(self.sht_label, 0, 0, 1, 2)

        self.bme_pressure_label = QLabel(self.scrollAreaWidgetContents)
        self.bme_pressure_label.setObjectName("bme_pressure_label")
        sizePolicy.setHeightForWidth(
            self.bme_pressure_label.sizePolicy().hasHeightForWidth()
        )
        self.bme_pressure_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.bme_pressure_label, 6, 3, 1, 1)

        self.buzzer_label = QLabel(self.scrollAreaWidgetContents)
        self.buzzer_label.setObjectName("buzzer_label")
        sizePolicy.setHeightForWidth(self.buzzer_label.sizePolicy().hasHeightForWidth())
        self.buzzer_label.setSizePolicy(sizePolicy)
        self.buzzer_label.setFont(font)

        self.gridLayout_6.addWidget(self.buzzer_label, 26, 0, 1, 1)

        self.set_brightness_slider = QSlider(self.scrollAreaWidgetContents)
        self.set_brightness_slider.setObjectName("set_brightness_slider")
        sizePolicy.setHeightForWidth(
            self.set_brightness_slider.sizePolicy().hasHeightForWidth()
        )
        self.set_brightness_slider.setSizePolicy(sizePolicy)
        self.set_brightness_slider.setOrientation(Qt.Orientation.Horizontal)

        self.gridLayout_6.addWidget(self.set_brightness_slider, 23, 3, 1, 1)

        self.read_bme_button = QPushButton(self.scrollAreaWidgetContents)
        self.read_bme_button.setObjectName("read_bme_button")

        self.gridLayout_6.addWidget(self.read_bme_button, 7, 4, 1, 1)

        self.brightness_label = QLabel(self.scrollAreaWidgetContents)
        self.brightness_label.setObjectName("brightness_label")
        sizePolicy.setHeightForWidth(
            self.brightness_label.sizePolicy().hasHeightForWidth()
        )
        self.brightness_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.brightness_label, 22, 3, 1, 1)

        self.horizontalSpacer = QSpacerItem(
            520, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum
        )

        self.gridLayout_6.addItem(self.horizontalSpacer, 4, 0, 1, 6)

        self.bme_humidity = QLCDNumber(self.scrollAreaWidgetContents)
        self.bme_humidity.setObjectName("bme_humidity")
        sizePolicy.setHeightForWidth(self.bme_humidity.sizePolicy().hasHeightForWidth())
        self.bme_humidity.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.bme_humidity, 7, 1, 1, 2)

        self.sht_temperature_label = QLabel(self.scrollAreaWidgetContents)
        self.sht_temperature_label.setObjectName("sht_temperature_label")
        sizePolicy.setHeightForWidth(
            self.sht_temperature_label.sizePolicy().hasHeightForWidth()
        )
        self.sht_temperature_label.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.sht_temperature_label, 1, 0, 1, 1)

        self.bme_pressure = QLCDNumber(self.scrollAreaWidgetContents)
        self.bme_pressure.setObjectName("bme_pressure")
        sizePolicy.setHeightForWidth(self.bme_pressure.sizePolicy().hasHeightForWidth())
        self.bme_pressure.setSizePolicy(sizePolicy)

        self.gridLayout_6.addWidget(self.bme_pressure, 7, 3, 1, 1)

        self.scrollArea.setWidget(self.scrollAreaWidgetContents)

        self.gridLayout_5.addWidget(self.scrollArea, 0, 0, 1, 1)

        self.tabWidget.addTab(self.tab_4, "")
        self.tab_data_logs = QWidget()
        self.tab_data_logs.setObjectName("tab_data_logs")
        self.gridLayout_data_logs = QGridLayout(self.tab_data_logs)
        self.gridLayout_data_logs.setObjectName("gridLayout_data_logs")

        self.read_all_logs_button = QPushButton(self.tab_data_logs)
        self.read_all_logs_button.setObjectName("read_all_logs_button")
        self.gridLayout_data_logs.addWidget(self.read_all_logs_button, 0, 0, 1, 1)

        self.export_logs_button = QPushButton(self.tab_data_logs)
        self.export_logs_button.setObjectName("export_logs_button")
        self.gridLayout_data_logs.addWidget(self.export_logs_button, 0, 1, 1, 1)

        self.clear_logs_table_button = QPushButton(self.tab_data_logs)
        self.clear_logs_table_button.setObjectName("clear_logs_table_button")
        self.gridLayout_data_logs.addWidget(self.clear_logs_table_button, 0, 2, 1, 1)

        self.logs_spacer = QSpacerItem(
            40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum
        )
        self.gridLayout_data_logs.addItem(self.logs_spacer, 0, 3, 1, 1)

        self.log_count_label = QLabel(self.tab_data_logs)
        self.log_count_label.setObjectName("log_count_label")
        self.gridLayout_data_logs.addWidget(self.log_count_label, 0, 4, 1, 1)

        self.data_logs_progress = QProgressBar(self.tab_data_logs)
        self.data_logs_progress.setObjectName("data_logs_progress")
        self.data_logs_progress.setValue(0)
        self.data_logs_progress.setTextVisible(True)
        self.data_logs_progress.hide()
        self.gridLayout_data_logs.addWidget(self.data_logs_progress, 1, 0, 1, 5)

        self.data_logs_table = QTableView(self.tab_data_logs)
        self.data_logs_table.setObjectName("data_logs_table")
        self.gridLayout_data_logs.addWidget(self.data_logs_table, 2, 0, 1, 5)

        self.tabWidget.addTab(self.tab_data_logs, "")
        self.tab_chart = QWidget()
        self.tab_chart.setObjectName("tab_chart")
        self.chart_layout = QVBoxLayout(self.tab_chart)
        self.chart_layout.setObjectName("chart_layout")
        self.tabWidget.addTab(self.tab_chart, "")
        self.tab_3 = QWidget()
        self.tab_3.setObjectName("tab_3")
        self.gridLayout_2 = QGridLayout(self.tab_3)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.tool_box = QToolBox(self.tab_3)
        self.tool_box.setObjectName("tool_box")
        self.page = QWidget()
        self.page.setObjectName("page")
        self.page.setGeometry(QRect(0, 0, 508, 597))
        self.gridLayout = QGridLayout(self.page)
        self.gridLayout.setObjectName("gridLayout")
        self.logger_data_label = QLabel(self.page)
        self.logger_data_label.setObjectName("logger_data_label")
        self.logger_data_label.setFont(font)

        self.gridLayout.addWidget(self.logger_data_label, 0, 0, 1, 1)

        self.logger_id_label = QLabel(self.page)
        self.logger_id_label.setObjectName("logger_id_label")

        self.gridLayout.addWidget(self.logger_id_label, 1, 0, 1, 1)

        self.set_logger_id_line = QLineEdit(self.page)
        self.set_logger_id_line.setObjectName("set_logger_id_line")

        self.gridLayout.addWidget(self.set_logger_id_line, 1, 1, 1, 1)

        self.sensor_id_label = QLabel(self.page)
        self.sensor_id_label.setObjectName("sensor_id_label")

        self.gridLayout.addWidget(self.sensor_id_label, 2, 0, 1, 1)

        self.set_sensor_id_line = QLineEdit(self.page)
        self.set_sensor_id_line.setObjectName("set_sensor_id_line")

        self.gridLayout.addWidget(self.set_sensor_id_line, 2, 1, 1, 1)

        self.line_3 = QFrame(self.page)
        self.line_3.setObjectName("line_3")
        self.line_3.setFrameShape(QFrame.Shape.HLine)
        self.line_3.setFrameShadow(QFrame.Shadow.Sunken)

        self.gridLayout.addWidget(self.line_3, 3, 0, 1, 2)

        self.horizontalSpacer_3 = QSpacerItem(
            479, 48, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum
        )

        self.gridLayout.addItem(self.horizontalSpacer_3, 4, 0, 1, 2)

        self.wifi_settings_label = QLabel(self.page)
        self.wifi_settings_label.setObjectName("wifi_settings_label")
        self.wifi_settings_label.setFont(font)

        self.gridLayout.addWidget(self.wifi_settings_label, 5, 0, 1, 1)

        self.ssid_label = QLabel(self.page)
        self.ssid_label.setObjectName("ssid_label")

        self.gridLayout.addWidget(self.ssid_label, 6, 0, 1, 1)

        self.set_wifi_ssid_line = QLineEdit(self.page)
        self.set_wifi_ssid_line.setObjectName("set_wifi_ssid_line")

        self.gridLayout.addWidget(self.set_wifi_ssid_line, 6, 1, 1, 1)

        self.password_label = QLabel(self.page)
        self.password_label.setObjectName("password_label")

        self.gridLayout.addWidget(self.password_label, 7, 0, 1, 1)

        self.set_wifi_password_line = QLineEdit(self.page)
        self.set_wifi_password_line.setObjectName("set_wifi_password_line")

        self.gridLayout.addWidget(self.set_wifi_password_line, 7, 1, 1, 1)

        self.line_4 = QFrame(self.page)
        self.line_4.setObjectName("line_4")
        self.line_4.setFrameShape(QFrame.Shape.HLine)
        self.line_4.setFrameShadow(QFrame.Shadow.Sunken)

        self.gridLayout.addWidget(self.line_4, 8, 0, 1, 2)

        self.horizontalSpacer_5 = QSpacerItem(
            480, 48, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum
        )

        self.gridLayout.addItem(self.horizontalSpacer_5, 9, 0, 1, 2)

        self.ntp_server_label = QLabel(self.page)
        self.ntp_server_label.setObjectName("ntp_server_label")
        self.ntp_server_label.setFont(font)

        self.gridLayout.addWidget(self.ntp_server_label, 10, 0, 1, 1)

        self.ntp_server_ip_label = QLabel(self.page)
        self.ntp_server_ip_label.setObjectName("ntp_server_ip_label")

        self.gridLayout.addWidget(self.ntp_server_ip_label, 11, 0, 1, 1)

        self.set_ntp_server_line = QLineEdit(self.page)
        self.set_ntp_server_line.setObjectName("set_ntp_server_line")

        self.gridLayout.addWidget(self.set_ntp_server_line, 11, 1, 1, 1)

        self.line_5 = QFrame(self.page)
        self.line_5.setObjectName("line_5")
        self.line_5.setFrameShape(QFrame.Shape.HLine)
        self.line_5.setFrameShadow(QFrame.Shadow.Sunken)

        self.gridLayout.addWidget(self.line_5, 12, 0, 1, 2)

        self.horizontalSpacer_6 = QSpacerItem(
            518, 48, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum
        )

        self.gridLayout.addItem(self.horizontalSpacer_6, 13, 0, 1, 2)

        self.mqtt_settings_label = QLabel(self.page)
        self.mqtt_settings_label.setObjectName("mqtt_settings_label")
        self.mqtt_settings_label.setFont(font)

        self.gridLayout.addWidget(self.mqtt_settings_label, 14, 0, 1, 1)

        self.mqtt_server_label = QLabel(self.page)
        self.mqtt_server_label.setObjectName("mqtt_server_label")

        self.gridLayout.addWidget(self.mqtt_server_label, 15, 0, 1, 1)

        self.set_mqtt_server_line = QLineEdit(self.page)
        self.set_mqtt_server_line.setObjectName("set_mqtt_server_line")

        self.gridLayout.addWidget(self.set_mqtt_server_line, 15, 1, 1, 1)

        self.mqtt_user_label = QLabel(self.page)
        self.mqtt_user_label.setObjectName("mqtt_user_label")

        self.gridLayout.addWidget(self.mqtt_user_label, 16, 0, 1, 1)

        self.set_mqtt_user_line = QLineEdit(self.page)
        self.set_mqtt_user_line.setObjectName("set_mqtt_user_line")

        self.gridLayout.addWidget(self.set_mqtt_user_line, 16, 1, 1, 1)

        self.mqtt_password = QLabel(self.page)
        self.mqtt_password.setObjectName("mqtt_password")

        self.gridLayout.addWidget(self.mqtt_password, 17, 0, 1, 1)

        self.set_mqtt_password_line = QLineEdit(self.page)
        self.set_mqtt_password_line.setObjectName("set_mqtt_password_line")

        self.gridLayout.addWidget(self.set_mqtt_password_line, 17, 1, 1, 1)

        self.splitter = QSplitter(self.page)
        self.splitter.setObjectName("splitter")
        self.splitter.setOrientation(Qt.Orientation.Horizontal)
        self.save_connection_data_button = QPushButton(self.splitter)
        self.save_connection_data_button.setObjectName("save_connection_data_button")
        self.splitter.addWidget(self.save_connection_data_button)
        self.restore_connection_data_button = QPushButton(self.splitter)
        self.restore_connection_data_button.setObjectName(
            "restore_connection_data_button"
        )
        self.splitter.addWidget(self.restore_connection_data_button)

        self.gridLayout.addWidget(self.splitter, 18, 0, 1, 2)

        self.tool_box.addItem(self.page, "Connection data")
        self.page_2 = QWidget()
        self.page_2.setObjectName("page_2")
        self.page_2.setGeometry(QRect(0, 0, 520, 319))
        self.gridLayout_4 = QGridLayout(self.page_2)
        self.gridLayout_4.setObjectName("gridLayout_4")
        self.ext_rtc_flag = QCheckBox(self.page_2)
        self.ext_rtc_flag.setObjectName("ext_rtc_flag")

        self.gridLayout_4.addWidget(self.ext_rtc_flag, 0, 0, 1, 1)

        self.flash_memory_flag = QCheckBox(self.page_2)
        self.flash_memory_flag.setObjectName("flash_memory_flag")

        self.gridLayout_4.addWidget(self.flash_memory_flag, 1, 0, 1, 1)

        self.lcd_display_flag = QCheckBox(self.page_2)
        self.lcd_display_flag.setObjectName("lcd_display_flag")

        self.gridLayout_4.addWidget(self.lcd_display_flag, 2, 0, 1, 1)

        self.sht_sensor_flag = QCheckBox(self.page_2)
        self.sht_sensor_flag.setObjectName("sht_sensor_flag")

        self.gridLayout_4.addWidget(self.sht_sensor_flag, 3, 0, 1, 1)

        self.bme_sensor_flag = QCheckBox(self.page_2)
        self.bme_sensor_flag.setObjectName("bme_sensor_flag")

        self.gridLayout_4.addWidget(self.bme_sensor_flag, 4, 0, 1, 1)

        self.ina226_flag = QCheckBox(self.page_2)
        self.ina226_flag.setObjectName("ina226_flag")

        self.gridLayout_4.addWidget(self.ina226_flag, 5, 0, 1, 1)

        self.adc_measurement_flag = QCheckBox(self.page_2)
        self.adc_measurement_flag.setObjectName("adc_measurement_flag")

        self.gridLayout_4.addWidget(self.adc_measurement_flag, 6, 0, 1, 2)

        self.can_flag = QCheckBox(self.page_2)
        self.can_flag.setObjectName("can_flag")

        self.gridLayout_4.addWidget(self.can_flag, 7, 0, 1, 2)

        self.network_enable_flag = QCheckBox(self.page_2)
        self.network_enable_flag.setObjectName("network_enable_flag")

        self.gridLayout_4.addWidget(self.network_enable_flag, 8, 0, 1, 2)

        self.save_flags_button = QPushButton(self.page_2)
        self.save_flags_button.setObjectName("save_flags_button")

        self.gridLayout_4.addWidget(self.save_flags_button, 9, 0, 1, 1)

        self.restore_flags_button = QPushButton(self.page_2)
        self.restore_flags_button.setObjectName("restore_flags_button")

        self.gridLayout_4.addWidget(self.restore_flags_button, 9, 1, 1, 1)

        self.tool_box.addItem(self.page_2, "Flags")

        self.gridLayout_2.addWidget(self.tool_box, 0, 1, 1, 1)

        self.tabWidget.addTab(self.tab_3, "")
        self.tab_2 = QWidget()
        self.tab_2.setObjectName("tab_2")
        self.log_view = QListView(self.tab_2)
        self.log_view.setObjectName("log_view")
        self.log_view.setGeometry(QRect(9, 9, 511, 331))
        self.save_log_button = QPushButton(self.tab_2)
        self.save_log_button.setObjectName("save_log_button")
        self.save_log_button.setGeometry(QRect(350, 350, 78, 26))
        self.clear_log_button = QPushButton(self.tab_2)
        self.clear_log_button.setObjectName("clear_log_button")
        self.clear_log_button.setGeometry(QRect(433, 350, 78, 26))
        self.tabWidget.addTab(self.tab_2, "")
        self.tab_service = QWidget()
        self.tab_service.setObjectName("tab_service")

        self.service_login_label = QLabel(self.tab_service)
        self.service_login_label.setObjectName("service_login_label")
        self.service_login_label.setGeometry(QRect(11, 11, 200, 21))
        self.service_login_label.setFont(font)

        self.service_username_label = QLabel(self.tab_service)
        self.service_username_label.setObjectName("service_username_label")
        self.service_username_label.setGeometry(QRect(11, 40, 80, 26))

        self.service_username_input = QLineEdit(self.tab_service)
        self.service_username_input.setObjectName("service_username_input")
        self.service_username_input.setGeometry(QRect(95, 40, 200, 26))

        self.service_password_label = QLabel(self.tab_service)
        self.service_password_label.setObjectName("service_password_label")
        self.service_password_label.setGeometry(QRect(11, 72, 80, 26))

        self.service_password_input = QLineEdit(self.tab_service)
        self.service_password_input.setObjectName("service_password_input")
        self.service_password_input.setGeometry(QRect(95, 72, 200, 26))
        self.service_password_input.setEchoMode(QLineEdit.EchoMode.Password)

        self.service_login_button = QPushButton(self.tab_service)
        self.service_login_button.setObjectName("service_login_button")
        self.service_login_button.setGeometry(QRect(305, 40, 90, 26))

        self.service_logout_button = QPushButton(self.tab_service)
        self.service_logout_button.setObjectName("service_logout_button")
        self.service_logout_button.setGeometry(QRect(305, 72, 90, 26))

        self.service_status_label = QLabel(self.tab_service)
        self.service_status_label.setObjectName("service_status_label")
        self.service_status_label.setGeometry(QRect(405, 40, 120, 26))

        self.line_service = QFrame(self.tab_service)
        self.line_service.setObjectName("line_service")
        self.line_service.setGeometry(QRect(0, 105, 531, 16))
        self.line_service.setFrameShape(QFrame.Shape.HLine)
        self.line_service.setFrameShadow(QFrame.Shadow.Sunken)

        self.keygen_label = QLabel(self.tab_service)
        self.keygen_label.setObjectName("keygen_label")
        self.keygen_label.setGeometry(QRect(11, 120, 200, 21))
        self.keygen_label.setFont(font)

        self.keygen_date_label = QLabel(self.tab_service)
        self.keygen_date_label.setObjectName("keygen_date_label")
        self.keygen_date_label.setGeometry(QRect(11, 150, 80, 26))

        self.keygen_date_input = QLineEdit(self.tab_service)
        self.keygen_date_input.setObjectName("keygen_date_input")
        self.keygen_date_input.setGeometry(QRect(95, 150, 200, 26))
        self.keygen_date_input.setEnabled(False)

        self.keygen_generate_button = QPushButton(self.tab_service)
        self.keygen_generate_button.setObjectName("keygen_generate_button")
        self.keygen_generate_button.setGeometry(QRect(305, 150, 90, 26))
        self.keygen_generate_button.setEnabled(False)

        self.keygen_result_label = QLabel(self.tab_service)
        self.keygen_result_label.setObjectName("keygen_result_label")
        self.keygen_result_label.setGeometry(QRect(11, 185, 80, 26))

        self.keygen_result_output = QLineEdit(self.tab_service)
        self.keygen_result_output.setObjectName("keygen_result_output")
        self.keygen_result_output.setGeometry(QRect(95, 185, 200, 26))
        self.keygen_result_output.setReadOnly(True)
        self.keygen_result_output.setEnabled(False)

        self.tabWidget.addTab(self.tab_service, "")

        self.gridLayout_3.addWidget(self.tabWidget, 0, 0, 1, 1)

        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(MainWindow)
        self.menubar.setObjectName("menubar")
        self.menubar.setGeometry(QRect(0, 0, 562, 33))
        self.menubar.setDefaultUp(False)
        self.menubar.setNativeMenuBar(True)
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)

        self.tabWidget.setCurrentIndex(0)
        self.tool_box.setCurrentIndex(1)

        QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(
            QCoreApplication.translate("MainWindow", "IoT Logger 1.0", None)
        )
        self.port_box.setPlaceholderText(
            QCoreApplication.translate("MainWindow", "Port", None)
        )
        self.connect_button.setText(
            QCoreApplication.translate("MainWindow", "Connect", None)
        )
        self.disconnect_button.setText(
            QCoreApplication.translate("MainWindow", "Disconnect", None)
        )
        self.refresh_port_button.setText("")
        self.ping_button.setText(QCoreApplication.translate("MainWindow", "Ping", None))
        self.reset_button.setText(
            QCoreApplication.translate("MainWindow", "Reset", None)
        )
        self.sync_time_button.setText(
            QCoreApplication.translate("MainWindow", "Sync time", None)
        )
        self.read_time_button.setText(
            QCoreApplication.translate("MainWindow", "Read time", None)
        )
        self.read_data_button.setText(
            QCoreApplication.translate("MainWindow", "Read data", None)
        )
        self.read_flags_button.setText(
            QCoreApplication.translate("MainWindow", "Read flags", None)
        )
        self.select_com_label.setText(
            QCoreApplication.translate("MainWindow", "Select COM", None)
        )
        self.service_enable_button.setText(
            QCoreApplication.translate("MainWindow", "Enable", None)
        )
        self.service_disable_button.setText(
            QCoreApplication.translate("MainWindow", "Disable", None)
        )
        self.service_password.setPlaceholderText(
            QCoreApplication.translate("MainWindow", "Password", None)
        )
        self.service_model_label.setText(
            QCoreApplication.translate("MainWindow", "Service mode", None)
        )
        self.data_label.setText(QCoreApplication.translate("MainWindow", "Data", None))
        self.tabWidget.setTabText(
            self.tabWidget.indexOf(self.tab),
            QCoreApplication.translate("MainWindow", "Communication", None),
        )
        self.output_off_button.setText(
            QCoreApplication.translate("MainWindow", "OFF", None)
        )
        self.outputs_label.setText(
            QCoreApplication.translate("MainWindow", "Outputs", None)
        )
        self.volume_label.setText(
            QCoreApplication.translate("MainWindow", "Volume", None)
        )
        self.set_pwm_button.setText(
            QCoreApplication.translate("MainWindow", "SET", None)
        )
        self.duty_cycle_label.setText(
            QCoreApplication.translate("MainWindow", "Duty cycle", None)
        )
        self.rgb_label.setText(QCoreApplication.translate("MainWindow", "RGB", None))
        self.read_sht_button.setText(
            QCoreApplication.translate("MainWindow", "Read", None)
        )
        self.sht_humidity_label.setText(
            QCoreApplication.translate("MainWindow", "Humidity", None)
        )
        self.current_color.setText("")
        self.bme_humidity_label.setText(
            QCoreApplication.translate("MainWindow", "Humidity", None)
        )
        self.red_label.setText(QCoreApplication.translate("MainWindow", "Red", None))
        self.green_label.setText(
            QCoreApplication.translate("MainWindow", "Green", None)
        )
        self.output_select_label.setText(
            QCoreApplication.translate("MainWindow", "Select output", None)
        )
        self.frequency_label.setText(
            QCoreApplication.translate("MainWindow", "Frequency", None)
        )
        self.bme_label.setText(
            QCoreApplication.translate("MainWindow", "BME280 sensor", None)
        )
        self.bme_temperature_label.setText(
            QCoreApplication.translate("MainWindow", "Temperature", None)
        )
        self.pwm_select_label.setText(
            QCoreApplication.translate("MainWindow", "Select output", None)
        )
        self.output_on_button.setText(
            QCoreApplication.translate("MainWindow", "ON", None)
        )
        self.blue_label.setText(QCoreApplication.translate("MainWindow", "Blue", None))
        self.color_label.setText(
            QCoreApplication.translate("MainWindow", "Color", None)
        )
        self.pwm_label.setText(QCoreApplication.translate("MainWindow", "PWM", None))
        self.sht_label.setText(
            QCoreApplication.translate("MainWindow", "SHT40 sensor", None)
        )
        self.bme_pressure_label.setText(
            QCoreApplication.translate("MainWindow", "Pressure", None)
        )
        self.buzzer_label.setText(
            QCoreApplication.translate("MainWindow", "Buzzer", None)
        )
        self.read_bme_button.setText(
            QCoreApplication.translate("MainWindow", "Read", None)
        )
        self.brightness_label.setText(
            QCoreApplication.translate("MainWindow", "Brightness", None)
        )
        self.sht_temperature_label.setText(
            QCoreApplication.translate("MainWindow", "Temperature", None)
        )
        self.tabWidget.setTabText(
            self.tabWidget.indexOf(self.tab_4),
            QCoreApplication.translate("MainWindow", "Data", None),
        )
        self.read_all_logs_button.setText(
            QCoreApplication.translate("MainWindow", "Read All Logs", None)
        )
        self.export_logs_button.setText(
            QCoreApplication.translate("MainWindow", "Export CSV", None)
        )
        self.clear_logs_table_button.setText(
            QCoreApplication.translate("MainWindow", "Clear", None)
        )
        self.log_count_label.setText(
            QCoreApplication.translate("MainWindow", "Logs: -", None)
        )
        self.tabWidget.setTabText(
            self.tabWidget.indexOf(self.tab_data_logs),
            QCoreApplication.translate("MainWindow", "Data Logs", None),
        )
        self.tabWidget.setTabText(
            self.tabWidget.indexOf(self.tab_chart),
            QCoreApplication.translate("MainWindow", "Chart", None),
        )
        self.logger_data_label.setText(
            QCoreApplication.translate("MainWindow", "Logger Data", None)
        )
        self.logger_id_label.setText(
            QCoreApplication.translate("MainWindow", "Logger ID", None)
        )
        self.sensor_id_label.setText(
            QCoreApplication.translate("MainWindow", "Sensor ID", None)
        )
        self.wifi_settings_label.setText(
            QCoreApplication.translate("MainWindow", "WiFi settings", None)
        )
        self.ssid_label.setText(QCoreApplication.translate("MainWindow", "SSID", None))
        self.password_label.setText(
            QCoreApplication.translate("MainWindow", "Password", None)
        )
        self.ntp_server_label.setText(
            QCoreApplication.translate("MainWindow", "NTP settings", None)
        )
        self.ntp_server_ip_label.setText(
            QCoreApplication.translate("MainWindow", "NTP Server", None)
        )
        self.mqtt_settings_label.setText(
            QCoreApplication.translate("MainWindow", "MQTT Settings", None)
        )
        self.mqtt_server_label.setText(
            QCoreApplication.translate("MainWindow", "MQTT Server", None)
        )
        self.mqtt_user_label.setText(
            QCoreApplication.translate("MainWindow", "MQTT User", None)
        )
        self.mqtt_password.setText(
            QCoreApplication.translate("MainWindow", "MQTT Password", None)
        )
        self.save_connection_data_button.setText(
            QCoreApplication.translate("MainWindow", "Save", None)
        )
        self.restore_connection_data_button.setText(
            QCoreApplication.translate("MainWindow", "Restore", None)
        )
        self.tool_box.setItemText(
            self.tool_box.indexOf(self.page),
            QCoreApplication.translate("MainWindow", "Connection data", None),
        )
        self.ext_rtc_flag.setText(
            QCoreApplication.translate("MainWindow", "External RTC", None)
        )
        self.flash_memory_flag.setText(
            QCoreApplication.translate("MainWindow", "Flash memory", None)
        )
        self.lcd_display_flag.setText(
            QCoreApplication.translate("MainWindow", "LCD display", None)
        )
        self.sht_sensor_flag.setText(
            QCoreApplication.translate("MainWindow", "SHT40 sensor", None)
        )
        self.bme_sensor_flag.setText(
            QCoreApplication.translate("MainWindow", "BME280 sensor", None)
        )
        self.ina226_flag.setText(
            QCoreApplication.translate("MainWindow", "INA226", None)
        )
        self.adc_measurement_flag.setText(
            QCoreApplication.translate("MainWindow", "ADC measurement", None)
        )
        self.can_flag.setText(QCoreApplication.translate("MainWindow", "CAN", None))
        self.network_enable_flag.setText(
            QCoreApplication.translate("MainWindow", "Network enabled", None)
        )
        self.save_flags_button.setText(
            QCoreApplication.translate("MainWindow", "Save", None)
        )
        self.restore_flags_button.setText(
            QCoreApplication.translate("MainWindow", "Restore", None)
        )
        self.tool_box.setItemText(
            self.tool_box.indexOf(self.page_2),
            QCoreApplication.translate("MainWindow", "Flags", None),
        )
        self.tabWidget.setTabText(
            self.tabWidget.indexOf(self.tab_3),
            QCoreApplication.translate("MainWindow", "Settings", None),
        )
        self.save_log_button.setText(
            QCoreApplication.translate("MainWindow", "Save", None)
        )
        self.clear_log_button.setText(
            QCoreApplication.translate("MainWindow", "Clear", None)
        )
        self.tabWidget.setTabText(
            self.tabWidget.indexOf(self.tab_2),
            QCoreApplication.translate("MainWindow", "Logs", None),
        )
        self.service_login_label.setText(
            QCoreApplication.translate("MainWindow", "Service Login", None)
        )
        self.service_username_label.setText(
            QCoreApplication.translate("MainWindow", "Username", None)
        )
        self.service_password_label.setText(
            QCoreApplication.translate("MainWindow", "Password", None)
        )
        self.service_login_button.setText(
            QCoreApplication.translate("MainWindow", "Login", None)
        )
        self.service_logout_button.setText(
            QCoreApplication.translate("MainWindow", "Logout", None)
        )
        self.service_status_label.setText(
            QCoreApplication.translate("MainWindow", "Not logged in", None)
        )
        self.keygen_label.setText(
            QCoreApplication.translate("MainWindow", "Key Generator", None)
        )
        self.keygen_date_label.setText(
            QCoreApplication.translate("MainWindow", "Date", None)
        )
        self.keygen_date_input.setPlaceholderText(
            QCoreApplication.translate("MainWindow", "YYYY-MM-DD", None)
        )
        self.keygen_generate_button.setText(
            QCoreApplication.translate("MainWindow", "Generate", None)
        )
        self.keygen_result_label.setText(
            QCoreApplication.translate("MainWindow", "Key", None)
        )
        self.tabWidget.setTabText(
            self.tabWidget.indexOf(self.tab_service),
            QCoreApplication.translate("MainWindow", "Service", None),
        )
