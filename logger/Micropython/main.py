import machine
from machine import Pin, I2C, Timer, PWM, ADC
import time
import network
import ntptime
from config import Config
from rtc_clock import RTC_Clock
from i2c_lcd import I2cLcd
from sht30 import SHT30
from sht40 import SHT40
from bme280_i2c import BME280
import urequests as requests
import json
import gc

cfg = Config()
i = 0

debounce_delay_ms = 50
LONG_PRESS_MS = 3000

backlight_off_time = 20
backlight_on = True

btn1_dirty = False
btn1_pressed = False
btn1_press_start = None
btn1_long_fired = False
btn1_last_stable_change = 0
btn2_dirty = False
btn2_pressed = False
btn2_press_start = None
btn2_long_fired = False
btn2_last_stable_change = 0
long_press_reset = False


def network_connection():
    ssid = cfg.get_ssid()
    password = cfg.get_password()

    station = network.WLAN(network.STA_IF)
    station.active(True)
    station.connect(ssid, password)

    timeout = 0
    while station.isconnected() == False and timeout < 20:
        time.sleep(1)
        timeout += 1

    if station.isconnected():
        try:
            ntptime.settime()
        except Exception as e:
            print("NTP time sync error:", e)
    else:
        print("WiFi connection failed!")


def set_time(clock):
    try:
        aT = time.localtime(time.time() + 1 * 60 * 60)
        year = aT[0]
        month = aT[1]
        day = aT[2]
        hour = aT[3]
        minute = aT[4]
        second = aT[5]
        clock.set_time((year, month, day, 0, hour, minute, second))
    except Exception as e:
        send_error_log("Failed to set RTC time", str(e))


def send_error_log(message, details=None):
    try:
        payload = {
            "equipmentId": cfg.get_logger_id(),
            "message": message,
            "details": details,
            "severity": "error",
            "type": "Equipment",
        }
        try:
            resp = requests.post(
                f"{cfg.get_base_url()}{cfg.get_error_url()}",
                json=payload,
                timeout=3,
            )
        finally:
            if resp:
                resp.close()
    except Exception as e:
        print("Cannot send message to backend:", e)


def map_color(color):
    return int(color * 65535 / 255)


def set_color(leds, red, green, blue):
    try:
        leds[0].duty_u16(map_color(red))
        leds[1].duty_u16(map_color(green))
        leds[2].duty_u16(map_color(blue))
    except Exception as e:
        print("Cannot set LED:", e)


def button1_irq(pin):
    global btn1_dirty
    btn1_dirty = True


def button2_irq(pin):
    global btn2_dirty
    btn2_dirty = True


def button1_handler():
    global backlight_on, backlight_off_time
    if not backlight_on:
        cfg.set_backlight_flag(2)
    else:
        cfg.set_backlight_flag(1)


def button1_long_handler():
    global long_press_reset
    long_press_reset = True


def button2_handler():
    print("Button 2 pressed")


def button2_long_handler():
    cfg.set_logging_enabled(not cfg.is_logging_enabled())


def update_button(which, pin):
    global debounce_delay_ms, LONG_PRESS_MS, btn1_dirty, btn1_pressed, btn1_press_start, btn1_long_fired, btn1_last_stable_change
    global btn2_dirty, btn2_pressed, btn2_press_start, btn2_long_fired, btn2_last_stable_change
    now = time.ticks_ms()
    if which == 1:
        dirty = "btn1_dirty"
        pressed = "btn1_pressed"
        start = "btn1_press_start"
        fired = "btn1_long_fired"
        last = "btn1_last_stable_change"
        on_short = button1_handler
        on_long = button1_long_handler
    else:
        dirty = "btn2_dirty"
        pressed = "btn2_pressed"
        start = "btn2_press_start"
        fired = "btn2_long_fired"
        last = "btn2_last_stable_change"
        on_short = button2_handler
        on_long = button2_long_handler
    need_check = (
        globals()[dirty] or time.ticks_diff(now, globals()[last]) >= debounce_delay_ms
    )
    if need_check:
        raw_pressed = pin.value() == 0
        if (
            raw_pressed != globals()[pressed]
            and time.ticks_diff(now, globals()[last]) >= debounce_delay_ms
        ):
            globals()[last] = now
            globals()[pressed] = raw_pressed
            if raw_pressed:
                globals()[start] = now
                globals()[fired] = False
            else:
                ps = globals()[start]
                lf = globals()[fired]
                if ps is not None and not lf:
                    dur = time.ticks_diff(now, ps)
                    if dur >= debounce_delay_ms:
                        try:
                            on_short()
                        except Exception as e:
                            print("Button short callback error:", e)
                globals()[start] = None
                globals()[fired] = False
        globals()[dirty] = False
    if globals()[pressed] and globals()[start] is not None and not globals()[fired]:
        held = time.ticks_diff(now, globals()[start])
        if held >= LONG_PRESS_MS:
            globals()[fired] = True
            try:
                on_long()
            except Exception as e:
                print("Button long callback error:", e)


def main():
    sda_pin = 0
    scl_pin = 1
    I2C_ADDR = 0x27
    totalRows = 2
    totalColumns = 16
    conversion_factor = 3.3 / (65535)
    gain = (100000 + 100000) / 100000

    i2c = I2C(0, scl=Pin(scl_pin), sda=Pin(sda_pin), freq=100000)
    lcd = I2cLcd(i2c, I2C_ADDR, totalRows, totalColumns)
    lcd.move_to(0, 0)
    lcd.putstr("Logger ready")
    lcd.move_to(0, 1)
    lcd.putstr("Initializing...")
    network_connection()

    pins = cfg.get_led_pins()
    leds = [PWM(Pin(pin)) for pin in pins]
    for led in leds:
        led.freq(1000)
    adc_pin = Pin(26, mode=Pin.IN)
    adc = ADC(adc_pin)
    clock = RTC_Clock(i2c)
    if cfg.is_set_time_enabled():
        set_time(clock)
    if cfg.get_sht_type() == 30:
        sensor = SHT30(i2c)
    elif cfg.get_sht_type() == 40:
        sensor = SHT40(i2c)
    elif cfg.get_sht_type() == 0:
        sensor = BME280(i2c)
    set_color(leds, 0, 255, 255)
    switch1 = Pin(cfg.get_switch_pins()[0], Pin.IN, Pin.PULL_UP)
    switch2 = Pin(cfg.get_switch_pins()[1], Pin.IN, Pin.PULL_UP)
    switch1.irq(
        trigger=Pin.IRQ_FALLING,
        handler=button1_irq,
    )
    switch2.irq(
        trigger=Pin.IRQ_FALLING,
        handler=button2_irq,
    )
    time.sleep(2)
    lcd.clear()

    def buttons_tick(_t):
        update_button(1, switch1)
        update_button(2, switch2)

    def timer_callback(timer):
        global backlight_off_time, backlight_on, i, long_press_reset
        error = False
        if cfg.get_backlight_flag() == 2:
            lcd.backlight_on()
            backlight_off_time = 20
            backlight_on = True
            cfg.set_backlight_flag(0)
        elif cfg.get_backlight_flag() == 1:
            backlight_off_time = 0
            backlight_on = False
            cfg.set_backlight_flag(0)
            lcd.backlight_off()
        if backlight_on and backlight_off_time <= 0:
            lcd.backlight_off()
            backlight_on = False
        try:
            batt_raw = adc.read_u16()
            batt_v = batt_raw * conversion_factor * gain
            current_time = clock.read_time()
            if i % 2 == 0:
                set_color(leds, 0, 0, 0)
                formated_time = "{}-{:02d}-{:02d} {:02d}:{:02d}".format(
                    current_time[0],
                    current_time[1],
                    current_time[2],
                    current_time[3],
                    current_time[4],
                    current_time[5],
                )
            elif i % 2 != 0:
                if current_time[3] <= 20 and current_time[3] >= 8:
                    if cfg.is_logging_enabled() and batt_v >= 1.5:
                        set_color(leds, 0, 255, 0)
                    elif batt_v < 1.5:
                        set_color(leds, 255, 255, 0)
                    else:
                        set_color(leds, 255, 255, 255)
                formated_time = "{}-{:02d}-{:02d} {:02d} {:02d}".format(
                    current_time[0],
                    current_time[1],
                    current_time[2],
                    current_time[3],
                    current_time[4],
                    current_time[5],
                )
                if batt_v < 1.5:
                    formated_time = "LOW BATT.       "
        except Exception as e:
            send_error_log("RTC read error", str(e))
            error = True
        try:
            temperature = sensor.temperature()
            humidity = sensor.relative_humidity()
            if cfg.is_pressure_enabled() == 1:
                pressure = sensor.atm_pressure()
            else:
                pressure = None
        except Exception as e:
            send_error_log("Sensor read error", str(e))
            error = True
        try:
            lcd.move_to(0, 0)
            lcd.putstr(
                "T/H: {}C {}% ".format(round(temperature, 1), round(humidity, 1))
            )
            lcd.move_to(0, 1)
            lcd.putstr(formated_time)
        except Exception as e:
            send_error_log("LCD display error", str(e))
            error = True
        if i == cfg.get_post_time():
            data = []
            # 1 cycle = 1s
            # 10 cycles = 10s
            # 600 cycles = 10min
            try:
                time_send = "{}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}".format(
                    current_time[0],
                    current_time[1],
                    current_time[2],
                    current_time[3],
                    current_time[4],
                    current_time[5],
                )
                temp_send = round(temperature, 2)
                hum_send = round(humidity, 2)
                if cfg.is_pressure_enabled() == 1 and pressure is not None:
                    pressure_send = round(pressure, 2)
                if cfg.is_temperature_enabled() == 1 and temp_send is not None:
                    temp_entry = {
                        "time": time_send,
                        "value": temp_send,
                        "definition": "temperature",
                        "equLoggerId": cfg.get_logger_id(),
                        "equSensorId": cfg.get_sensor_id(),
                    }
                    data.append(temp_entry)
                if cfg.is_humidity_enabled() == 1 and hum_send is not None:
                    hum_entry = {
                        "time": time_send,
                        "value": hum_send,
                        "definition": "humidity",
                        "equLoggerId": cfg.get_logger_id(),
                        "equSensorId": cfg.get_sensor_id(),
                    }
                    data.append(hum_entry)
                if cfg.is_pressure_enabled() == 1 and pressure_send is not None:
                    pressure_entry = {
                        "time": time_send,
                        "value": pressure_send,
                        "definition": "atmPressure",
                        "equLoggerId": cfg.get_logger_id(),
                        "equSensorId": cfg.get_sensor_id(),
                    }
                    data.append(pressure_entry)
            except Exception as e:
                send_error_log("Preparing data fail", str(e))
                error = True
            if cfg.is_logging_enabled() and not error and len(data) > 0:
                try:
                    resp = requests.get(
                        f"{cfg.get_base_url()}{cfg.get_token_url()}",
                        timeout=0.5,
                    )
                    try:
                        token = json.loads(resp.text)["token"]
                    finally:
                        resp.close()
                    resp2 = requests.post(
                        f"{cfg.get_base_url()}{cfg.get_data_url()}",
                        headers={"Authorization": "Bearer {}".format(token)},
                        json=data,
                        timeout=0.5,
                    )
                    try:
                        status_code = resp2.status_code
                        if status_code not in (200, 201):
                            send_error_log(
                                "Data post error",
                                f"Status code: {status_code}, data could not be send",
                            )
                    finally:
                        resp2.close()
                        gc.collect()
                except Exception as e:
                    send_error_log("Wrong data format", str(e))
            i = 0
        i += 1
        backlight_off_time -= 1
        if backlight_off_time < 0:
            backlight_off_time = 0
        if long_press_reset:
            long_press_reset = False
            machine.reset()

    logger_timer = Timer()
    logger_timer.init(mode=Timer.PERIODIC, period=1000, callback=timer_callback)
    btn_timer = Timer(-1)
    btn_timer.init(mode=Timer.PERIODIC, period=10, callback=buttons_tick)


if __name__ == "__main__":
    main()

