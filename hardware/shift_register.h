#ifndef _SHIFT_REGISTER_H_
#define _SHIFT_REGISTER_H_

#include "pico/stdlib.h"

typedef enum {
    PISO_SHIFT_REGISTER,        // Parallel-in-Serial-Out shift register (for more inputs)
    SIPO_SHIFT_REGISTER         // Serial-in-Parallel-Out shift register (for more outputs)
} ShiftRegisterType;

typedef struct {
    const uint8_t mDataPin;
    const uint8_t mLatchPin;
    const uint8_t mClockPin;
    const ShiftRegisterType mType;

    uint8_t mCurrentValue;
} ShiftRegister;

// Generic functions
void init_shift_register(ShiftRegister *shiftRegister);
void reset_shift_register(ShiftRegister *shiftRegister);

// Write functions (SIPO shift register)
void set_shift_register_state(ShiftRegister *shiftRegister, uint8_t pos, bool on);
void set_shift_register_states(ShiftRegister *shiftRegister, uint8_t states);
void write_shift_register_states(ShiftRegister *shiftRegister);

// Read functions (PISO shift register)
void read_shift_register_states(ShiftRegister *shiftRegister);
bool get_shift_register_state(ShiftRegister *shiftRegister, uint8_t pos);

#endif
