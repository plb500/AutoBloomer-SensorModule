#ifndef _SHIFT_REGISTER_H_
#define _SHIFT_REGISTER_H_

#include "pico/stdlib.h"


class ShiftRegister {
    public:
        enum Type {
            PISO_SHIFT_REGISTER,        // Parallel-in-Serial-Out shift register (for more inputs)
            SIPO_SHIFT_REGISTER         // Serial-in-Parallel-Out shift register (for more outputs)
        };

        ShiftRegister(
            uint8_t dataPin, uint8_t latchPin, uint8_t clockPin, Type type, uint8_t numBits
        );

        void initialize();
        void reset();

        // Write functions (SIPO shift register only)
        void setState(uint8_t pos, bool on);
        void setStates(uint32_t states);
        void writeStates();

        // Read functions (PISO shift register)
        bool getState(uint16_t pos);
        void readStates();

    private:
        const uint8_t mDataPin;
        const uint8_t mLatchPin;
        const uint8_t mClockPin;
        const Type mType;
        const uint8_t mNumBits;

        bool mInitialized;
        uint32_t mCurrentValue;
};

#endif      // _SHIFT_REGISTER_H_
