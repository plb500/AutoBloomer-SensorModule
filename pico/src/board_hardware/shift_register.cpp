#include "shift_register.h"

ShiftRegister::ShiftRegister(uint8_t dataPin, uint8_t latchPin, uint8_t clockPin, Type type, uint8_t numBits) :
    mDataPin{dataPin},
    mLatchPin{latchPin},
    mClockPin{clockPin},
    mType{type},
    mNumBits{numBits},
    mInitialized{false}
{}

void ShiftRegister::initialize() {
    if(mInitialized) {
        return;
    }

    // Initialize pins
    gpio_init(mLatchPin);
    gpio_set_dir(mLatchPin, GPIO_OUT);

    gpio_init(mClockPin);
    gpio_set_dir(mClockPin, GPIO_OUT);

    gpio_init(mDataPin);

    switch(mType) {
        case PISO_SHIFT_REGISTER:
            gpio_set_dir(mDataPin, GPIO_IN);
            break;
        case SIPO_SHIFT_REGISTER:
            gpio_set_dir(mDataPin, GPIO_OUT);
            break;
    }

    reset();
    mInitialized = true;
}

void ShiftRegister::reset() {
    mCurrentValue = 0;
}

void ShiftRegister::setState(uint8_t pos, bool on) {
    assert(mType == SIPO_SHIFT_REGISTER);
    assert(pos < mNumBits);

    if(on) {
        mCurrentValue |= (1 << pos);
    } else {
        mCurrentValue &= ~(1 << pos);
    }
}

void ShiftRegister::setStates(uint32_t states) {
    assert(mType == SIPO_SHIFT_REGISTER);

    mCurrentValue = states;
}

void ShiftRegister::writeStates() {
    assert(mType == SIPO_SHIFT_REGISTER);

    // Set latch pin low while we are setting the data
    gpio_put(mLatchPin, 0);
    sleep_us(1);

    // Clock each bit out individually
    for(int bit = (mNumBits - 1); bit >= 0; --bit) {
        gpio_put(mClockPin, 0);
        sleep_us(1);
        gpio_put(mDataPin, (mCurrentValue & (1 << bit)));
        sleep_us(1);
        gpio_put(mClockPin, 1);
        sleep_us(1);
    }

    // Set latch high to store data
    gpio_put(mLatchPin, 1);
}

bool ShiftRegister::getState(uint16_t pos) {
    assert(mType == PISO_SHIFT_REGISTER);

    return !(mCurrentValue & (1 << pos));
}

void ShiftRegister::readStates() {
    assert(mType == PISO_SHIFT_REGISTER);

    mCurrentValue = 0;

    // Latch the register
    gpio_put(mLatchPin, false);
    sleep_us(1);
    gpio_put(mLatchPin, true);
    sleep_us(1);

    // Shift input bits
    for(int i = (mNumBits - 1); i >= 0; --i) {
        if(gpio_get(mDataPin)) {
            mCurrentValue |= (1 << i);
        }

        // Shift next bit
        gpio_put(mClockPin, true);
        sleep_us(1);
        gpio_put(mClockPin, false);
        sleep_us(1);
    }
}
