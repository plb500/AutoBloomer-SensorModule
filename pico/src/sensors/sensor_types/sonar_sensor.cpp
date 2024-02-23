#include "sonar_sensor.h"
#include "uart_rx.pio.h"
#include "pico/time.h"

#include <tuple>
#include <cstring>
#include <cstdio>


using std::make_tuple;
using std::get;


constexpr const char* DISTANCE_JSON_KEY               = "distance";


SonarSensor::SonarSensor(
    PIOWrapper &pioWrapper, int stateMachineID, int txPin, int rxPin, int baud, ConnectionIO& connectionIO) :
    Sensor(Sensor::SONAR_SENSOR, &SonarSensor::serializeDataToJSON),
    mPIOWrapper(pioWrapper),
    mStateMachineID(stateMachineID),
    mTXPin(txPin),
    mRXPin(rxPin),
    mBaudrate(baud),
    mConnectionIO(connectionIO)
{}

void SonarSensor::doInitialization() {
    // Initialize the connect-detect shift register
    mConnectionIO.initialize();

    // Setup the PIO if it hasn't already been done
    if(!mPIOWrapper.mInitialized) {
        initializeSonarPIO(mPIOWrapper);
        mPIOWrapper.mInitialized = true;
    }

    // Set TX pin high to trigger serial transmit on sonar
    gpio_init(mTXPin);
    gpio_set_dir(mTXPin, GPIO_OUT);
    gpio_put(mTXPin, 1);

    // Initialize sonar UART - sets up the state machine we're going to use to receive sonar uart characters.
    uart_rx_program_init(
        mPIOWrapper.mPIO, 
        mStateMachineID, 
        mPIOWrapper.mOffset,
        mRXPin, 
        mBaudrate
    );

    mCurrentBufferPos = 0;
}

void SonarSensor::reset() {
    // Not much we can do here
    gpio_put(mTXPin, 0);

    mCurrentBufferPos = 0;

    sleep_ms(1);

    gpio_put(mTXPin, 1);
}

void SonarSensor::shutdown() {
    // Nothing to do here, either
}

int SonarSensor::serializeDataToJSON(uint8_t* data, uint8_t dataSize, char* jsonBuffer, int jsonBufferSize) {
    uint16_t distance;

    memcpy(&distance, data, sizeof(uint16_t));

    return sprintf(jsonBuffer,
         "\"%s\": %d",
        DISTANCE_JSON_KEY, distance
    );
}

Sensor::SensorUpdateResponse SonarSensor::doUpdate(absolute_time_t currentTime, uint8_t *dataStorageBuffer, size_t bufferSize) {
    mConnectionIO.update();

    if(!mConnectionIO.isConnected()) {
        return make_tuple(Sensor::SENSOR_NOT_CONNECTED, 0);
    }

    SensorUpdateResponse response = make_tuple(Sensor::SENSOR_OK_NO_DATA, 0);

    while(uart_rx_program_has_data(mPIOWrapper.mPIO, mStateMachineID)) {
        char c = uart_rx_program_getc(mPIOWrapper.mPIO, mStateMachineID);
        
        if(c == 0xFF) {
            mCurrentBufferPos = 0;
        }

        mPacketBuffer[mCurrentBufferPos++] = c;

        if(mCurrentBufferPos >= SONAR_SENSOR_PACKET_SIZE) {
            // Full packet, compute checksum
            char checksum = mPacketBuffer[0] + mPacketBuffer[1] + mPacketBuffer[2];

            // If checksum is valid calculate distance
            if(checksum == mPacketBuffer[3]) {
                uint16_t distance = ((mPacketBuffer[1] << 8) + mPacketBuffer[2]);

                get<0>(response) = SENSOR_OK;
                get<1>(response) = sizeof(uint16_t);
                memcpy(dataStorageBuffer, &distance, sizeof(uint16_t));
            } else {
                get<0>(response) = SENSOR_MALFUNCTIONING;
                get<1>(response) = 0;
            }

            mCurrentBufferPos = 0;

            // Only process a max of one packet per update
            break;
        }
    }

    return response;
}

void SonarSensor::initializeSonarPIO(PIOWrapper& pioWrapper) {
    pioWrapper.mOffset = pio_add_program(pioWrapper.mPIO, &uart_rx_program);
}
