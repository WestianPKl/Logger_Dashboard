#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

#include "driver/uart.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_err.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include "mqtt_client.h"

#include "cJSON.h"
#include "support.h"

#define LED_GPIO               22
#define UART_TX_GPIO           20
#define UART_RX_GPIO           21

#define UART_PORT              UART_NUM_1
#define UART_BAUD              115200
#define UART_DRIVER_RX_BUF_SZ  2048

#define FRAME_LEN_APP          32
#define FRAME_PAYLOAD_MAX      (FRAME_LEN_APP - 5)
#define DEV_ADDR               0xB2

#define ERROR_RESPONSE         0x7F

#define DEVICE_ID              376

#define NTP_SERVER             "192.168.18.158"
#define WIFI_SSID              "TP-Link_0A7B"
#define WIFI_PASS              "12345678"
#define MQTT_BROKER_URI        "mqtt://192.168.18.6:1883"
#define MQTT_USERNAME          "pico_user"
#define MQTT_PASSWORD          "HASLO"

#define WIFI_CONNECTED_BIT     BIT0
#define WIFI_FAIL_BIT          BIT1
#define WIFI_MAXIMUM_RETRY     10

static const char *TAG = "APP";

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static bool mqtt_ready = false;

static TaskHandle_t uart_rx_task_handle = NULL;
static TaskHandle_t led_blink_task_handle = NULL;

static esp_mqtt_client_handle_t mqtt_client = NULL;
static char mqtt_topic_cmd[64];
static char mqtt_topic_status[64];

static void init_led(void);
static void init_uart(void);

static void uart_rx_task(void *pvParameters);
static void led_blink_task(void *pvParameters);

static void ntp_sync(void);
static bool get_unix_timestamp(uint32_t *ts_out);

static void wifi_init_sta(void);
static void mqtt_app_start(void);

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data);

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data);

static bool find_output_channel_id(const char *name, uint8_t *channel_id);

static esp_err_t send_frame_to_stm32(uint8_t status, uint8_t cmd, uint8_t param,
                                     const uint8_t *payload, uint32_t payload_len);

static void decode_response(const uint8_t *req);
static void process_mqtt_command_json(const char *data, int data_len);

static void mqtt_publish_json(cJSON *root);
static void mqtt_publish_error_json(const char *source, const char *msg);
static cJSON *build_status_base(const char *result);

static void init_led(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

static void init_uart(void)
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_TX_GPIO, UART_RX_GPIO,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, UART_DRIVER_RX_BUF_SZ, 0, 0, NULL, 0));
}

static void ntp_sync(void)
{
    ESP_LOGI(TAG, "Starting SNTP");

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, NTP_SERVER);
    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 15;

    while (retry < retry_count) {
        time(&now);
        localtime_r(&now, &timeinfo);

        if ((timeinfo.tm_year + 1900) >= 2024) {
            ESP_LOGI(TAG, "Time synchronized");
            return;
        }

        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry + 1, retry_count);
        vTaskDelay(pdMS_TO_TICKS(1000));
        retry++;
    }

    ESP_LOGW(TAG, "Failed to synchronize time");
}

static bool get_unix_timestamp(uint32_t *ts_out)
{
    if (ts_out == NULL) {
        return false;
    }

    time_t now = 0;
    time(&now);

    if (now < 1700000000) {
        return false;
    }

    *ts_out = (uint32_t)now;
    return true;
}

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry Wi-Fi connection");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "Failed to connect to AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    configASSERT(s_wifi_event_group != NULL);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished");

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP: %s", WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to AP: %s", WIFI_SSID);
    } else {
        ESP_LOGE(TAG, "Unexpected Wi-Fi event");
    }
}

static void mqtt_publish_json(cJSON *root)
{
    if (root == NULL || mqtt_client == NULL || !mqtt_ready) {
        if (root) {
            cJSON_Delete(root);
        }
        return;
    }

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (json_str == NULL) {
        ESP_LOGE(TAG, "Failed to serialize JSON");
        return;
    }

    int msg_id = esp_mqtt_client_publish(mqtt_client, mqtt_topic_status, json_str, 0, 1, 0);
    ESP_LOGI(TAG, "MQTT publish, msg_id=%d, topic=%s, payload=%s",
             msg_id, mqtt_topic_status, json_str);

    free(json_str);
}

static cJSON *build_status_base(const char *result)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        return NULL;
    }

    cJSON *info = cJSON_CreateObject();
    if (info == NULL) {
        cJSON_Delete(root);
        return NULL;
    }

    uint32_t ts = 0;
    if (!get_unix_timestamp(&ts)) {
        ts = 0;
    }

    cJSON_AddStringToObject(root, "result", result ? result : "UNKNOWN");
    cJSON_AddItemToObject(root, "info", info);
    cJSON_AddStringToObject(root, "type", "STATUS");
    cJSON_AddNumberToObject(root, "timestamp", ts);

    cJSON_AddNumberToObject(info, "timestamp", ts);
    cJSON_AddNumberToObject(info, "logger_id", DEVICE_ID);

    return root;
}

static void mqtt_publish_error_json(const char *source, const char *msg)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        return;
    }

    cJSON *info = cJSON_CreateObject();
    if (info == NULL) {
        cJSON_Delete(root);
        return;
    }

    uint32_t ts = 0;
    if (!get_unix_timestamp(&ts)) {
        ts = 0;
    }

    cJSON_AddStringToObject(root, "result", "ERROR");
    cJSON_AddItemToObject(root, "info", info);
    cJSON_AddStringToObject(root, "type", "ERROR");
    cJSON_AddNumberToObject(root, "timestamp", ts);

    cJSON_AddNumberToObject(info, "timestamp", ts);
    cJSON_AddStringToObject(info, "source", source ? source : "UNKNOWN");
    cJSON_AddStringToObject(info, "msg", msg ? msg : "UNKNOWN");

    mqtt_publish_json(root);
}

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            mqtt_ready = true;
            ESP_LOGI(TAG, "MQTT connected");
            esp_mqtt_client_subscribe(event->client, mqtt_topic_cmd, 1);

            {
                cJSON *root = build_status_base("MQTT_CONNECTED");
                if (root) {
                    mqtt_publish_json(root);
                }
            }
            break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_ready = false;
            ESP_LOGW(TAG, "MQTT disconnected");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT subscribed, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT topic: %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "MQTT data : %.*s", event->data_len, event->data);
            process_mqtt_command_json(event->data, event->data_len);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error");
            break;

        default:
            break;
    }
}

static void mqtt_app_start(void)
{
    snprintf(mqtt_topic_cmd, sizeof(mqtt_topic_cmd), "devices/%d/cmd", DEVICE_ID);
    snprintf(mqtt_topic_status, sizeof(mqtt_topic_status), "devices/%d/status", DEVICE_ID);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    configASSERT(mqtt_client != NULL);

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL)
    );

    ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));
}

static bool find_output_channel_id(const char *name, uint8_t *channel_id)
{
    if (name == NULL || channel_id == NULL) {
        return false;
    }

    if (strcmp(name, "LED1") == 0) {
        *channel_id = 0x01;
        return true;
    }
    else if (strcmp(name, "LED2") == 0) {
        *channel_id = 0x02;
        return true;
    }
    else if (strcmp(name, "PB12") == 0) {
        *channel_id = 0x03;
        return true;
    }
    else if (strcmp(name, "PC0") == 0) {
        *channel_id = 0x04;
        return true;
    }
    else if (strcmp(name, "PC1") == 0) {
        *channel_id = 0x05;
        return true;
    }
    else if (strcmp(name, "PC2") == 0) {
        *channel_id = 0x06;
        return true;
    }
    else if (strcmp(name, "PC3") == 0) {
        *channel_id = 0x07;
        return true;
    }
    else if (strcmp(name, "ESP32_STATUS") == 0) {
        *channel_id = 0x01;
        return true;
    }

    return false;
}

static bool find_pwm_channel_id(const char *name, uint8_t *channel_id)
{
    if (name == NULL || channel_id == NULL) {
        return false;
    }

    if (strcmp(name, "TIM1_CH1") == 0) {
        *channel_id = 0x01;
        return true;
    }
    else if (strcmp(name, "TIM2_CH3") == 0) {
        *channel_id = 0x02;
        return true;
    }
    else if (strcmp(name, "TIM4_CH3") == 0) {
        *channel_id = 0x03;
        return true;
    }
    else if (strcmp(name, "TIM4_CH4") == 0) {
        *channel_id = 0x04;
        return true;
    }

    return false;
}

static bool find_input_channel_id(const char *name, uint8_t *channel_id)
{
    if (name == NULL || channel_id == NULL) {
        return false;
    }

    if (strcmp(name, "BTN1") == 0) {
        *channel_id = 0x01;
        return true;
    } 
    else if (strcmp(name, "BTN2") == 0) {
        *channel_id = 0x02;
        return true;
    } 
    else if (strcmp(name, "ESP32_STATUS") == 0) {
        *channel_id = 0x03;
        return true;
    }

    return false;
}

static esp_err_t send_frame_to_stm32(uint8_t status, uint8_t cmd, uint8_t param,
                                     const uint8_t *payload, uint32_t payload_len)
{
    uint8_t frame[FRAME_LEN_APP];
    memset(frame, 0, sizeof(frame));

    if (payload_len > FRAME_PAYLOAD_MAX) {
        ESP_LOGE(TAG, "Payload too large: %" PRIu32, payload_len);
        return ESP_ERR_INVALID_SIZE;
    }

    frame[0] = DEV_ADDR;
    frame[1] = status;
    frame[2] = cmd;
    frame[3] = param;

    if (payload && payload_len > 0) {
        memcpy(&frame[4], payload, payload_len);
    }

    frame[FRAME_LEN_APP - 1] = crc8_atm(frame, FRAME_LEN_APP - 1);

    int written = uart_write_bytes(UART_PORT, frame, FRAME_LEN_APP);
    if (written != FRAME_LEN_APP) {
        ESP_LOGE(TAG, "UART write failed: %d/%d", written, FRAME_LEN_APP);
        return ESP_FAIL;
    }

    return ESP_OK;
}

static void process_mqtt_command_json(const char *data, int data_len)
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
        if (!find_output_channel_id(channel_item->valuestring, &channel_id)) {
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

static void uart_rx_task(void *pvParameters)
{
    uint8_t frame[FRAME_LEN_APP];
    size_t collected = 0;

    while (1) {
        int len = uart_read_bytes(
            UART_PORT,
            &frame[collected],
            FRAME_LEN_APP - collected,
            pdMS_TO_TICKS(50)
        );

        if (len > 0) {
            collected += (size_t)len;

            if (collected == FRAME_LEN_APP) {
                decode_response(frame);
                collected = 0;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

static void led_blink_task(void *pvParameters)
{
    while (1) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(100));

        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void decode_response(const uint8_t *req)
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