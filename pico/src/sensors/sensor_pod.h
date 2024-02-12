#ifndef _SENSOR_POD_H_
#define _SENSOR_POD_H_

#include "stemma_soil_sensor.h"
#include "sensor_i2c_interface.h"
#include "pico/types.h"
#include <optional>

using std::optional;

class SensorPod {
    public:
        enum Status {
            SENSOR_DISCONNECTED                 = 0,
            SENSOR_CONNECTED_MALFUNCTIONING     = 1,
            SENSOR_CONNECTED_NO_DATA            = 2,
            SENSOR_CONNECTED_VALID_DATA         = 3
        };

        struct Data {
            bool mSCD30SensorDataValid;
            bool mSoilSensorDataValid;
            Status mStatus;
            float mCO2Level;
            float mTemperature;
            float mHumidity;
            uint16_t mSoilSensorData;
        };

        SensorPod(
            optional<StemmaSoilSensor>& soilSensor,
            optional<I2CInterface>& scd30Interface
        );

        bool initialize();
        bool reset();
        void startReadings();
        void update();

        bool hasValidData();
        const Data& getCurrentData();
        bool hasSoilSensor();
        bool hasSCD30Sensor();
        bool isSoilSensorActive();
        bool isSCD30Active();
        void setTemperatureOffset(double offset);
        void setForcedRecalibrationValue(uint16_t frc);

    private:
        void initializeSoilSensorConnection();
        void initializeSCD30Connection();
        void shutdownSoilSensorConnection();
        void shutdownSCD30Connection();
        void resetSoilSensorConnection();
        void resetSCD30Connection();
        void startSoilSensorReadings();
        void startSCD30Readings();
        bool updateSoilSensor();
        bool updateSCD30();

        optional<StemmaSoilSensor>& mSoilSensor;
        optional<I2CInterface>& mSCD30Interface;

        bool mSoilSensorActive;
        bool mSCD30SensorActive;

        uint8_t mSCD30PowerControlPin;

        Data mCurrentData;
        absolute_time_t mSCD30ResetTimeout;    
};


#endif      // _SENSOR_POD_H_
