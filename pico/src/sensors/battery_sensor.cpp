#include "battery_sensor.h"
#include "util/debug_io.h"

#include "hardware/adc.h"
// #include "pico/stdlib.h"
#include <cmath>
#include <cstring>

using std::make_tuple;
using std::get;


// Max value will be 3.3v (internal ADC ref). Convert from 12-bit value to voltage.
// On the HIB, the voltage is put through a voltage divider which halves the voltage
const float ADC_CONVERSION_FACTOR = 0.001591464080024;      // Got this via actually measuring the pin voltage. There's some drop over the voltage divider
const uint BATTERY_SAMPLE_PERIOD_MS = 300000;               // Battery isn't going to be draining rapidly. Can measure once every five minutes
const uint BATTERY_CHARGE_PERIOD_MS = 20;                   // Time for capacitor to build up

#define BATTERY_SAMPLE_COUNT_FACTOR     (3)                 // We'll take 8 (2^3) battery readings


BatteryVoltageSensor::BatteryVoltageSensor(int enablePin, int measurePin, int adcInput) :
    Sensor(BATTERY_SENSOR, &BatteryVoltageSensor::serializeDataToJSON),
    mEnableSensePin(enablePin),
    mBatteryMeasurePin(measurePin),
    mADCInput(adcInput)
{}

void BatteryVoltageSensor::initialize() {
    // Initialize the GPIO pin which will trigger battery current into the ADC pin.
    // It is driven LOW to activate the circuit
    gpio_init(mEnableSensePin);
    gpio_set_dir(mEnableSensePin, GPIO_OUT);
    gpio_put(mEnableSensePin, 1);

    // Initialize the ADC
    adc_init();
    adc_gpio_init(mBatteryMeasurePin);
    adc_select_input(mADCInput);

    mCurrentState = BATTERY_SENSOR_SLEEPING;
    mSensorTransitionTime = make_timeout_time_ms(0);
}

void BatteryVoltageSensor::reset() {
    mCurrentState = BATTERY_SENSOR_SLEEPING;
    mSensorTransitionTime = make_timeout_time_ms(0);
}

void BatteryVoltageSensor::shutdown() {
    // Nothing really to do here
}

int BatteryVoltageSensor::serializeDataToJSON(uint8_t* data, uint8_t dataSize, char* jsonBuffer, int jsonBufferSize) {
    return -1;
}

Sensor::SensorUpdateResponse BatteryVoltageSensor::doUpdate(absolute_time_t currentTime, uint8_t *dataStorageBuffer, size_t bufferSize) {
    volatile uint adc_data = 0;
    float voltage = 0.f;
    bool hasReading = false;
    
    SensorUpdateResponse response = make_tuple(Sensor::SENSOR_OK_NO_DATA, 0);

    if(absolute_time_diff_us(mSensorTransitionTime, get_absolute_time()) <= 0) {
        return response;
    }

    // Handle our transition point
    switch(mCurrentState) {
        case BATTERY_SENSOR_SLEEPING:
            // Enable measurement circuit
            gpio_put(mEnableSensePin, 0);
            mSensorTransitionTime = make_timeout_time_ms(BATTERY_CHARGE_PERIOD_MS);
            mCurrentState = BATTERY_SENSOR_CHARGING;
            break;

        case BATTERY_SENSOR_CHARGING:
            // Time to read the sensor
            for(int i = 0; i < (1 << BATTERY_SAMPLE_COUNT_FACTOR); ++i) {
                adc_data = adc_data + adc_read();
            }
            adc_data = (adc_data >> BATTERY_SAMPLE_COUNT_FACTOR);

            voltage = (round(adc_data * ADC_CONVERSION_FACTOR * 100) / 100.f);
            memcpy(dataStorageBuffer, &voltage, sizeof(float));

            // Disable measurement circuit
            gpio_put(mEnableSensePin, 1);

            mSensorTransitionTime = make_timeout_time_ms(BATTERY_SAMPLE_PERIOD_MS);
            mCurrentState = BATTERY_SENSOR_SLEEPING;
            hasReading = true;

            get<0>(response) = Sensor::SENSOR_OK;
            get<1>(response) = sizeof(float);
            break;
    }

    return response;
}
