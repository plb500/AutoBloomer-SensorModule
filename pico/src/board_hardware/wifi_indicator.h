#ifndef _WIFI_INDICATOR_H_
#define _WIFI_INDICATOR_H_

class WiFiIndicator {
    public:
        WiFiIndicator() = default;

        virtual void ledOn() = 0;
        virtual void ledOff() = 0;
};

#endif
