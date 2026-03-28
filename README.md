# 🌡️ IoT Logger Dashboard

A full-stack IoT monitoring platform for collecting, visualizing, and managing environmental sensor data from distributed devices. The system integrates custom-built hardware (ESP32, STM32, MicroPython nodes) with a modern web dashboard, communicating over MQTT and WebSockets for real-time data streaming.

---

## 📋 Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Technology Stack](#technology-stack)
- [Features](#features)
- [Project Structure](#project-structure)
- [Backend](#backend)
- [Frontend](#frontend)
- [Logger Firmware](#logger-firmware)
- [Database](#database)
- [Authentication & Authorization](#authentication--authorization)
- [MQTT Communication](#mqtt-communication)
- [Hardware](#hardware)
- [Getting Started](#getting-started)
- [Environment Variables](#environment-variables)
- [API Reference](#api-reference)
- [Testing](#testing)

---

## Overview

IoT Logger Dashboard is designed to monitor environmental conditions (temperature, humidity, atmospheric pressure, altitude, voltage) across multiple locations. Sensor nodes deployed in buildings collect data and transmit it via MQTT to a centralized backend. Users interact with the system through a responsive web dashboard that provides real-time charts, floor plan visualizations, equipment management, and a full role-based administration panel.

**Key metrics:**

- 380+ tracked devices (loggers, sensors, relays)
- 247,000+ recorded measurements
- 12 measurement parameter types
- Multi-location, multi-floor support
- 90 concurrent database connections
- 4 JWT token types for layered security

---

## Architecture

```
┌──────────────────┐      MQTT       ┌──────────────────┐     HTTP/WS     ┌──────────────────┐
│   Sensor Nodes   │ ──────────────► │     Backend      │ ◄────────────── │    Frontend      │
│                  │                 │                  │ ──────────────► │                  │
│  ESP32 + STM32   │  devices/+/    │  Express.js 5.2  │   Socket.io     │  React 19 + TS   │
│  MicroPython     │  status        │  Sequelize ORM   │   4.8           │  MUI 7 + Redux   │
│  SHT40 / BME280  │                │  MQTT Client     │                 │  ECharts + XYFlow│
│  PCF8563T RTC    │  ◄──────────── │  Socket.io 4.8   │                 │  RTK Query       │
│                  │  devices/      │  Sharp + Multer  │                 │  Vite 7.3 (SWC)  │
│                  │  {id}/cmd      │  Nodemailer      │                 │                  │
└──────────────────┘                 └────────┬─────────┘                 └──────────────────┘
                                              │
                                     ┌────────▼─────────┐
                                     │   MySQL 9.x DB   │
                                     │  14 tables       │
                                     │  5 views         │
                                     │  Pool: 90 conn   │
                                     └──────────────────┘
```

---

## Technology Stack

### Backend

| Technology        | Version    | Purpose                                    |
| ----------------- | ---------- | ------------------------------------------ |
| Node.js           | ES Modules | Runtime environment                        |
| Express.js        | 5.2.1      | HTTP framework                             |
| Sequelize         | 6.37.7     | MySQL ORM with transactions & soft deletes |
| mysql2            | 3.16.1     | MySQL driver                               |
| Socket.io         | 4.8.3      | Real-time WebSocket communication          |
| MQTT.js           | 5.14.1     | IoT message broker client                  |
| JSON Web Token    | 9.0.3      | Authentication (4 token types)             |
| express-validator | 7.3.1      | Request body validation                    |
| bcryptjs          | 3.0.3      | Password hashing                           |
| Sharp             | 0.34.5     | Image processing & WebP conversion         |
| Multer            | 2.0.2      | File upload handling (500KB limit)         |
| Nodemailer        | 7.0.12     | Email via Gmail SMTP (password reset)      |
| Morgan            | 1.10.1     | HTTP access logging                        |
| Compression       | 1.8.1      | Gzip response compression                  |
| uuid              | 13.0.0     | Unique filename generation                 |
| dotenv            | 17.2.3     | Environment variable management            |
| Jest              | 30.2.0     | Testing framework                          |
| Supertest         | 7.2.2      | HTTP integration testing                   |
| ESLint + Prettier | —          | Code quality & formatting                  |

### Frontend

| Technology          | Version | Purpose                            |
| ------------------- | ------- | ---------------------------------- |
| React               | 19.2.3  | UI framework                       |
| TypeScript          | 5.9.3   | Type safety                        |
| Vite                | 7.3.1   | Build tool (SWC transpiler)        |
| Material UI (MUI)   | 7.3.7   | Component library                  |
| MUI X Data Grid     | 8.26.0  | Advanced sortable/paginated tables |
| Redux Toolkit       | 2.11.2  | State management with RTK Query    |
| React Redux         | 9.2.0   | React-Redux bindings               |
| React Router        | 7.13.0  | Client-side routing with loaders   |
| Socket.io Client    | 4.8.3   | Real-time device updates           |
| ECharts             | 3.0.6   | Time-series data visualization     |
| XYFlow (React Flow) | 12.10.0 | Graph / flow diagrams              |
| Styled Components   | 6.3.8   | CSS-in-JS styling                  |
| Emotion             | 11.14.0 | MUI styling engine                 |
| xlsx                | 0.18.5  | Excel spreadsheet generation       |
| file-saver          | 2.0.5   | Client-side file download          |
| jwt-decode          | 4.0.0   | Client-side JWT inspection         |

### Embedded / Logger

| Technology                        | Purpose                                      |
| --------------------------------- | -------------------------------------------- |
| ESP32 (MicroPython)               | Main IoT gateway — WiFi, MQTT, UART to STM32 |
| STM32L476RG / STM32F412RG (CMSIS) | Low-level sensor hub — bare-metal C with DMA |
| MicroPython (generic)             | Standalone sensor nodes — HTTP POST to API   |
| PlatformIO                        | STM32 build & flash toolchain                |
| KiCad                             | PCB schematic & layout design                |

### Sensors & Peripherals

| Component     | Type       | Measurements / Function                               |
| ------------- | ---------- | ----------------------------------------------------- |
| SHT30 / SHT40 | I2C sensor | Temperature (-40–125°C), Humidity (0–100%)            |
| BME280        | I2C sensor | Temperature, Humidity, Atmospheric Pressure, Altitude |
| PCF8563T      | I2C RTC    | Real-Time Clock with battery backup                   |
| I2C LCD       | Display    | Sensor readings & status display                      |
| RGB LEDs      | Output     | Status indication (PWM, 0–65535 range)                |
| Relay module  | Output     | 4-channel relay control (pins 12–15)                  |
| Buzzer        | Output     | Audible alerts on threshold breach                    |
| Push Buttons  | Input      | User interaction (debounced, long-press 3s)           |

---

## Features

- **Real-Time Monitoring** — Live sensor data via MQTT → Socket.io WebSocket pipeline with per-logger events
- **Interactive Charts** — Time-series visualization with ECharts, range selection, and XLSX export
- **Floor Plan Visualization** — Upload floor plan images, place loggers with x/y positioning and zoom control
- **Multi-Location Support** — Organize devices by house → floor → logger hierarchy
- **Equipment Inventory** — Full CRUD for loggers, sensors, relays with soft delete and restore
- **Device Telemetry** — Track firmware/hardware versions, serial numbers, build dates, IP address, last seen
- **Sensor Capability Mapping** — Many-to-many mapping of sensors to data definition parameters
- **Role-Based Access Control** — Three-dimensional permission matrix: functionality × object × access level
- **Superuser Override** — Superuser flag bypasses all permission checks
- **User Management** — Registration, login, avatar upload with Sharp WebP conversion, password reset via Gmail SMTP
- **Data Export** — Export sensor data to XLSX spreadsheets with pivot view support
- **Error Tracking** — Log device and system errors with type (Equipment/DB/Other) and severity (Critical→Info)
- **Equipment Event Logs** — Track STATUS, DATA, OTA, ERROR events per device
- **Process Management** — Define and manage operational process types and definitions
- **Audit Trail** — `created_by` / `updated_by` user tracking on all key entities
- **OTA Support** — Custom UART bootloader for STM32 firmware updates (app at `0x08008000`)
- **Service Mode** — ESP32 3-hour OTA window for configuration and firmware updates
- **Image Processing** — Automatic thumbnail generation and WebP conversion for avatars and floor plans
- **HTTP Access Logging** — Morgan combined-format logging to `access.log`
- **Gzip Compression** — Response compression via compression middleware
- **Transaction Safety** — Sequelize transactions with rollback on all write operations

---

## Project Structure

```
Logger_Dashboard/
│
├── backend/                    # Node.js Express API server
│   ├── server/
│   │   ├── app.js              # Express app setup, middleware, CORS
│   │   ├── mqttClient.js       # MQTT subscriber & data processing
│   │   ├── api/
│   │   │   ├── controller/     # Request handlers
│   │   │   ├── model/          # Sequelize models
│   │   │   └── routes/         # Express route definitions
│   │   ├── bin/www.js          # HTTP server bootstrap + Socket.io init
│   │   ├── libs/jwtToken.js    # JWT generation (4 token types)
│   │   ├── middleware/
│   │   │   ├── body-validation.js   # Request body validation
│   │   │   ├── file.js              # File upload handling (multer)
│   │   │   ├── jwtValidation.js     # JWT auth + permission checking
│   │   │   └── socket.js           # Socket.io middleware
│   │   └── util/
│   │       ├── database.js          # Sequelize connection config
│   │       ├── nodemailer.js        # Email transport setup
│   │       ├── responseHelper.js    # Standardized API responses
│   │       └── sequelizeTools.js    # Query builder utilities
│   ├── uploads/                # Uploaded files (avatars, floor plans)
│   ├── data.json               # Static configuration data
│   ├── package.json
│   ├── jest.config.js
│   └── eslint.config.js
│
├── frontend/                   # React SPA
│   ├── src/
│   │   ├── App.tsx             # Root component
│   │   ├── main.tsx            # Entry point
│   │   ├── components/         # Reusable UI components
│   │   │   ├── selectors/      # Equipment, sensor, house dropdowns
│   │   │   ├── ui/             # SnackBar, forms, modals
│   │   │   └── scripts/        # API interaction helpers
│   │   ├── modules/            # Feature modules (pages)
│   │   │   ├── admin/          # User/role/permission management
│   │   │   ├── dashboard/      # Real-time overview
│   │   │   ├── data/           # Charts, data tables, layout view
│   │   │   ├── equipment/      # Device inventory
│   │   │   ├── house/          # Location & floor management
│   │   │   ├── user/           # User profile
│   │   │   └── process/        # Process workflows
│   │   ├── router/             # SPA route configuration
│   │   ├── socket/             # Socket.io client setup
│   │   ├── store/              # Redux store (auth, account, app state)
│   │   └── util/               # Theme, API helpers, utilities
│   ├── dist-frontend/          # Production build output
│   ├── package.json
│   ├── vite.config.ts
│   └── tsconfig.json
│
├── logger/                     # Embedded firmware
│   ├── ESP32/                  # ESP32 MicroPython firmware
│   │   ├── main.py             # Boot: WiFi → NTP → MQTT → STM32
│   │   ├── program.py          # Main loop: status/data publishing
│   │   ├── mqtt_simple.py      # Lightweight MQTT client
│   │   ├── wireless.py         # WiFi + NTP connection manager
│   │   ├── stm32_uart.py       # UART bridge to STM32 (115200 baud)
│   │   ├── config.py           # Device configuration (IDs, WiFi, MQTT)
│   │   └── status.json         # Runtime status snapshot
│   ├── Micropython/            # Generic MicroPython sensor nodes
│   │   ├── main.py             # Sensor reading + HTTP POST loop
│   │   ├── config.py           # Node configuration
│   │   ├── sht30.py / sht40.py # Temperature/humidity drivers
│   │   ├── bme280_i2c.py       # Pressure/altitude driver
│   │   ├── i2c_lcd.py          # LCD display driver
│   │   ├── rtc_clock.py        # RTC time management
│   │   └── ...                 # Additional peripheral drivers
│   └── STM32/                  # STM32 bare-metal firmware (C / CMSIS)
│       ├── application/        # Main application (PlatformIO project)
│       │   └── src/            # adc, bme280, sht40, i2c, spi, uart,
│       │                       # lcd, rtc, pcf8563t, dma drivers
│       ├── bootloader/         # UART bootloader for OTA updates
│       │   └── src/            # main.c, uart.c, systick.c
│       ├── production_files/   # Flash programming scripts & binaries
│       ├── STM32F412RGT6/      # Reference docs & alt. target
│       └── STM32L4/            # Reference docs for L476RG
│
├── database/                   # Database setup
│   ├── db.sql                  # Schema: tables, views, indexes
│   └── db_data.sql             # Seed data (types, definitions, roles)
│
├── permissions/
│   └── permissions.json        # Default permission matrix
│
└── docs/                       # Documentation & hardware resources
    ├── LoggerDashboard.postman_collection.json  # API collection
    ├── Cover/PicoLogger/       # 3D-printable enclosures (.stl)
    └── PCB/PicoLogger/         # KiCad PCB designs
        ├── IoT_Logger_B_1.0/
        ├── IoT_Logger_C_1.0/
        ├── Pico_TH_Logger_SMD_PCF8563T_2.1/
        └── Pico_TH_Logger_Relay_SMD_PCF8563T_2.1/
```

---

## Backend

### Express API Server

The backend is a Node.js Express 5.2 server with ES module support. It provides a RESTful API, handles MQTT message ingestion from IoT devices, and broadcasts real-time updates to connected clients via Socket.io.

**Middleware stack (in order):**

1. **CORS** — Configurable origins via `FRONTEND_ORIGIN` env var (comma-separated)
2. **Compression** — Gzip response compression
3. **Morgan** — HTTP access logging (combined format → `access.log`)
4. **Express JSON/URL-encoded** — Body parsing with 10MB limit
5. **Cookie Parser** — Cookie handling
6. **Static Files** — Serves `/uploads` and frontend build (`dist-frontend/`)
7. **JWT Validation** — Bearer token verification on protected routes
8. **Permission Checking** — Granular RBAC enforcement per endpoint

**Route mounting:**

| Mount Path       | Module     |
| ---------------- | ---------- |
| `/api/user`      | User       |
| `/api/data`      | Data       |
| `/api/equipment` | Equipment  |
| `/api/process`   | Process    |
| `/api/house`     | House      |
| `/api/common`    | Error Logs |
| `/api/mqtt`      | MQTT       |
| `/api/adm`       | Admin      |

**Error handling:**

- 404 → `notFound()` for unknown routes
- Global error handler → `internalServerError()`
- In production, unknown routes serve `index.html` (SPA fallback)

**Standardized API response format:**

```json
{
	"success": true,
	"message": "Description of result",
	"data": {}
}
```

**HTTP status code helpers:** `200`, `201`, `204`, `400`, `401`, `403`, `404`, `409`, `422`, `429`, `500`, `503`

### File Upload & Image Processing

| Setting       | Value                                              |
| ------------- | -------------------------------------------------- |
| Max file size | 500 KB                                             |
| MIME types    | `image/png`, `image/jpeg`, `image/jpg`             |
| Storage       | `uploads/` directory                               |
| Filename      | `${uuid()}.${extension}`                           |
| Processing    | Sharp — auto WebP conversion, thumbnail generation |

Image variants stored:

- **Avatars:** `avatar` (thumbnail) + `avatar_big` (full-size)
- **House pictures:** `picture_link` + `picture_link_big`
- **Floor layouts:** `layout` (small) + `layout_big` (full-size)

### MQTT Data Pipeline

When sensor data arrives over MQTT:

1. Message is parsed from the `devices/+/status` topic (QoS 0)
2. Message type is identified: `DATA`, `STATUS`, `ERROR`, or `OTA`
3. Timestamps are normalized to `Europe/Warsaw` timezone (UTC+1/+2)
4. **For DATA messages:**
   - SHT40 readings extracted → `temperature`, `humidity`
   - BME280 readings extracted → `temperature`, `humidity`, `pressure` → `atmPressure`, `altitude`
   - Voltage readings extracted → `vin` array → individual voltage values
   - Each measurement matched to `DataDefinition` by name
   - `DataLogs` record created with value + timestamp
   - `DataLastValue` cache replaced with latest reading
5. **For all messages:** `EquStats` updated (firmware versions, serial numbers, build dates, IP, `last_seen`)
6. Socket.io emits `logger_${loggerId}` event with `'refresh'` payload
7. **For ERROR messages:** `ErrorLog` record created in database

**MQTT connection config:**

```
Reconnect period: 2000ms
Keep-alive: 30s
Clean session: true
```

---

## Frontend

### React SPA

The frontend is a single-page application built with React 19, TypeScript, and Material UI 7. It uses Redux Toolkit with RTK Query for API calls and Socket.io for real-time updates.

**State management (Redux store slices):**

- `accountSlice` — User profile data, avatar
- `authenticateSlice` — JWT tokens, login state, permissions
- `applicationSlice` — Global UI state (snackbar alerts, loading indicators)
- `api` — RTK Query endpoints with auto-caching and tag-based invalidation

**RTK Query features:**

- Auto Bearer token injection from localStorage
- 401 error handling → automatic logout + redirect to `/login`
- Zero retry policy (`maxRetries: 0`)
- Cache tags: User, Equipment, House, Data, Admin (14 tag types)

**Socket.io client:**

- Timeout: 5000ms, reconnection attempts: 3, delay: 1000ms
- Listens to `logger_${loggerId}` events for per-device data refresh

### Routes & Pages

```
/ ........................... Main menu
/login ...................... Login page
/register ................... Registration page
/logout ..................... Logout action
/password-reset ............. Request password reset email
/password-reset/:token ...... Complete password reset

/house ...................... House management
  /house/houses ............. House list & CRUD
  /house/floors ............. Floor list & CRUD
  /house/loggers ............ Logger placement management
/house-details/:houseId ..... Floor plan viewer with logger positions

/equipment .................. Equipment inventory
  Types, vendors, models, equipment CRUD
  Equipment log viewer

/logs/:equLoggerId .......... Equipment event log

/data ....................... Data module
  /data ..................... Data summary & definitions
  /data/data-logger/:id ..... Time-series chart view (ECharts)

/admin-panel ................ Administration
  Functionality, object, access level definitions
  Role & permission management
  User management

/user ....................... User module
  Profile, permissions view, roles
```

**Module views:**

| Module            | Description                                                                      |
| ----------------- | -------------------------------------------------------------------------------- |
| **Dashboard**     | Real-time overview cards with latest device readings                             |
| **Data**          | Time-series charts (ECharts), pivot tables, XLSX export, layout visualization    |
| **Equipment**     | CRUD for devices, vendors, models, types; soft delete/restore; event logs        |
| **House**         | Location CRUD, floor plans with image upload, interactive logger placement       |
| **House Details** | Floor plan viewer with logger positions, floor tree navigation, edit forms       |
| **Admin**         | Functionality/object/access level definitions, role CRUD, permission matrix      |
| **User**          | Login, registration, profile with avatar upload, password reset, permission view |
| **Process**       | Process type & definition management                                             |

### Reusable Components

**18 Select dropdowns:** `HouseSelect`, `HouseFloorSelect`, `EquipmentSelect`, `EquipmentVendorSelect`, `EquipmentTypeSelect`, `EquipmentModelSelect`, `EquipmentLoggerSelect`, `EquipmentSensorSelect`, `DataDefinitionSelect`, `AdminUserSelect`, `AdminAccessLevelSelect`, `AdminFunctionalityDefinitionSelect`, `AdminObjectDefinitionSelect`, and more.

**UI components:** `SnackBar` (alert notifications), `LoadingCircle` (spinner), `Wrapper` (layout), `TabPanel` (tabbed content).

---

## Logger Firmware

### ESP32 (MicroPython) — IoT Gateway

The ESP32 acts as the main IoT gateway, bridging STM32 sensor hardware to the cloud. On boot it:

1. Configures GPIO pins (status LED, STM32 control, power LED)
2. Initializes I2C bus for sensor modules and LCD
3. Connects to WiFi (up to 20 retries, 1s each)
4. Synchronizes time via NTP (UTC+1, max 10 retries)
5. Initializes MQTT connection to the broker
6. Establishes UART communication with the STM32 (115200 baud)
7. Enters the main loop publishing status and sensor data

**GPIO Configuration:**

| Pin | Function          | Mode              |
| --- | ----------------- | ----------------- |
| 18  | Status interrupt  | Input, IRQ rising |
| 19  | STM32 PWM control | Output            |
| 22  | Status LED        | Output            |

**STM32 UART Protocol:**

- Baud rate: 115200
- Device address: `0xB2`
- Frame length: 32 bytes
- Status OK: `0x40`, Error: `0x7F`
- Authentication key for secure UART communication

**I/O Mapping (via STM32):**

| ID   | Inputs  | ID        | Outputs       |
| ---- | ------- | --------- | ------------- |
| 0x01 | BTN1    | 0x01      | LED1          |
| 0x02 | BTN2    | 0x02      | LED2          |
| 0x03 | ESP_STS | 0x03–0x07 | PB12, PC0–PC3 |

**FRAM Persistent Storage:**

- `0x008` — Logger ID
- `0x028` — Sensor ID
- `0x128` — Measurement interval

**Key features:**

- Service Mode: 3-hour OTA update window (triggered by command)
- Debounced IRQ handling (50ms debounce)
- Color mapping for RGB LEDs (0–255 → 0–65535 PWM)
- Automatic WiFi reconnection (5s interval)
- MQTT keepalive: 7200s (2 hours)
- Error reporting to backend via HTTP POST (3s timeout)

### STM32 (Bare-Metal C / CMSIS) — Sensor Hub

The STM32 handles all low-level sensor communication using DMA for efficient, non-blocking peripheral access.

**Target MCUs:**

| MCU           | Board         | Framework | Debug   |
| ------------- | ------------- | --------- | ------- |
| STM32L476RG   | NUCLEO-L476RG | CMSIS     | ST-Link |
| STM32F412RGT6 | Custom PCB    | CMSIS     | ST-Link |

**Memory layout:**

```
0x08000000 ─ Bootloader (UART-based, for OTA updates)
0x08008000 ─ Application (sensor management + data streaming)
0x080FF800 ─ Info page (device production data, flashed via OpenOCD)
```

**Application source modules (bare-metal C):**

| Module           | Function                                    |
| ---------------- | ------------------------------------------- |
| `main.c`         | System init, DMA config, main state machine |
| `adc_dma.c`      | ADC voltage readings with DMA transfer      |
| `bme280_dma.c`   | BME280 pressure/altitude sensor (I2C + DMA) |
| `sht40.c`        | SHT40 temperature/humidity sensor           |
| `pcf8563t_dma.c` | PCF8563T RTC clock with DMA                 |
| `i2c_dma.c`      | I2C peripheral driver with DMA              |
| `spi_dma.c`      | SPI peripheral driver with DMA              |
| `uart_dma.c`     | UART communication with ESP32 (115200 baud) |
| `dma.c`          | DMA controller configuration                |
| `lcd.c`          | LCD display driver                          |
| `rtc.c`          | Internal RTC peripheral                     |
| `irq.c`          | Interrupt request handlers                  |
| `timer.c`        | General-purpose timers (PWM, compare)       |
| `systick.c`      | System tick timer (1ms resolution)          |
| `support.c`      | System support (clock config, reset)        |

**Bootloader:** Minimal UART bootloader (`main.c`, `uart.c`, `systick.c`) enabling firmware updates without physical access to the device.

### MicroPython Sensor Nodes — Standalone

Independent nodes that periodically read sensors and POST data directly to the backend REST API (no MQTT).

| File              | Purpose                            |
| ----------------- | ---------------------------------- |
| `main.py`         | Boot, WiFi, sensor loop, HTTP POST |
| `config.py`       | IDs, WiFi, server URL, intervals   |
| `sht30.py`        | SHT30 full driver (I2C protocol)   |
| `sht30_simple.py` | SHT30 simplified driver            |
| `sht40.py`        | SHT40 driver                       |
| `bme280_i2c.py`   | BME280 driver (pressure/altitude)  |
| `i2c_lcd.py`      | I2C LCD display driver             |
| `lcd_api.py`      | LCD abstraction layer              |
| `rtc_clock.py`    | PCF8563 RTC full driver            |
| `rtc_simple.py`   | RTC simplified driver              |
| `test.py`         | Sensor interface tests             |

**Configuration:**

- Post interval: 600 seconds (configurable)
- Endpoint: `http://{SERVER_IP}:{PORT}/api/data/data-log`
- Thresholds: Temperature 20–25°C, Humidity 30–80% (triggers buzzer/LED alerts)
- LCD backlight auto-off: 20 seconds
- Long-press button detection: 3000ms threshold

---

## Database

MySQL 9.x database with 14 tables and 5 views. Connection pool: max 90 connections, idle timeout 10s, timezone UTC.

### Entity-Relationship Overview

```
┌───────────┐    ┌───────────────┐    ┌─────────────┐
│   users   │───►│ adm_roles_user│◄───│  adm_roles  │
└─────┬─────┘    └───────────────┘    └──────┬──────┘
      │                                      │
      │          ┌───────────────────┐        │
      └─────────►│  superusers       │        │
                 └───────────────────┘        │
      │                                      ▼
      ▼                              ┌──────────────────────┐
┌─────────────────┐                  │ adm_functionality_def│
│ adm_permissions  │◄────────────────└──────────────────────┘
│ (user/role →     │                 ┌──────────────────────┐
│  func → obj →    │◄────────────────│ adm_object_definition│
│  access level)   │                 └──────────────────────┘
└─────────────────┘                  ┌──────────────────────┐
                                     │ adm_access_level_def │
                                     └──────────────────────┘

┌───────────┐    ┌─────────────┐    ┌────────────┐
│ equ_type  │◄───│equ_equipment│───►│ equ_vendor │
└───────────┘    └──────┬──────┘    └────────────┘
                        │           ┌────────────┐
                        ├──────────►│ equ_model  │
                        │           └────────────┘
                        │           ┌───────────────────┐
                        ├──────────►│equ_sensor_functions│ (M:M → data_definitions)
                        │           └───────────────────┘
                        ▼
              ┌─────────────────┐
              │   equ_stats     │  (last_seen, firmware, hw, IP, serial)
              │   equ_log       │  (STATUS, DATA, OTA, ERROR, OTHER)
              │   error_log     │  (Equipment/DB/Other × Critical→Info)
              └─────────────────┘

┌─────────────────┐    ┌──────────────┐    ┌──────────────┐
│ data_definitions│◄───│  data_logs   │───►│equ_equipment │
└─────────────────┘    └──────┬───────┘    │ (logger+sensor)│
                              │            └──────────────┘
                       ┌──────▼───────┐
                       │data_last_value│
                       └──────────────┘

┌─────────────┐    ┌──────────────┐    ┌──────────────┐
│ house_house │───►│ house_floors │───►│ house_logger │
│ (address,   │    │ (layout img, │    │ (pos_x/y,    │
│  picture)   │    │  zoom, x/y)  │    │  1:1 logger) │
└─────────────┘    └──────────────┘    └──────────────┘

┌──────────────┐    ┌───────────────────┐
│ process_type │───►│ process_definition│
└──────────────┘    └───────────────────┘
```

### Complete Table Reference

#### Users & Authorization

| Table            | Key Columns                                                                                                                           | Records |
| ---------------- | ------------------------------------------------------------------------------------------------------------------------------------- | ------- |
| `users`          | id, username (UNIQUE), email (UNIQUE), password (bcrypt), avatar, avatar_big, reset_password_token, reset_password_expires, confirmed | ~23     |
| `superusers`     | id, user_id (FK → users) — bypasses all permission checks                                                                             | ~2      |
| `adm_roles_user` | role_id (PK), user_id (PK) — many-to-many                                                                                             | —       |

#### Admin Permissions

| Table                          | Key Columns                                                                                                                                      | Records                                                                              |
| ------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------ |
| `adm_roles`                    | id, name, description, created_by_id, updated_by_id                                                                                              | ~4 (Admin, Common, Technician)                                                       |
| `adm_functionality_definition` | id, name, description                                                                                                                            | ~17 (equ, house, data, adm, process, common)                                         |
| `adm_object_definition`        | id, name, description                                                                                                                            | ~27 (equType, equVendor, equEquipment, houseHouse, houseFloor, dataDefinition, etc.) |
| `adm_access_level_definition`  | id, name, access_level (INT)                                                                                                                     | ~20 (READ, WRITE, DELETE, Admin; levels 0–50)                                        |
| `adm_permissions`              | id, user_id (nullable), role_id (nullable), adm_functionality_definition_id, adm_object_definition_id (nullable), adm_access_level_definition_id | ~56                                                                                  |

#### Equipment

| Table                  | Key Columns                                                                                                                       | Records |
| ---------------------- | --------------------------------------------------------------------------------------------------------------------------------- | ------- |
| `equ_type`             | id, name                                                                                                                          | ~14     |
| `equ_vendor`           | id, name                                                                                                                          | ~18     |
| `equ_model`            | id, name                                                                                                                          | ~21     |
| `equ_equipment`        | id, serial_number, equ_vendor_id, equ_model_id, equ_type_id, created_by_id, updated_by_id, deleted_at, is_deleted                 | ~380    |
| `equ_sensor_functions` | equ_sensor_id (PK), data_definition_id (PK) — composite key, M:M                                                                  | —       |
| `equ_stats`            | id, equipment_id, last_seen, sn_contr, fw_contr, hw_contr, build_contr, prod_contr, sn_com, fw_com, hw_com, build_com, ip_address | ~3,045  |
| `equ_log`              | id, equipment_id, message, type (ENUM: STATUS/DATA/OTA/ERROR/OTHER)                                                               | ~32     |

#### Data Measurements

| Table              | Key Columns                                                       | Records                                                                  |
| ------------------ | ----------------------------------------------------------------- | ------------------------------------------------------------------------ |
| `data_definitions` | id, name, unit, description                                       | ~12 (temperature °C, humidity %, atmPressure hPa, altitude m, voltage V) |
| `data_logs`        | id, value, data_definition_id, equ_logger_id, equ_sensor_id, time | ~247,158                                                                 |
| `data_last_value`  | id, data_log_id, equ_logger_id, equ_sensor_id, data_definition_id | ~247,150                                                                 |

#### Houses & Locations

| Table          | Key Columns                                                                                                     | Records |
| -------------- | --------------------------------------------------------------------------------------------------------------- | ------- |
| `house_house`  | id, name, postal_code, city, street, house_number, picture_link, picture_link_big, created_by_id, updated_by_id | ~60     |
| `house_floors` | id, name, layout, layout_big, house_id, x, y, zoom, pos_x, pos_y                                                | ~17     |
| `house_logger` | id, equ_logger_id (UNIQUE), house_floor_id, pos_x, pos_y                                                        | ~38     |

#### Other

| Table                | Key Columns                                                                                                             | Records |
| -------------------- | ----------------------------------------------------------------------------------------------------------------------- | ------- |
| `process_type`       | id, name                                                                                                                | ~6      |
| `process_definition` | id, process_type_id, name, created_by_id, updated_by_id                                                                 | ~2      |
| `error_log`          | id, message, details (JSON), type (Equipment/DB/Other), severity (Critical/Error/Warning/Info), equipment_id (nullable) | ~937    |

### Database Views

| View                         | Purpose                                                                         |
| ---------------------------- | ------------------------------------------------------------------------------- |
| `adm_view_permission`        | Flattened permission matrix joining adm_permissions with adm_roles              |
| `data_view_last_value`       | Latest sensor values enriched with time, value, parameter name, unit, floor IDs |
| `data_view_logs`             | Pivoted data logs — columns: temperature, humidity, atmPressure, altitude       |
| `data_view_connected_sensor` | Sensors connected to loggers on house floors with vendor, model, serial         |
| `equ_view_unused_logger`     | Loggers of type 'logger' not yet assigned to any house floor                    |

---

## Authentication & Authorization

### JWT Token System

| Type | Expiry   | Secret                    | Purpose                              |
| ---- | -------- | ------------------------- | ------------------------------------ |
| 0    | 5 hours  | `TOKEN`                   | Standard authentication              |
| 1    | 365 days | `TOKEN`                   | "Remember me" persistent session     |
| 2    | 5 hours  | `TOKEN_SECRET_PERMISSION` | Permission token (stores user perms) |
| 3    | 1 minute | `TOKEN`                   | Short-lived data access token        |

Protected routes require a `Bearer` token in the `Authorization` header. The middleware validates the token signature and attaches the decoded user object to `req.user`.

### Login Flow

```
1. User submits username + password → POST /api/user/user-login
2. Backend finds user by username in users table
3. bcrypt.compare() against stored password hash
4. On success:
   ├─ Generate Type 0 auth token (5h) with TOKEN secret
   ├─ Generate Type 2 permission token (5h) with TOKEN_SECRET_PERMISSION
   └─ Return both tokens to frontend
5. Frontend stores both tokens in localStorage
6. Permission token used for client-side UI access control
7. Auth token sent as Bearer header on every API request
```

### Password Reset Flow

```
1. User submits email → POST /api/user/reset-password-request
2. Backend generates reset token + expiry, saves to users table
3. Nodemailer sends email via Gmail SMTP with reset link
4. User clicks link → POST /api/user/reset-password/:token
5. Backend validates token + expiry, hashes new password, clears token
```

### Role-Based Access Control (RBAC)

Permissions are defined as a three-dimensional matrix:

```
Permission = Functionality × Object × Access Level
```

- **Functionality** — module scope: `equ`, `house`, `data`, `adm`, `process`, `common` (~17 definitions)
- **Object** — resource type: `equType`, `equVendor`, `equEquipment`, `houseHouse`, `houseFloor`, `dataDefinition`, etc. (~27 definitions)
- **Access Level** — operation: `READ`, `WRITE`, `DELETE`, `Admin` (levels 0–50, ~20 definitions)

**Permission resolution (`checkPermission()`):**

```
1. Extract user from JWT token
2. Find direct user permissions (adm_permissions.user_id = userId)
3. Find user's roles via adm_roles_user
4. Find role-based permissions (adm_permissions.role_id IN userRoles)
5. Combine all permissions
6. Check superuser flag → if superuser, grant all (bypasses checks)
7. Match required: functionality + object (optional) + accessLevel
8. Return 403 Forbidden if no match
```

**Default Roles:**

| Role       | Capabilities                                         |
| ---------- | ---------------------------------------------------- |
| Common     | Read-only access across modules                      |
| Technician | Read + limited write on equipment and data           |
| Admin      | Full access including user and permission management |

Permissions support both **user-specific** (user_id set, role_id null) and **role-based** (role_id set, user_id null) assignment, allowing fine-grained overrides.

---

## MQTT Communication

### Topic Structure

```
Inbound (Device → Backend):   devices/+/status     (subscribed, QoS 0)
Outbound (Backend → Device):  devices/{deviceId}/cmd  (published via API)
```

### Message Format

```json
{
	"type": "DATA",
	"result": "OK",
	"info": {
		"logger_id": 376,
		"sensor_id": 377,
		"rtc": "2026-03-22T10:30:00",
		"ip_address": "192.168.1.100",
		"controller_serial": "SN-001",
		"controller_sw": "v1.2.3",
		"controller_hw": "v2.0",
		"controller_build_date": "2025-12-01",
		"controller_prod_date": "2025-11-15",
		"communication_sw": "v1.0.0",
		"communication_build": "2025-12-01",
		"communication_prod": "2025-11-15",
		"sht40": {
			"temperature": "23.45",
			"humidity": "55.20"
		},
		"sht40_error": 0,
		"bme280": {
			"temperature": "23.40",
			"humidity": "55.10",
			"pressure": "1013.25",
			"altitude": "125.5"
		},
		"bme280_error": 0,
		"vin": ["5.0", "3.3"]
	}
}
```

### Message Types

| Type     | Backend Action                                                                                   |
| -------- | ------------------------------------------------------------------------------------------------ |
| `DATA`   | Store sensor values in data_logs, update data_last_value, update equ_stats, emit Socket.io event |
| `STATUS` | Update equipment telemetry (equ_stats)                                                           |
| `ERROR`  | Create error_log record with device details                                                      |
| `OTA`    | Prepared for over-the-air firmware update handling                                               |

### Connection Configuration

| Parameter              | Value            |
| ---------------------- | ---------------- |
| QoS                    | 0 (at most once) |
| Keepalive              | 30 seconds       |
| Reconnect period       | 2000ms           |
| Clean session          | true             |
| Timezone normalization | Europe/Warsaw    |

---

## Hardware

### PCB Designs (KiCad)

Custom PCB designs located in `docs/PCB/PicoLogger/`:

| Board                                     | Description                              |
| ----------------------------------------- | ---------------------------------------- |
| **IoT_Logger_B_1.0**                      | Base logger board, revision B            |
| **IoT_Logger_C_1.0**                      | Base logger board, revision C            |
| **Pico_TH_Logger_SMD_PCF8563T_2.1**       | SMD temperature/humidity logger with RTC |
| **Pico_TH_Logger_Relay_SMD_PCF8563T_2.1** | SMD logger with relay output + RTC       |

### 3D-Printable Enclosures

Located in `docs/Cover/PicoLogger/`:

- **Pico_TH_Logger** — Standard logger enclosure
- **Pico_TH_Logger_Relay** — Relay variant enclosure
- **ButtonExtender.stl** — Button extender accessory

### STM32 Reference Documentation

MCU datasheets, reference manuals, and safety library guides are stored alongside each target in `logger/STM32/STM32F412RGT6/` and `logger/STM32/STM32L4/`.

---

## Getting Started

### Prerequisites

- **Node.js** (v18+)
- **MySQL** 9.x
- **MQTT Broker** (e.g., Mosquitto)
- **PlatformIO** (for STM32 firmware development)
- **Python 3** (for MicroPython device flashing)

### 1. Database Setup

```bash
# Create the database and load schema
mysql -u root -p < database/db.sql

# Load seed data (types, roles, definitions)
mysql -u root -p < database/db_data.sql
```

### 2. Backend Setup

```bash
cd backend
npm install

# Create .env file with required environment variables (see below)

# Development
npm run dev

# Production
npm run prod
```

The server starts on port `3000` (dev) or `8000` (prod) by default.

### 3. Frontend Setup

```bash
cd frontend
npm install

# Create .env file
# VITE_API_IP=http://localhost:3000

# Development
npm run dev

# Production build
npm run build:prod
```

The dev server runs at `http://localhost:5173`.

### 4. Logger Firmware

**ESP32 / MicroPython:**

1. Edit `config.py` with your WiFi, MQTT, and device IDs
2. Flash MicroPython firmware to the device
3. Upload all `.py` files to the device

**STM32:**

```bash
cd logger/STM32/application
# Build and flash via PlatformIO + ST-Link
pio run --target upload

cd ../bootloader
pio run --target upload
```

---

## Environment Variables

### Backend (`.env`)

```env
# Server Mode
NODE_ENV=development

# Database (Development)
DB_DEV=logger_dashboard
DBUSER_DEV=root
DBPASSWORD_DEV=your_password
DBHOST_DEV=localhost
DBPORT_DEV=3306

# Database (Production)
DB_PROD=logger_dashboard
DBUSER_PROD=root
DBPASSWORD_PROD=your_password
DBHOST_PROD=localhost
DBPORT_PROD=3306

# Server Ports
PORT_DEV=3000
PORT_PROD=8000

# MQTT Broker (Development)
MQTT_URL_DEV=mqtt://localhost:1883
MQTT_USER_DEV=mqtt_user
MQTT_PASS_DEV=mqtt_password

# MQTT Broker (Production)
MQTT_URL_PROD=mqtt://broker:1883
MQTT_USER_PROD=mqtt_user
MQTT_PASS_PROD=mqtt_password

# JWT Secrets
TOKEN=your_jwt_secret
TOKEN_SECRET_PERMISSION=your_permission_token_secret

# CORS (comma-separated origins)
FRONTEND_ORIGIN=http://localhost:5173

# Email (Gmail SMTP)
GMAIL_USER=your_gmail@gmail.com
GMAIL_PASSWORD=your_app_password
```

### Frontend (`.env`)

```env
VITE_API_IP=http://localhost:3000
```

### Logger (`config.py`)

```python
SSID = "your_wifi_ssid"
PASSWORD = "your_wifi_password"
NTP_SERVER_IP = "pool.ntp.org"
MQTT_SERVER = "broker_ip"
MQTT_PORT = 1883
MQTT_USER = "mqtt_user"
MQTT_PASSWORD = "mqtt_password"
LOGGER_ID = 376
SENSOR_ID = 377
SERVER_IP = "backend_ip"
SERVER_PORT = 3000
```

---

## API Reference

A Postman collection with all endpoints is available at [`docs/LoggerDashboard.postman_collection.json`](docs/LoggerDashboard.postman_collection.json).

### Endpoint Overview

| Module        | Base Path        | Auth    | Endpoints | Description                                                       |
| ------------- | ---------------- | ------- | --------- | ----------------------------------------------------------------- |
| **User**      | `/api/user`      | Partial | 8         | Registration, login, profile, password reset                      |
| **Equipment** | `/api/equipment` | Yes     | 25+       | Device CRUD — types, vendors, models, equipment, stats, logs      |
| **Data**      | `/api/data`      | Yes     | 20        | Data definitions, logs, last values, chart views                  |
| **House**     | `/api/house`     | Yes     | 16        | Locations, floors, logger placement                               |
| **Admin**     | `/api/adm`       | Yes     | 18        | Roles, permissions, functionality/object/access level definitions |
| **Common**    | `/api/common`    | Partial | 5         | Error logging                                                     |
| **Process**   | `/api/process`   | Yes     | 10        | Process types and definitions                                     |
| **MQTT**      | `/api/mqtt`      | Yes     | 1         | Send commands to devices                                          |

### All Endpoints

#### User (`/api/user`)

```
POST   /users                          # Get all users (paginated)
GET    /user/:userId                   # Get single user
POST   /user-register                  # Register new user
POST   /user-login                     # Login → JWT tokens
PATCH  /user/:userId                   # Update profile + avatar upload
DELETE /user/:userId                   # Delete user
POST   /reset-password-request         # Send password reset email
POST   /reset-password/:token          # Complete password reset
```

#### Equipment (`/api/equipment`)

```
# Types
POST   /equ-types                      # Get equipment types
GET    /equ-type/:id                   # Get single type
POST   /equ-type                       # Add type
PATCH  /equ-type/:id                   # Update type
DELETE /equ-type/:id                   # Delete type

# Vendors (same CRUD pattern as types)
POST   /equ-vendors                    # Get vendors
GET    /equ-vendor/:id                 # Get vendor
POST   /equ-vendor                     # Add vendor
PATCH  /equ-vendor/:id                 # Update vendor
DELETE /equ-vendor/:id                 # Delete vendor

# Models (same CRUD pattern)
POST   /equ-models                     # Get models
GET    /equ-model/:id                  # Get model
POST   /equ-model                      # Add model
PATCH  /equ-model/:id                  # Update model
DELETE /equ-model/:id                  # Delete model

# Equipment
POST   /equipments                     # Get all equipment
POST   /equipments-admin               # Admin view (includes soft-deleted)
GET    /equipment/:id                  # Get single equipment
POST   /equipment                      # Register new device
PATCH  /equipment/:id                  # Update device
DELETE /equipment/:id                  # Soft delete
DELETE /equipment-forced/:id           # Hard delete (permanent)
PATCH  /equipment-restore/:id          # Restore soft-deleted device

# Sensor Functions
POST   /equ-unused-loggers             # Get loggers not assigned to floors
POST   /equ-sensor-function            # Map sensor → data definition
DELETE /equ-sensor-function/:id        # Remove mapping

# Stats & Logs
POST   /equ-stats                      # Get device telemetry
GET    /equ-stat/:id                   # Get single stat
POST   /equ-stat                       # Add stat
PATCH  /equ-stat/:id                   # Update stat
DELETE /equ-stat/:id                   # Delete stat
POST   /equ-logs                       # Get equipment event logs
GET    /equ-log/:id                    # Get single log
POST   /equ-log                        # Add log
PATCH  /equ-log/:id                    # Update log
DELETE /equ-log/:id                    # Delete log
```

#### Data (`/api/data`)

```
# Definitions
POST   /data-definitions               # Get all data definitions
GET    /data-definition/:id            # Get single definition
POST   /data-definition                # Add definition
PATCH  /data-definition/:id            # Update definition
DELETE /data-definition/:id            # Delete definition

# Logs
POST   /data-logs                      # Get data logs (filtered/paginated)
GET    /data-log/:id                   # Get single log
POST   /data-log                       # Add data log
PATCH  /data-log/:id                   # Update log
DELETE /data-log/:id                   # Delete log

# Last Values
POST   /data-last-values               # Get last values
GET    /data-last-value/:id            # Get single last value
POST   /data-last-value                # Add last value
PATCH  /data-last-value/:id            # Update last value
DELETE /data-last-value/:id            # Delete last value

# Views & Special
POST   /data-last-values-view          # Latest values with metadata
POST   /data-connected-sensor-view     # Sensor network topology
POST   /data-logs-view                 # Pivoted data (temp/humidity columns)
POST   /data-last-values-multi         # Multi-logger values (no auth)
GET    /data-token                     # Short-lived data access token (1min)
```

#### House (`/api/house`)

```
# Houses
POST   /houses                         # Get all houses
GET    /house/:id                      # Get house + floors + loggers
POST   /house                          # Add house + image upload
PATCH  /house/:id                      # Update house + image
DELETE /house/:id                      # Delete house

# Floors
POST   /house-floors                   # Get all floors
GET    /house-floor/:id                # Get single floor
POST   /house-floor                    # Add floor + layout image
PATCH  /house-floor/:id                # Update floor + layout
PATCH  /house-floor-layout/:id         # Update layout position only
DELETE /house-floor/:id                # Delete floor

# Logger Placement
POST   /house-loggers                  # Get logger placements
GET    /house-logger/:id               # Get single placement
POST   /house-logger                   # Add logger to floor
PATCH  /house-logger/:id               # Update position (x, y)
DELETE /house-logger/:id               # Remove from floor
```

#### Admin (`/api/adm`)

```
# Permissions
POST   /adm-permissions                # Get permissions (user/role based)
POST   /adm-permission                 # Add permission
DELETE /adm-permission/:id             # Delete permission

# Functionality/Object/AccessLevel Definitions (same CRUD pattern each)
POST   /adm-functionality-definitions  # Get all
GET    /adm-functionality-definition/:id
POST   /adm-functionality-definition
PATCH  /adm-functionality-definition/:id
DELETE /adm-functionality-definition/:id

POST   /adm-object-definitions
GET    /adm-object-definition/:id
POST   /adm-object-definition
PATCH  /adm-object-definition/:id
DELETE /adm-object-definition/:id

POST   /adm-access-level-definitions
GET    /adm-access-level-definition/:id
POST   /adm-access-level-definition
PATCH  /adm-access-level-definition/:id
DELETE /adm-access-level-definition/:id

# Roles
POST   /adm-roles                      # Get all roles
GET    /adm-role/:id                   # Get single role
POST   /adm-role                       # Add role
PATCH  /adm-role/:id                   # Update role
DELETE /adm-role/:id                   # Delete role

# Role-User Assignments
POST   /adm-role-users                 # Get assignments
POST   /adm-role-user                  # Assign role to user
DELETE /adm-role-user/:id              # Remove assignment
```

#### Common (`/api/common`)

```
POST   /error-logs                     # Get error logs (no auth required)
GET    /error-log/:id                  # Get single error
POST   /error-log                      # Add error log
PATCH  /error-log/:id                  # Update error log
DELETE /error-log/:id                  # Delete error log
```

#### Process (`/api/process`)

```
POST   /process-types                  # Get process types
GET    /process-type/:id               # Get single type
POST   /process-type                   # Add type
PATCH  /process-type/:id               # Update type
DELETE /process-type/:id               # Delete type

POST   /process-definitions            # Get definitions
GET    /process-definition/:id         # Get single definition
POST   /process-definition             # Add definition
PATCH  /process-definition/:id         # Update definition
DELETE /process-definition/:id         # Delete definition
```

#### MQTT (`/api/mqtt`)

```
POST   /:deviceId/cmd                  # Send MQTT command to device
```

---

## Testing

Backend tests use **Jest 30.2** with **Supertest** for HTTP endpoint integration testing. Tests run with experimental VM modules for ESM support.

```bash
cd backend

# Run all tests
npm test

# Run specific test file
npx jest server/__tests__/equipment.test.js
```

**Jest configuration:**

- `--experimental-vm-modules` flag for ES modules
- `--detectOpenHandles` for async leak detection
- `--forceExit` to prevent hanging on open connections

### Test Suites

| Test File           | Coverage                                                |
| ------------------- | ------------------------------------------------------- |
| `user.test.js`      | User registration, login, profile CRUD, password reset  |
| `equipment.test.js` | Equipment types, vendors, models, devices, soft deletes |
| `data.test.js`      | Data definitions, logs, views, last values              |
| `house.test.js`     | House, floor, logger placement CRUD                     |
| `adm.test.js`       | Roles, permissions, admin definitions                   |
| `common.test.js`    | Error log CRUD                                          |
| `process.test.js`   | Process types and definitions                           |
