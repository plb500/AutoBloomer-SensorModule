#ifndef _PICO_W_ONBOARD_LED_INDICATOR_H_
#define _PICO_W_ONBOARD_LED_INDICATOR_H_

#include "wifi_indicator.h"
#include "pico/cyw43_arch.h"


class PicoWOnboardLEDIndicator : public WiFiIndicator {
    public:
        virtual void ledOn() {
            if(cyw43_arch_async_context()) {
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            }
        }

        virtual void ledOff() {
            if(cyw43_arch_async_context()) {
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            }
        }
};

#endif      // _PICO_W_ONBOARD_LED_INDICATOR_H_
