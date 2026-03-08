#include <stdio.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

void app_main(void) {
    adc_oneshot_unit_handle_t adc_handle;

    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    adc_oneshot_new_unit(&init_cfg, &adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_11,      // do ~3.3V (zależnie od chipu)
        .bitwidth = ADC_BITWIDTH_DEFAULT
    };
    adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_6, &chan_cfg); // ADC1_CH6 ~ GPIO34 na ESP32 :contentReference[oaicite:10]{index=10}

    while (1) {
        int raw = 0;
        adc_oneshot_read(adc_handle, ADC_CHANNEL_6, &raw);
        printf("ADC raw = %d\n", raw);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}