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

IoT Logger Dashboard is designed to monitor environmental conditions (temperature, humidity, atmospheric pressure, altitude) across multiple locations. Sensor nodes deployed in buildings collect data and transmit it via MQTT to a centralized backend. Users interact with the system through a responsive web dashboard that provides real-time charts, floor plan visualizations, equipment management, and a full role-based administration panel.

**Key metrics:**

- 380+ tracked devices (loggers, sensors, relays)
- 247,000+ recorded measurements
- 12 measurement parameter types
- Multi-location, multi-floor support

---

## Architecture

```
┌──────────────────┐      MQTT       ┌──────────────────┐     HTTP/WS     ┌──────────────────┐
│   Sensor Nodes   │ ──────────────► │     Backend      │ ◄────────────── │    Frontend      │
│                  │                 │                  │ ──────────────► │                  │
│  ESP32 + STM32   │                 │  Express.js API  │   Socket.io     │  React + MUI     │
│  MicroPython     │                 │  Sequelize ORM   │                 │  Redux Toolkit   │
│  SHT40 / BME280  │                 │  MQTT Client     │                 │  ECharts         │
└──────────────────┘                 │  Socket.io       │                 └──────────────────┘
                                     └────────┬─────────┘
                                              │
                                     ┌────────▼─────────┐
                                     │     MySQL DB     │
                                     │  13 tables       │
                                     │  4 views         │
                                     └──────────────────┘
```

---

## Technology Stack

### Backend

| Technology        | Version    | Purpose                                 |
| ----------------- | ---------- | --------------------------------------- |
| Node.js           | ES Modules | Runtime                                 |
| Express.js        | 5.2        | HTTP framework                          |
| Sequelize         | 6.37       | MySQL ORM                               |
| Socket.io         | 4.8        | Real-time WebSocket communication       |
| MQTT.js           | 5.14       | IoT message broker client               |
| JSON Web Token    | 9.0        | Authentication                          |
| express-validator | 7.3        | Request validation                      |
| Sharp             | 0.34       | Image processing (avatars, floor plans) |
| Nodemailer        | 7.0        | Email (password reset)                  |
| Jest + Supertest  | 30.2       | Testing                                 |

### Frontend

| Technology          | Version | Purpose                     |
| ------------------- | ------- | --------------------------- |
| React               | 19.2    | UI framework                |
| TypeScript          | 5.9     | Type safety                 |
| Vite                | 7.3     | Build tool (SWC transpiler) |
| Material UI (MUI)   | 7.3     | Component library           |
| Redux Toolkit       | 2.11    | State management            |
| React Router        | 7.13    | Client-side routing         |
| Socket.io Client    | 4.8     | Real-time updates           |
| ECharts             | 3.0     | Data visualization / charts |
| MUI X Data Grid     | 8.26    | Advanced data tables        |
| XYFlow (React Flow) | 12.10   | Graph / flow diagrams       |
| xlsx + file-saver   | —       | Data export to Excel        |

### Embedded / Logger

| Technology                        | Purpose                             |
| --------------------------------- | ----------------------------------- |
| ESP32 (MicroPython)               | Main IoT gateway with WiFi + MQTT   |
| STM32L476RG / STM32F412RG (CMSIS) | Low-level sensor hub (bare-metal C) |
| MicroPython (generic)             | Standalone sensor nodes             |
| PlatformIO                        | STM32 build system                  |
| KiCad                             | PCB design                          |

### Sensors

| Sensor        | Measurements                                          |
| ------------- | ----------------------------------------------------- |
| SHT30 / SHT40 | Temperature (-40–125°C), Humidity (0–100%)            |
| BME280        | Temperature, Humidity, Atmospheric Pressure, Altitude |
| PCF8563T      | Real-Time Clock (RTC)                                 |

---

## Features

- **Real-Time Monitoring** — Live sensor data via MQTT → WebSocket pipeline
- **Interactive Charts** — Time-series visualization with ECharts, range selection, and export
- **Floor Plan Visualization** — Place loggers on uploaded floor plan images with x/y positioning
- **Multi-Location Support** — Organize devices by house, floor, and room
- **Equipment Inventory** — Full CRUD management for loggers, sensors, relays, vendors, and models
- **Sensor Capability Mapping** — Define which data parameters each sensor can measure
- **Role-Based Access Control** — Granular permissions: functionality → object → access level (READ/WRITE/DELETE)
- **User Management** — Registration, login, JWT authentication, password reset via email
- **Data Export** — Export measurements to XLSX spreadsheets
- **Error Tracking** — Log and categorize device errors by severity (Critical/Error/Warning/Info)
- **Device Telemetry** — Track firmware version, hardware info, IP address, last seen timestamp
- **Process Management** — Define and manage operational processes
- **Audit Trail** — `created_by` / `updated_by` tracking on all entities
- **OTA Support** — Over-the-air firmware update infrastructure for STM32 via bootloader

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

The backend is a Node.js Express server with ES module support. It provides a RESTful API for all CRUD operations, handles MQTT message ingestion from IoT devices, and broadcasts real-time updates to connected clients via Socket.io.

**Key responsibilities:**

- REST API for frontend communication
- MQTT subscription and sensor data processing
- JWT-based authentication and permission enforcement
- File uploads (avatars, floor plan images) via multer + Sharp
- Standardized response formatting
- Email dispatch for password reset flows

### MQTT Data Pipeline

When sensor data arrives over MQTT:

1. Message is parsed from the `devices/+/status` topic
2. Timestamps are normalized to `Europe/Warsaw` timezone
3. Equipment `last_seen` status is updated
4. Sensor readings are stored in `data_logs`
5. `data_last_value` cache is refreshed
6. Socket.io broadcasts the update to all connected clients

---

## Frontend

### React SPA

The frontend is a single-page application built with React 19, TypeScript, and Material UI. It uses Redux Toolkit for state management and RTK Query for API calls.

**Module views:**

| Module        | Description                                                                    |
| ------------- | ------------------------------------------------------------------------------ |
| **Dashboard** | Real-time overview of all connected devices and latest readings                |
| **Data**      | Interactive charts (ECharts), pivot tables, data export, layout visualization  |
| **Equipment** | CRUD management for devices (loggers, sensors, relays), vendors, models, types |
| **House**     | Location management with floor plans, logger placement on floor maps           |
| **Admin**     | User management, role assignment, granular permission configuration            |
| **Process**   | Process type & definition management                                           |
| **User**      | User profile, avatar upload                                                    |

---

## Logger Firmware

### ESP32 (MicroPython)

The ESP32 acts as the main IoT gateway. On boot it:

1. Connects to WiFi
2. Synchronizes time via NTP
3. Initializes MQTT connection to the broker
4. Establishes UART communication with the STM32
5. Enters a loop publishing status and sensor data

**GPIO Configuration:**
| Pin | Function |
|---|---|
| 18 | Status LED |
| 19 | STM32 control |
| 22 | Power LED |

### STM32 (Bare-Metal C / CMSIS)

The STM32 handles low-level sensor communication using DMA for efficient peripheral access:

- **Target MCUs:** STM32L476RG, STM32F412RGT6
- **Build System:** PlatformIO with CMSIS framework
- **Peripherals:** I2C (SHT40, BME280, PCF8563T RTC), SPI, ADC, UART, LCD
- **Bootloader:** Custom UART bootloader at `0x08000000`, application loaded at `0x08008000`
- **DMA Drivers:** I2C, SPI, UART, BME280, PCF8563T — all use DMA for non-blocking data transfer

### MicroPython Sensor Nodes

Standalone nodes that periodically read sensors and POST data directly to the backend API:

- **Sensors:** SHT30/SHT40, BME280
- **Display:** I2C LCD
- **Peripherals:** RGB LEDs, buzzer, relay outputs (4 channels), push buttons
- **Thresholds:** Configurable alerts (e.g., temperature 20–25°C, humidity 30–80%)
- **Post Interval:** 600 seconds (configurable)

---

## Database

MySQL 9.1 database with 13 tables and 4 views.

### Entity-Relationship Overview

```
┌───────────┐    ┌───────────────┐    ┌─────────────┐
│   users   │───►│ adm_roles_user│◄───│  adm_roles  │
└───────────┘    └───────────────┘    └─────────────┘
      │                                      │
      ▼                                      ▼
┌─────────────────┐              ┌──────────────────────┐
│ adm_permissions  │◄────────────│ adm_functionality_def│
│ (user/role →     │              └──────────────────────┘
│  func → obj →    │              ┌──────────────────────┐
│  access level)   │◄────────────│ adm_object_definition│
└─────────────────┘              └──────────────────────┘
                                 ┌──────────────────────┐
                                 │ adm_access_level_def │
                                 └──────────────────────┘

┌───────────┐    ┌─────────────┐    ┌────────────┐
│ equ_type  │◄───│equ_equipment│───►│ equ_vendor │
└───────────┘    └──────┬──────┘    └────────────┘
                        │           ┌────────────┐
                        └──────────►│ equ_model  │
                        │           └────────────┘
                        ▼
              ┌─────────────────┐
              │   equ_stats     │  (last_seen, firmware, IP)
              │   equ_log       │  (STATUS, DATA, OTA, ERROR)
              │   error_log     │  (Critical → Info)
              └─────────────────┘

┌─────────────────┐    ┌──────────────┐    ┌──────────────┐
│ data_definitions│◄───│  data_logs   │───►│equ_equipment │
└─────────────────┘    └──────────────┘    └──────────────┘
                       ┌──────────────┐
                       │data_last_value│
                       └──────────────┘

┌─────────────┐    ┌──────────────┐    ┌──────────────┐
│ house_house │───►│ house_floors │───►│ house_logger │
└─────────────┘    └──────────────┘    └──────────────┘
```

### Key Tables

| Table              | Purpose                                                              |
| ------------------ | -------------------------------------------------------------------- |
| `users`            | User accounts (username, email, password hash, avatar)               |
| `equ_equipment`    | Device registry (serial number, vendor, model, type)                 |
| `equ_stats`        | Device telemetry (last seen, firmware, hardware, IP)                 |
| `equ_log`          | Equipment event logs (STATUS, DATA, OTA, ERROR)                      |
| `data_definitions` | Measurement types (temperature, humidity, pressure, altitude)        |
| `data_logs`        | Raw sensor measurements (timestamp, value, logger_id, sensor_id)     |
| `data_last_value`  | Cached latest readings per logger-sensor-parameter                   |
| `house_house`      | Properties / locations                                               |
| `house_floors`     | Building floors with layout images                                   |
| `house_logger`     | Logger placement on floor plans (x/y coordinates)                    |
| `adm_roles`        | Role templates (Admin, Editor, Common)                               |
| `adm_permissions`  | Permission rules (user/role → functionality → object → access level) |
| `error_log`        | System & device errors with severity levels                          |

### Database Views

| View                         | Purpose                                                          |
| ---------------------------- | ---------------------------------------------------------------- |
| `adm_view_permission`        | Resolved permission matrix                                       |
| `data_view_last_value`       | Latest sensor values with full metadata                          |
| `data_view_logs`             | Pivoted data (temperature, humidity, pressure, altitude columns) |
| `data_view_connected_sensor` | Sensor-logger-floor relationship graph                           |
| `equ_view_unused_logger`     | Loggers not yet assigned to a floor                              |

---

## Authentication & Authorization

### JWT Token System

| Type | Expiry   | Purpose                            |
| ---- | -------- | ---------------------------------- |
| 0    | 5 hours  | Standard authentication            |
| 1    | 365 days | "Remember me" persistent session   |
| 2    | 5 hours  | Permission token (separate secret) |
| 3    | 1 minute | Short-lived data access token      |

Protected routes require a `Bearer` token in the `Authorization` header. The middleware validates the token and attaches user info to the request object.

### Role-Based Access Control (RBAC)

Permissions are defined as a three-dimensional matrix:

```
Permission = Functionality × Object × Access Level
```

- **Functionality** — module scope: `equ` (Equipment), `house` (House), `data` (Data), `adm` (Admin)
- **Object** — resource type: `equType`, `equVendor`, `equEquipment`, `houseHouse`, `houseFloor`, etc.
- **Access Level** — operation: `READ`, `WRITE`, `DELETE`

**Default Roles:**

| Role   | Capabilities                                         |
| ------ | ---------------------------------------------------- |
| Common | Read-only access across modules                      |
| Editor | Read + Write on equipment, house, and data           |
| Admin  | Full access including user and permission management |

---

## MQTT Communication

### Topic Structure

```
devices/{deviceId}/status
```

### Message Format

```json
{
	"type": "STATUS | DATA | ERROR",
	"info": {
		"logger_id": 376,
		"sensor_id": 377,
		"rtc": "2026-03-08 12:34:56",
		"sht40": {
			"temperature": 23.5,
			"humidity": 45.2
		},
		"bme280": {
			"pressure": 1013.25,
			"atmPressure": 1012.0,
			"altitude": 120.5
		}
	}
}
```

### Configuration

| Parameter | Development      | Production    |
| --------- | ---------------- | ------------- |
| QoS       | 0 (at most once) | 0             |
| Keepalive | 30s              | 30s           |
| Reconnect | 5s interval      | 5s interval   |
| Timezone  | Europe/Warsaw    | Europe/Warsaw |

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
TOKEN_SECRET_PERMISSION=your_permission_secret

# CORS
FRONTEND_ORIGIN=http://localhost:5173

# Email (Nodemailer)
EMAIL_HOST=smtp.example.com
EMAIL_PORT=587
EMAIL_USER=noreply@example.com
EMAIL_PASS=email_password
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

| Module        | Base Path        | Auth    | Description                                          |
| ------------- | ---------------- | ------- | ---------------------------------------------------- |
| **User**      | `/api/user`      | Partial | Registration, login, profile, password reset         |
| **Equipment** | `/api/equipment` | Yes     | Device CRUD — types, vendors, models, equipment      |
| **Data**      | `/api/data`      | Yes     | Data definitions, logs, last values, chart views     |
| **House**     | `/api/house`     | Yes     | Locations, floors, logger placement                  |
| **Admin**     | `/api/adm`       | Yes     | Roles, permissions, functionality/object definitions |
| **Common**    | `/api/common`    | Yes     | Error logging                                        |
| **Process**   | `/api/process`   | Yes     | Process types and definitions                        |
| **MQTT**      | `/api/mqtt`      | Yes     | Send commands to devices                             |

### Key Endpoints

```
POST   /api/user/user-register           # Register new user
POST   /api/user/user-login              # Login → JWT token
POST   /api/user/reset-password-request  # Request password reset email
POST   /api/user/reset-password/:token   # Complete password reset

POST   /api/equipment/equipments         # List all equipment
POST   /api/equipment/equipment          # Create device
GET    /api/equipment/equipment/:id      # Get device details
PATCH  /api/equipment/equipment/:id      # Update device
DELETE /api/equipment/equipment/:id      # Delete device

POST   /api/data/data-logs               # Query sensor data logs
POST   /api/data/data-logs-view          # Pivoted data view
POST   /api/data/data-last-values-view   # Latest values view
GET    /api/data/data-token              # Short-lived data access token

POST   /api/house/houses                 # List locations
POST   /api/house/house                  # Create location
POST   /api/house/floors                 # List floors
POST   /api/house/house-loggers          # Logger placement

POST   /api/mqtt/:deviceId/cmd           # Send MQTT command to device
```

---

## Testing

Backend tests use **Jest** with **Supertest** for HTTP endpoint integration testing.

```bash
cd backend

# Run all tests
npm test

# Run specific test file
npx jest server/__tests__/equipment.test.js
```

### Test Suites

| Test File           | Coverage                                  |
| ------------------- | ----------------------------------------- |
| `user.test.js`      | User registration, login, profile CRUD    |
| `equipment.test.js` | Equipment types, vendors, models, devices |
| `data.test.js`      | Data definitions, logs, views             |
| `house.test.js`     | House, floor, logger placement            |
| `adm.test.js`       | Roles, permissions, admin operations      |
| `common.test.js`    | Error logging                             |
| `process.test.js`   | Process types and definitions             |

---

## License

_Not specified._
