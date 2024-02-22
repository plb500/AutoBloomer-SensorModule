#ifndef _SENSOR_H_
#define _SENSOR_H_

#include "messaging/sensor_control_message.h"
#include "pico/types.h"

#include <map>
#include <tuple>


using std::map;
using std::tuple;

class Sensor {
    public:
        enum SensorType {
            SCD30_SENSOR            = 0x01,
            STEMMA_SOIL_SENSOR      = 0x02,
            BATTERY_SENSOR          = 0x03,
            SONAR_SENSOR            = 0x04,

            DUMMY_SENSOR            = 0xFF
        };

        enum SensorStatus {
            SENSOR_OK,
            SENSOR_OK_NO_DATA,
            SENSOR_INACTIVE,
            SENSOR_MALFUNCTIONING
        };

        struct SensorDataBuffer {
            void initializeBuffer(uint32_t bufferSize);

            SensorStatus mStatus;
            uint8_t* mDataBytes;
            uint8_t mDataLen;
            absolute_time_t mDataExpiryTime;
        };

        typedef int (*JsonSerializer)(uint8_t*, uint8_t, char*, int);


        Sensor(uint8_t sensorType, JsonSerializer serializer);

        // Must be unique per-sensor type
        uint8_t getSensorTypeID() const { return mSensorType; };      

        // Perform all required hardware initialization
        virtual void initialize();

        // Fully reset the sensor hardware (will be used if sensor stops responding for a period of time)
        virtual void reset() = 0;

        // Completely disable the sensor
        virtual void shutdown() = 0;                

        // (Optionally) respond to a sensor control command
        virtual bool handleSensorControlCommand(SensorControlMessage& message) { return false; }

        // The total size required to pack this sensor's raw data into a binary blob
        virtual constexpr uint16_t getRawDataSize() const { return 0; }

        virtual uint32_t getDataCacheTimeout() const { return SENSOR_DATA_CACHE_TIME_MS; }

        void update(absolute_time_t currentTime);
        const SensorDataBuffer& getCachedData() const { return mCachedData; }

        static int getDataAsJSON(uint8_t sensorTypeID, uint8_t* data, uint8_t dataLength, char* jsonBuffer, int jsonBufferSize);
        static void registerJSONSerializer(int sensorTypeID, JsonSerializer serializer);

    protected:
        typedef tuple<SensorStatus, uint8_t> SensorUpdateResponse;

        virtual void doInitialization() = 0;

        // Update the underlying sensor hardware, serializing any current data into the supplied buffer
        virtual SensorUpdateResponse doUpdate(absolute_time_t currentTime, uint8_t *dataStorageBuffer, size_t bufferSize) = 0;

    private:
        inline void resetUpdateWatchdogTimer();

        static constexpr uint32_t UPDATE_WATCHDOG_TIMEOUT_MS    = (15 * 1000);      // Reset sensor if it hasn't responded in 15s
        static constexpr uint32_t SENSOR_DATA_CACHE_TIME_MS     = (5 * 1000);       // Keep old sensor data around for 5s

        static map<int, JsonSerializer> sJSONSerializerMap;

        const uint8_t mSensorType;
        absolute_time_t mUpdateWatchdogTimeout;
        SensorDataBuffer mCachedData;
};

#endif      // _SENSOR_H_
