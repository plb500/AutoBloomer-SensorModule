#ifndef _HIB_LED_INDICATOR_H_
#define _HIB_LED_INDICATOR_H_

#include "wifi_indicator.h"
#include "shift_register.h"

class HIBLEDIndicator : public WiFiIndicator {
    public:
        HIBLEDIndicator(ShiftRegister& shiftRegister, const uint8_t ledIndex) :
            mShiftRegister{shiftRegister},
            mLEDIndex{ledIndex} {}

        virtual void ledOn() {
            mShiftRegister.setState(mLEDIndex, true);
            mShiftRegister.writeStates();
        }

        virtual void ledOff() {
            mShiftRegister.setState(mLEDIndex, false);
            mShiftRegister.writeStates();
        }

    private:
        ShiftRegister& mShiftRegister;
        const uint8_t mLEDIndex;
};

#endif      // _HIB_LED_INDICATOR_H_
