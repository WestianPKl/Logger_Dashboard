#include "pico/stdlib.h"
#include "hardware/pwm.h"

int main() {
    const uint PWM_GPIO = 20;     // wybierz pin bez kolizji z SPI w Twoim setupie
    gpio_set_function(PWM_GPIO, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(PWM_GPIO);
    uint chan  = pwm_gpio_to_channel(PWM_GPIO);

    pwm_config cfg = pwm_get_default_config();
    // zostaw domyślne clkdiv i wrap, albo ustaw swoje:
    pwm_config_set_clkdiv(&cfg, 4.0f);   // wolniej
    pwm_config_set_wrap(&cfg, 255);      // 8-bit

    pwm_init(slice, &cfg, true);

    while (true) {
        pwm_set_chan_level(slice, chan, 0);
        sleep_ms(1000);
        pwm_set_chan_level(slice, chan, 128);
        sleep_ms(1000);
        pwm_set_chan_level(slice, chan, 255);
        sleep_ms(1000);
    }
}