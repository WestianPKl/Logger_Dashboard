#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "cJSON.h"
#include "gpio.h"
#include "mqtt.h"
#include "wifi.h"
#include "app_flags.h"
#include "uart.h"
#include "support.h"

void decode_response(const uint8_t *req);

void process_mqtt_command_json(const char *data, int data_len)
{
    char json_buf[256];

    if (data_len <= 0 || data_len >= (int)sizeof(json_buf)) {
        mqtt_publish_error_json("CMD", "MQTT payload too long");
        return;
    }

    memcpy(json_buf, data, data_len);
    json_buf[data_len] = '\0';

    cJSON *root = cJSON_Parse(json_buf);
    if (root == NULL) {
        mqtt_publish_error_json("CMD", "Invalid JSON");
        return;
    }

    cJSON *cmd_item = cJSON_GetObjectItemCaseSensitive(root, "cmd");
    cJSON *params_item = cJSON_GetObjectItemCaseSensitive(root, "params");

    if (!cJSON_IsString(cmd_item) || cmd_item->valuestring == NULL) {
        cJSON_Delete(root);
        mqtt_publish_error_json("CMD", "Missing cmd");
        return;
    }

    const char *cmd = cmd_item->valuestring;

    if (strcmp(cmd, "PING") == 0) {
        if (send_frame_to_stm32(0x00, 0x00, 0x00, NULL, 0) != ESP_OK) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Failed to send PING");
            return;
        }
    }
    else if (strcmp(cmd, "SYNC_TIME") == 0) {
        uint8_t datetime[7];
        time_t now = 0;
        time(&now);
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
 
        datetime[0] = (uint8_t)(timeinfo.tm_year + 1900 - 2000);
        datetime[1] = (uint8_t)(timeinfo.tm_mon + 1);
        datetime[2] = (uint8_t)timeinfo.tm_mday;
        datetime[3] = (uint8_t)timeinfo.tm_wday;
        datetime[4] = (uint8_t)timeinfo.tm_hour;
        datetime[5] = (uint8_t)timeinfo.tm_min;
        datetime[6] = (uint8_t)timeinfo.tm_sec;

        if (send_frame_to_stm32(0x00, 0x06, 0x01, datetime, sizeof(datetime)) != ESP_OK) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Failed to send SYNC_TIME");
            return;
        }
    }
    else if (strcmp(cmd, "READ_RTC") == 0) {
        if (send_frame_to_stm32(0x00, 0x06, 0x00, NULL, 0) != ESP_OK) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Failed to send READ_RTC");
            return;
        }
    }
    else if (strcmp(cmd, "READ_INPUTS") == 0) {
        if (!cJSON_IsObject(params_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing params for READ_INPUTS");
            return;
        }

        cJSON *channel_item = cJSON_GetObjectItemCaseSensitive(params_item, "channel");

        if (!cJSON_IsString(channel_item) || channel_item->valuestring == NULL) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing input channel");
            return;
        }

        uint8_t channel_id = 0;
        if (!find_input_channel_id(channel_item->valuestring, &channel_id)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Unknown input channel");
            return;
        }

        if (send_frame_to_stm32(0x00, 0x02, channel_id, NULL, 0) != ESP_OK) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Failed to send READ_INPUTS");
            return;
        }
    }
    else if (strcmp(cmd, "SET_OUTPUT") == 0) {
        if (!cJSON_IsObject(params_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing params for SET_OUTPUT");
            return;
        }

        cJSON *channel_item = cJSON_GetObjectItemCaseSensitive(params_item, "channel");
        cJSON *value_item = cJSON_GetObjectItemCaseSensitive(params_item, "value");

        if (!cJSON_IsString(channel_item) || channel_item->valuestring == NULL) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing output channel");
            return;
        }

        if (!cJSON_IsNumber(value_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing output value");
            return;
        }

        int value = value_item->valueint;
        if (value != 0 && value != 1) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Output value must be 0 or 1");
            return;
        }

        uint8_t channel_id = 0;
        if (!find_output_channel_id(channel_item->valuestring, &channel_id)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Unknown output channel");
            return;
        }

        uint8_t payload[1];
        payload[0] = (uint8_t)value;

        if (send_frame_to_stm32(0x00, 0x04, channel_id, payload, sizeof(payload)) != ESP_OK) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Failed to send SET_OUTPUT");
            return;
        }
    }
    else if (strcmp(cmd, "SET_PWM") == 0) {
        if (!cJSON_IsObject(params_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing params for SET_PWM");
            return;
        }

        cJSON *channel_item = cJSON_GetObjectItemCaseSensitive(params_item, "channel");
        cJSON *value_item = cJSON_GetObjectItemCaseSensitive(params_item, "duty_cycle");

        if (!cJSON_IsString(channel_item) || channel_item->valuestring == NULL) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing pwm channel");
            return;
        }

        if (!cJSON_IsNumber(value_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing duty cycle");
            return;
        }

        int value = value_item->valueint;
        if (value < 0 || value > 100) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Duty cycle must be between 0 and 100");
            return;
        }

        uint8_t channel_id = 0;
        if (!find_pwm_channel_id(channel_item->valuestring, &channel_id)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Unknown pwm channel");
            return;
        }

        uint8_t payload[1];
        payload[0] = (uint8_t)value;

        if (send_frame_to_stm32(0x00, 0x05, channel_id, payload, sizeof(payload)) != ESP_OK) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Failed to send SET_PWM");
            return;
        }
    }
    else if (strcmp(cmd, "RGB") == 0) {
        if (!cJSON_IsObject(params_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing params for RGB");
            return;
        }

        cJSON *red_item = cJSON_GetObjectItemCaseSensitive(params_item, "r");
        cJSON *green_item = cJSON_GetObjectItemCaseSensitive(params_item, "g");
        cJSON *blue_item = cJSON_GetObjectItemCaseSensitive(params_item, "b");
        cJSON *brightness_item = cJSON_GetObjectItemCaseSensitive(params_item, "brightness");

        if (!cJSON_IsNumber(red_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing red channel");
            return;
        }

        if (!cJSON_IsNumber(green_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing green channel");
            return;
        }

        if (!cJSON_IsNumber(blue_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing blue channel");
            return;
        }

        if (!cJSON_IsNumber(brightness_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing brightness channel");
            return;
        }

        int red_value = red_item->valueint;
        if (red_value < 0 || red_value > 255) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Red channel must be between 0 and 255");
            return;
        }

        int green_value = green_item->valueint;
        if (green_value < 0 || green_value > 255) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Green channel must be between 0 and 255");
            return;
        }

        int blue_value = blue_item->valueint;
        if (blue_value < 0 || blue_value > 255) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Blue channel must be between 0 and 255");
            return;
        }

        int brightness_value = brightness_item->valueint;
        if (brightness_value < 0 || brightness_value > 100) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Brightness channel must be between 0 and 100");
            return;
        }

        uint8_t payload[4];
        payload[0] = (uint8_t)red_value;
        payload[1] = (uint8_t)green_value;
        payload[2] = (uint8_t)blue_value;
        payload[3] = (uint8_t)brightness_value;

        if (send_frame_to_stm32(0x00, 0x05, 0x05, payload, sizeof(payload)) != ESP_OK) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Failed to send RGB");
            return;
        }
    }
    else if (strcmp(cmd, "BUZZER") == 0) {
        if (!cJSON_IsObject(params_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing params for BUZZER");
            return;
        }

        cJSON *freq_item = cJSON_GetObjectItemCaseSensitive(params_item, "freq");
        cJSON *vol_item = cJSON_GetObjectItemCaseSensitive(params_item, "volume");

        if (!cJSON_IsNumber(freq_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing frequency");

            return;
        }

        if (!cJSON_IsNumber(vol_item)) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Missing volume");
            return;
        }

        int freq_value = freq_item->valueint;
        if (freq_value < 0 || freq_value > 4096) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Frequency must be between 0 and 4096");
            return;
        }

        int vol_value = vol_item->valueint;
        if (vol_value < 0 || vol_value > 255) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Volume must be between 0 and 255");
            return;
        }

        uint8_t payload[3];
        payload[0] = (uint8_t)(freq_value >> 8);
        payload[1] = (uint8_t)(freq_value & 0xFF);
        payload[2] = (uint8_t)vol_value;

        if (send_frame_to_stm32(0x00, 0x05, 0x06, payload, sizeof(payload)) != ESP_OK) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Failed to send BUZZER");
            return;
        }
    }
    else if (strcmp(cmd, "READ_SHT40") == 0) {
        if (send_frame_to_stm32(0x00, 0x03, 0x00, NULL, 0) != ESP_OK) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Failed to send READ_SHT40");
            return;
        }
    }
    else if (strcmp(cmd, "READ_BME280") == 0) {
        if (send_frame_to_stm32(0x00, 0x03, 0x01, NULL, 0) != ESP_OK) {
            cJSON_Delete(root);
            mqtt_publish_error_json("CMD", "Failed to send READ_BME280");
            return;
        }
    }
    else {
        cJSON_Delete(root);
        mqtt_publish_error_json("CMD", "Unknown command");
        return;
    }

    cJSON_Delete(root);
}

void decode_response(const uint8_t *req)
{
    uint8_t addr = req[0];
    uint8_t status = req[1];
    uint8_t cmd = req[2];
    uint8_t param_addr = req[3];
    uint16_t cmd_combined = ((uint16_t)cmd << 8) | param_addr;

    uint8_t calc_crc = crc8_atm(req, FRAME_LEN_APP - 1);
    uint8_t rx_crc   = req[FRAME_LEN_APP - 1];

    if (addr != DEV_ADDR) {
        ESP_LOGW(TAG, "Invalid device address: 0x%02X", addr);
        mqtt_publish_error_json("STM32", "Invalid device address");
        return;
    }

    if (calc_crc != rx_crc) {
        ESP_LOGW(TAG, "CRC error: calc=0x%02X rx=0x%02X", calc_crc, rx_crc);
        mqtt_publish_error_json("STM32", "CRC error");
        return;
    }

    if (status == ERROR_RESPONSE) {
        ESP_LOGW(TAG, "Error response for command 0x%02X", cmd);
        mqtt_publish_error_json("STM32", "Error response");
        return;
    }

    switch (cmd_combined) {
        case 0x0000: {
            cJSON *root = build_status_base("ALIVE");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "data", "pong");
                mqtt_publish_json(root);
            }
            break;
        }


        

        case 0x0300: {
            int16_t temp_c_x100 = (int16_t)(((uint16_t)req[4] << 8) | req[5]);
            uint16_t rh_x100 = (uint16_t)(((uint16_t)req[6] << 8) | req[7]);

            cJSON *root = build_status_base("SHT40_READ");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddNumberToObject(info, "temperature", temp_c_x100 / 100.0);
                cJSON_AddNumberToObject(info, "humidity", rh_x100 / 100.0);
                mqtt_publish_json(root);
            }
            break;
        }

        case 0x0301: {
            int32_t temp_c_x100 =  (int32_t)(((uint32_t)req[4] << 24) | ((uint32_t)req[5] << 16) |
                                              ((uint32_t)req[6] << 8) | req[7]);
            uint32_t rh_x100 = (uint32_t)(((uint32_t)req[8] << 24) | ((uint32_t)req[9] << 16) |
                                              ((uint32_t)req[10] << 8) | req[11]);
            uint32_t pressure_pa = (uint32_t)(((uint32_t)req[12] << 24) | ((uint32_t)req[13] << 16) |
                                              ((uint32_t)req[14] << 8) | req[15]);


            cJSON *root = build_status_base("BME280_READ");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddNumberToObject(info, "temperature", temp_c_x100 / 100.0);
                cJSON_AddNumberToObject(info, "humidity", rh_x100 / 100.0);
                cJSON_AddNumberToObject(info, "pressure", pressure_pa / 100.0);
                mqtt_publish_json(root);
            }
            break;
        }

        case 0x0401: {
            cJSON *root = build_status_base("OUTPUT_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "LED1");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        case 0x0402: {
            cJSON *root = build_status_base("OUTPUT_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "LED2");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        case 0x0403: {
            cJSON *root = build_status_base("OUTPUT_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "PB12");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        
        case 0x0404: {
            cJSON *root = build_status_base("OUTPUT_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "PC0");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        
        case 0x0405: {
            cJSON *root = build_status_base("OUTPUT_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "PC1");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        
        case 0x0406: {
            cJSON *root = build_status_base("OUTPUT_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "PC2");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        
        case 0x0407: {
            cJSON *root = build_status_base("OUTPUT_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "PC3");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        
        case 0x0408: {
            cJSON *root = build_status_base("OUTPUT_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "ESP32_STATUS");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        
        case 0x0501: {
            cJSON *root = build_status_base("PWM_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "TIM1_CH1");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        case 0x0502: {
            cJSON *root = build_status_base("PWM_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "TIM2_CH3");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        case 0x0503: {
            cJSON *root = build_status_base("PWM_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "TIM4_CH3");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        case 0x0504: {
            cJSON *root = build_status_base("PWM_SET");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddStringToObject(info, "channel", "TIM4_CH4");
                cJSON_AddNumberToObject(info, "value", req[4]);
                mqtt_publish_json(root);
            }
            break;
        }

        case 0x0505: {
            cJSON *root = build_status_base("RGB");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddNumberToObject(info, "r", req[4]);
                cJSON_AddNumberToObject(info, "g", req[5]);
                cJSON_AddNumberToObject(info, "b", req[6]);
                cJSON_AddNumberToObject(info, "value", req[7]);
                mqtt_publish_json(root);
            }
            break;
        }

        case 0x0506: {
            cJSON *root = build_status_base("BUZZER");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddNumberToObject(info, "freq", ((uint16_t)req[4] << 8) | req[5]);
                cJSON_AddNumberToObject(info, "volume", req[6]);
                mqtt_publish_json(root);
            }
            break;
        }

        case 0x0600: {
            cJSON *root = build_status_base("RTC_READ");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddNumberToObject(info, "year", (req[4] + 2000));
                cJSON_AddNumberToObject(info, "month", req[5]);
                cJSON_AddNumberToObject(info, "day", req[6]);
                cJSON_AddNumberToObject(info, "hour", req[7]);
                cJSON_AddNumberToObject(info, "minute", req[8]);
                cJSON_AddNumberToObject(info, "second", req[9]);
                mqtt_publish_json(root);
            }
            break;
        }

        case 0x0601: {
            cJSON *root = build_status_base("SYNC_TIME");
            if (root) {
                cJSON *info = cJSON_GetObjectItem(root, "info");
                cJSON_AddNumberToObject(info, "year", (req[4] + 2000));
                cJSON_AddNumberToObject(info, "month", req[5]);
                cJSON_AddNumberToObject(info, "day", req[6]);
                cJSON_AddNumberToObject(info, "hour", req[7]);
                cJSON_AddNumberToObject(info, "minute", req[8]);
                cJSON_AddNumberToObject(info, "second", req[9]);
                mqtt_publish_json(root);
            }
            break;
        }

        default: {
            char err[64];
            snprintf(err, sizeof(err), "Unknown response 0x%04X", cmd_combined);
            mqtt_publish_error_json("STM32", err);
            break;
        }
    }
}

/*
    * @brief  Application entry point: init peripherals, connect Wi-Fi, start MQTT, create tasks.
*/
void app_main(void)
{
    init_uart();
    init_led();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();
    ntp_sync();
    mqtt_app_start();

    xTaskCreate(uart_rx_task, "uart_rx_task", 4096, NULL, 3, &uart_rx_task_handle);
    xTaskCreate(led_blink_task, "led_blink_task", 2048, NULL, 1, &led_blink_task_handle);

    ESP_LOGI(TAG, "System ready");
}