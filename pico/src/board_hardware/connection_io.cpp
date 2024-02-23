#include "connection_io.h"

ConnectionIO::ConnectionIO(
    ShiftRegister& connectDetectRegister, uint8_t connectDetectIndex, 
    ShiftRegister& connectIndicateRegister, uint8_t connectIndicateIndex
) : 
    mConnectDetectRegister{connectDetectRegister},
    mConnectDetectIndex{connectDetectIndex},
    mConnectIndicateRegister{connectIndicateRegister},
    mConnectIndicateIndex{connectIndicateIndex}
{}

void ConnectionIO::initialize() {
    mConnectDetectRegister.initialize();
    mConnectIndicateRegister.initialize();
}

void ConnectionIO::update() {
    mConnectDetectRegister.readStates();
    mConnectIndicateRegister.setState(mConnectIndicateIndex, isConnected());
    mConnectIndicateRegister.writeStates();
}

bool ConnectionIO::isConnected() {
    return mConnectDetectRegister.getState(mConnectDetectIndex);
}
