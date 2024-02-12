#ifndef _USER_DATA_H_
#define _USER_DATA_H_

#include <string>

using std::string;

class UserData {
    public:
        UserData();

        bool hasNetworkUserData();
        bool hasMQTTUserData();

        void setLocation(const char* location);
        void setSensorName(const char* name);
        void setSSID(const char* ssid);
        void setPSK(const char* psk);
        void setBrokerAddress(const char* brokerAddress);

        void writeToFlash();
        bool readFromFlash();

        const string& getLocationName() const;
        const string& getSensorName() const;
        const string& getSSID() const;
        const string& getPSK() const;
        const string& getBrokerAddress() const;

        static constexpr int MAX_LOCATION_LENGTH            = 32;
        static constexpr int MAX_SENSOR_NAME_LENGTH         = 32;
        static constexpr int MAX_SSID_LENGTH                = 32;
        static constexpr int MAX_PSK_LENGTH                 = 64;
        static constexpr int MAX_BROKER_LENGTH              = 256;
        static constexpr int VALID_DATA_KEY_LENGTH          = 26;
        static constexpr int USER_DATA_FLASH_SIZE = (
            MAX_LOCATION_LENGTH +
            MAX_SENSOR_NAME_LENGTH +
            MAX_SSID_LENGTH +
            MAX_PSK_LENGTH +
            MAX_BROKER_LENGTH +
            VALID_DATA_KEY_LENGTH +
            6       // (null terminating each above string)
        );

        // Will eventually use CRC or something here but I like being able to inspect the memory and see it
        static constexpr const char* const VALID_DATA_KEY   = "xXx This is valid data xXx";

    private:
        int serializeToByteArray(char *bytes, int bytesSize);
        bool serializeFromByteArray(const char *bytes, int bytesSize);

        string mLocationName;
        string mSensorName;
        string mSSID;
        string mPSK;
        string mBrokerAddress;
};

#endif      // _USER_DATA_H_