#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"

#define SHT30_ADDR 0x44

// IDF 5.2+ rename helperów (migration guide)
#if defined(i2c_master_transmit_receive)
  #define I2C_WRRD(port, addr, wbuf, wlen, rbuf, rlen, to) \
          i2c_master_transmit_receive((port), (addr), (wbuf), (wlen), (rbuf), (rlen), (to))
#else
  #define I2C_WRRD(port, addr, wbuf, wlen, rbuf, rlen, to) \
          i2c_master_write_read_device((port), (addr), (wbuf), (wlen), (rbuf), (rlen), (to))
#endif

static bool sht30_read(float *t_c, float *rh) {
    uint8_t cmd[2] = {0x2C, 0x06};
    esp_err_t err = i2c_master_write_to_device(I2C_PORT, SHT30_ADDR, cmd, sizeof(cmd), pdMS_TO_TICKS(100));
    if (err != ESP_OK) return false;

    vTaskDelay(pdMS_TO_TICKS(15));

    uint8_t buf[6];
    // Alternatywa: możesz zrobić i2c_master_read_from_device(), ale tu pokazuję “write-read helper”
    err = I2C_WRRD(I2C_PORT, SHT30_ADDR, NULL, 0, buf, sizeof(buf), pdMS_TO_TICKS(200));
    // ^ jeśli Twoja wersja helpera nie lubi NULL/0, użyj i2c_master_read_from_device() zamiast tego.
    if (err != ESP_OK) {
        err = i2c_master_read_from_device(I2C_PORT, SHT30_ADDR, buf, sizeof(buf), pdMS_TO_TICKS(200));
        if (err != ESP_OK) return false;
    }

    uint16_t t_raw  = ((uint16_t)buf[0] << 8) | buf[1];
    uint16_t rh_raw = ((uint16_t)buf[3] << 8) | buf[4];

    *t_c = -45.0f + 175.0f * ((float)t_raw / 65535.0f);
    *rh  = 100.0f * ((float)rh_raw / 65535.0f);
    return true;
}

void app_main(void) {
    i2c_init_master();

    while (1) {
        float t, h;
        if (sht30_read(&t, &h)) {
            printf("T=%.2fC RH=%.1f%%\n", t, h);
        } else {
            printf("SHT30 error\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}