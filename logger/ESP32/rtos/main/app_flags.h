
#ifndef APP_FLAGS_H
#define APP_FLAGS_H

#include <stdint.h>

#define DEV_ADDR               0xB2
#define ERROR_RESPONSE         0x7F
#define DEVICE_ID              376

/*
    * @brief  Global log tag used by all modules via ESP_LOGx macros.
*/
extern const char *TAG;

#endif // APP_FLAGS_H