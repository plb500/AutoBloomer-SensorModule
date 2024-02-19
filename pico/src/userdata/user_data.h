#ifndef _USER_DATA_H_
#define _USER_DATA_H_

#include <string>

using std::string;

class UserData {
    public:
        UserData();

        bool hasNetworkUserData();
        bool hasMQTTUserData();

        void setSSID(const char* ssid);
        void setPSK(const char* psk);
        void setHostName(const char* name);
        void setSensorGroupLocation(uint8_t groupIndex, const char* location);
        void setSensorGroupName(uint8_t groupIndex, const char* name);
        void setBrokerAddress(const char* brokerAddress);
        void wipe();

        void writeToFlash();
        bool readFromFlash();

        const string& getSSID() const;
        const string& getPSK() const;
        const string& getHostName() const;
        const string& getSensorGroupLocation(uint8_t groupIndex) const;
        const string& getSensorGroupName(uint8_t groupIndex) const;
        const string& getBrokerAddress() const;

        static constexpr int MAX_SSID_LENGTH                = 32;
        static constexpr int MAX_PSK_LENGTH                 = 64;
        static constexpr int MAX_HOST_NAME_LENGTH           = 32;
        static constexpr int MAX_GROUP_LOCATION_LENGTH      = 32;
        static constexpr int MAX_GROUP_NAME_LENGTH          = 32;
        static constexpr int MAX_BROKER_LENGTH              = 256;

    private:
        int serializeToByteArray(char *bytes, int bytesSize);
        bool serializeFromByteArray(const char *bytes, int bytesSize);


        string mSSID;
        string mPSK;
        string mHostName;
        string* mSensorGroupLocations;
        string* mSensorGroupNames;
        string mBrokerAddress;
        char* mScratchMemory;   // Used for temporarily serializing/deserializing the class from flash
};

#endif      // _USER_DATA_H_