#include "app_flags.h"

volatile app_io_state_t app_io_state = {0};
volatile app_rgb_t app_rgb = {0};
volatile app_buzzer_t app_buzzer = {0};
EventGroupHandle_t appEvents;