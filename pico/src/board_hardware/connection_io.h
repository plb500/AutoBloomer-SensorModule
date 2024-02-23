#ifndef _CONNECTION_IO_H_
#define _CONNECTION_IO_H_

#include "board_hardware/shift_register.h"

class ConnectionIO {
    public:
        ConnectionIO(
            ShiftRegister& connectDetectRegister, uint8_t connectDetectIndex, 
            ShiftRegister& connectIndicateRegister, uint8_t connectIndicateIndex);

        void initialize();
        void update();
        bool isConnected();

    private:
        ShiftRegister& mConnectDetectRegister;
        const uint8_t mConnectDetectIndex;
        ShiftRegister& mConnectIndicateRegister;
        const uint8_t mConnectIndicateIndex;
};

#endif      // _CONNECTION_IO_H_