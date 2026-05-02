idf_component_register(SRCS "app_flags.c" "wifi.c" "gpio.c" "uart.c" "mqtt.c" "support.c" "main.c"
                       PRIV_REQUIRES spi_flash
                       REQUIRES esp_driver_uart esp_driver_gpio
                       REQUIRES nvs_flash
                       REQUIRES esp_wifi
                       REQUIRES esp_netif
                       REQUIRES esp_event
                       REQUIRES lwip
                       INCLUDE_DIRS "")
