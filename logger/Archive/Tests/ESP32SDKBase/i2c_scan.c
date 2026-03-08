#include <stdio.h>
#include "driver/i2c.h"

static esp_err_t i2c_probe_addr(uint8_t addr7) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr7 << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    return err;
}

void app_main(void) {
    i2c_init_master();

    while (1) {
        printf("I2C scan:\n");
        for (int a = 1; a < 127; a++) {
            if (i2c_probe_addr(a) == ESP_OK) {
                printf("  found 0x%02X\n", a);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}