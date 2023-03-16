#include "shift_register.h"

void init_shift_register(ShiftRegister *shiftRegister) {
    // Initialize pins
    gpio_init(shiftRegister->mLatchPin);
    gpio_set_dir(shiftRegister->mLatchPin, GPIO_OUT);

    gpio_init(shiftRegister->mClockPin);
    gpio_set_dir(shiftRegister->mClockPin, GPIO_OUT);

    gpio_init(shiftRegister->mDataPin);

    switch(shiftRegister->mType) {
        case PISO_SHIFT_REGISTER:
            gpio_set_dir(shiftRegister->mDataPin, GPIO_IN);
            break;
        case SIPO_SHIFT_REGISTER:
            gpio_set_dir(shiftRegister->mDataPin, GPIO_OUT);
            break;
    }

    reset_shift_register(shiftRegister);
}

void reset_shift_register(ShiftRegister *shiftRegister) {
    shiftRegister->mCurrentValue = 0;
}

void set_shift_register_state(ShiftRegister *shiftRegister, uint8_t pos, bool on) {
    if(!shiftRegister || (shiftRegister->mType != SIPO_SHIFT_REGISTER)) {
        return;
    }

    if(on) {
        shiftRegister->mCurrentValue |= (1 << pos);
    } else {
        shiftRegister->mCurrentValue &= ~(1 << pos);
    }
}

void set_shift_register_states(ShiftRegister *shiftRegister, uint8_t states) {
    if(!shiftRegister || (shiftRegister->mType != SIPO_SHIFT_REGISTER)) {
        return;
    }

    shiftRegister->mCurrentValue = states;
}

void write_shift_register_states(ShiftRegister *shiftRegister) {
    if(!shiftRegister || (shiftRegister->mType != SIPO_SHIFT_REGISTER)) {
        return;
    }

    // Set latch pin low while we are setting the data
    gpio_put(shiftRegister->mLatchPin, 0);

    // Clock each bit out individually
    for(int bit = 0; bit < 8; ++bit) {
        gpio_put(shiftRegister->mClockPin, 0);
        gpio_put(shiftRegister->mDataPin, (shiftRegister->mCurrentValue & (1 << bit)));
        gpio_put(shiftRegister->mClockPin, 1);
    }

    // Set latch high to store data
    gpio_put(shiftRegister->mLatchPin, 1);
}


void read_shift_register_states(ShiftRegister *shiftRegister) {
    if(!shiftRegister || (shiftRegister->mType != PISO_SHIFT_REGISTER)) {
        return;
    }

    shiftRegister->mCurrentValue = 0;

    // Latch the register
    gpio_put(shiftRegister->mLatchPin, false);
    gpio_put(shiftRegister->mLatchPin, true);

    // Shift input bits
    for(int i = 7; i >= 0; --i) {
        if(gpio_get(shiftRegister->mDataPin)) {
            shiftRegister->mCurrentValue |= (1 << i);
        }

        // Shift next bit
        gpio_put(shiftRegister->mClockPin, true);
        gpio_put(shiftRegister->mClockPin, false);
    }    
}


bool get_shift_register_state(ShiftRegister *shiftRegister, uint8_t pos) {
    if(!shiftRegister || (shiftRegister->mType != PISO_SHIFT_REGISTER)) {
        return false;
    }

    return !(shiftRegister->mCurrentValue & (1 << pos));
}
