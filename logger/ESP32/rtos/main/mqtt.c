#include "mqtt.h"
#include "app_flags.h"
#include "wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"

#include <stdlib.h>
#include <stdio.h>

static esp_mqtt_client_handle_t mqtt_client = NULL;
static int8_t mqtt_ready = -1;

static char mqtt_topic_cmd[64];
static char mqtt_topic_status[64];

/*
    * @brief  Handle MQTT events: subscribe on connect, dispatch data, log errors.
*/
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data);



void mqtt_publish_json(cJSON *root)
{
    if (root == NULL || mqtt_client == NULL || mqtt_ready != 1) {
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

cJSON *build_status_base(const char *result)
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

void mqtt_publish_error_json(const char *source, const char *msg)
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
            mqtt_ready = 1;
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
            mqtt_ready = -1;
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

void mqtt_app_start(void)
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

