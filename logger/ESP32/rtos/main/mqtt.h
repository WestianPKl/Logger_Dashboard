
#ifndef MQTT_H
#define MQTT_H

#include <stdint.h>
#include "cJSON.h"

#define MQTT_BROKER_URI        "mqtt://192.168.18.6:1883"
#define MQTT_USERNAME          "pico_user"
#define MQTT_PASSWORD          "HASLO"

/*
    * @brief  Start the MQTT client, connect to the broker, and subscribe to the command topic.
*/
void mqtt_app_start(void);

/*
    * @brief  Publish a cJSON object as a JSON string on the device status topic.
    *         Takes ownership of root and deletes it after publishing.
    * @param  root: cJSON object to publish (will be freed).
*/
void mqtt_publish_json(cJSON *root);

/*
    * @brief  Publish a JSON error message on the device status topic.
    * @param  source: Error source identifier (e.g. "CMD", "STM32").
    * @param  msg: Human-readable error description.
*/
void mqtt_publish_error_json(const char *source, const char *msg);

/*
    * @brief  Create a base cJSON status object with result, type, timestamp, and logger_id fields.
    * @param  result: Result string (e.g. "ALIVE", "SHT40_READ").
    * @retval cJSON object on success, NULL on allocation failure.
*/
cJSON *build_status_base(const char *result);

/*
    * @brief  Parse an MQTT command JSON payload and send the corresponding frame to STM32.
    * @param  data: Raw JSON string (not null-terminated).
    * @param  data_len: Length of the JSON data in bytes.
*/
void process_mqtt_command_json(const char *data, int data_len);

#endif // MQTT_H