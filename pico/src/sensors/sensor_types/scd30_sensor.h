#ifndef _SCD30_SENSOR_H_
#define _SCD30_SENSOR_H_

#include "sensors/sensor.h"
#include "sensors/hardware_interfaces/sensor_i2c_interface.h"

class SCD30Sensor : public Sensor {
    public:
        SCD30Sensor(uint8_t sdaPin, uint8_t sclPin, uint8_t powerPin);

        virtual void reset();
        virtual void shutdown();                

        virtual bool handleSensorControlCommand(SensorControlMessage& message);
        
        void setTemperatureOffset(double offset);
        void setForcedRecalibrationValue(uint16_t frc);

        static int serializeDataToJSON(uint8_t* data, uint8_t dataSize, char* jsonBuffer, int jsonBufferSize);
        virtual constexpr uint16_t getRawDataSize() const { return RAW_DATA_SIZE; }
        
        static const uint32_t RAW_DATA_SIZE = (sizeof(float) * 3);
    protected:
        virtual void doInitialization();
        virtual Sensor::SensorUpdateResponse doUpdate(absolute_time_t currentTime, uint8_t *dataStorageBuffer, size_t bufferSize);

    private:
        // Types of incoming control commands this sensor handles
        enum SCD30CommandType {
            SCD30_SET_TEMP_OFFSET       = 0x504D4554,       // "TEMP"
            SCD30_SET_FRC               = 0x00435246        // "FRC"
        };

        void startReadings();
        void handleSetTemperatureOffsetCommand(const char *commandParam);
        void handleSetFRCCommand(const char *commandParam);
        

        static constexpr int SCD30_MEASUREMENT_INTERVAL_SECONDS      = 2;

        bool mActive;
        uint8_t mSDAPin;
        uint8_t mSCLPin;
        uint8_t mPowerControlPin;
};

#endif      // _SCD30_SENSOR_H_
