#include "user_data.h"

#include "hardware/flash.h"     // For flash erasing/writing
#include "hardware/sync.h"      // For disabling/enabling interrupts
#include <cstring>


// We reserved 4k in our custom linker script (memmap_custom.ld)
// This is exposed to code as ADDR_PERSISTENT
extern uint32_t ADDR_PERSISTENT[];
#define ADDR_PERSISTENT_BASE_ADDR           (ADDR_PERSISTENT)

// Will eventually use CRC or something here but I like being able to inspect the memory and see it
constexpr const char* const VALID_DATA_KEY   = "xXx This is valid data xXx";
constexpr int VALID_DATA_KEY_LENGTH          = 26;

extern const int NUM_SENSOR_GROUPS;
#define USER_DATA_FLASH_SIZE      (                                     \
    (UserData::MAX_SSID_LENGTH + 1) +                                   \
    (UserData::MAX_PSK_LENGTH + 1) +                                    \
    (UserData::MAX_HOST_NAME_LENGTH + 1) +                              \
    ((UserData::MAX_GROUP_LOCATION_LENGTH + 1) * NUM_SENSOR_GROUPS) +   \
    ((UserData::MAX_GROUP_NAME_LENGTH + 1) * NUM_SENSOR_GROUPS) +       \
    (UserData::MAX_BROKER_LENGTH + 1) +                                 \
    (VALID_DATA_KEY_LENGTH + 1)                                         \
)


UserData::UserData() : 
    mHostName(MAX_HOST_NAME_LENGTH, 0),
    mSSID(MAX_SSID_LENGTH, 0),
    mPSK(MAX_PSK_LENGTH, 0),
    mBrokerAddress(MAX_BROKER_LENGTH, 0)
{
    mScratchMemory = new char[USER_DATA_FLASH_SIZE];
    mSensorGroupLocations = new string[NUM_SENSOR_GROUPS];
    mSensorGroupNames = new string[NUM_SENSOR_GROUPS];
}

bool UserData::hasNetworkUserData() {
    return (
        !mSSID.empty() &&
        !mPSK.empty() &&
        !mHostName.empty()
    );
}

bool UserData::hasMQTTUserData() {
    return !mBrokerAddress.empty();
}

void UserData::setSSID(const char* ssid) {
    mSSID = ssid;
}

void UserData::setPSK(const char* psk) {
    mPSK = psk;
}

void UserData::setHostName(const char* name) {
    mHostName = name;
}

void UserData::setSensorGroupLocation(uint8_t groupIndex, const char* location) {
    assert(groupIndex < NUM_SENSOR_GROUPS);

    mSensorGroupLocations[groupIndex] = location;
}

void UserData::setSensorGroupName(uint8_t groupIndex, const char* name) {
    assert(groupIndex < NUM_SENSOR_GROUPS);

    mSensorGroupNames[groupIndex] = name;
}

void UserData::setBrokerAddress(const char* brokerAddress) {
    mBrokerAddress = brokerAddress;
}

void UserData::wipe() {
    mHostName.clear();
    mSSID.clear();
    mPSK.clear();
    mBrokerAddress.clear();

    for(int i = 0; i < NUM_SENSOR_GROUPS; ++i) {
        mSensorGroupLocations[i].clear();
        mSensorGroupNames[i].clear();
    }
}

void UserData::writeToFlash() {
    // Calculate the offset of flash memory at which our reserved memory area begins
    uint32_t persistentBaseAddress = (uint32_t) ADDR_PERSISTENT_BASE_ADDR;
    uint32_t offset = (persistentBaseAddress - XIP_BASE);

    // Calculate the total amount of flash space we will be writing
    int userDataSize = USER_DATA_FLASH_SIZE;
    int writeSize = (userDataSize / FLASH_PAGE_SIZE) + 1;                       // How many flash pages our data requires
    int sectorCount = ((writeSize * FLASH_PAGE_SIZE) / FLASH_SECTOR_SIZE) + 1;  // How many flash sectors this takes up
        
    // Actual byte counts
    uint32_t sectorBytesToErase = (FLASH_SECTOR_SIZE * sectorCount);            
    uint32_t pageBytesToWrite = (FLASH_PAGE_SIZE * writeSize);

    // Dump our data into the scratch area. We do this rather than just serializing directly from this object because we need
    // to disable interrupts (and also the other core) while we are writing to flash and we don't want to do that for
    // longer than necessary
    serializeToByteArray(mScratchMemory, USER_DATA_FLASH_SIZE);

    // Now copy into flash
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(offset, sectorBytesToErase);
    flash_range_program(offset, (uint8_t *) mScratchMemory, pageBytesToWrite);
    
    restore_interrupts(interrupts);
}

bool UserData::readFromFlash() {
    const char* flashContents = (const char *) ADDR_PERSISTENT;
    return serializeFromByteArray(flashContents, USER_DATA_FLASH_SIZE);
}

const string& UserData::getSSID() const {
    return mSSID;
}

const string& UserData::getPSK() const {
    return mPSK;
}

const string& UserData::getHostName() const {
    return mHostName;
}

const string& UserData::getSensorGroupLocation(uint8_t groupIndex) const {
    assert(groupIndex < NUM_SENSOR_GROUPS);

    return mSensorGroupLocations[groupIndex];
}

const string& UserData::getSensorGroupName(uint8_t groupIndex) const {
    assert(groupIndex < NUM_SENSOR_GROUPS);

    return mSensorGroupNames[groupIndex];
}

const string& UserData::getBrokerAddress() const {
    return mBrokerAddress;
}

int UserData::serializeToByteArray(char *bytes, int bytesSize) {
    if(!bytes || (bytesSize < USER_DATA_FLASH_SIZE)) {
        return 0;
    }

    char *writePtr = bytes;
    memset(writePtr, 0, USER_DATA_FLASH_SIZE);
    
    //      SERIALIZATION ORDER:
    //
    // - SSID
    // - WiFi PSK
    // - Hardware hostname
    // - <For each sensor group>
    //   - Group location
    //   - Group name
    // - Broker address
    // - Checksum/validation string
    memcpy(writePtr, mSSID.c_str(), mSSID.length());
    writePtr += (MAX_SSID_LENGTH + 1);

    memcpy(writePtr, mPSK.c_str(), mPSK.length());
    writePtr += (MAX_PSK_LENGTH + 1);

    memcpy(writePtr, mHostName.c_str(), mHostName.length());
    writePtr += (MAX_HOST_NAME_LENGTH + 1);

    for(int i = 0; i < NUM_SENSOR_GROUPS; ++i) {
        memcpy(writePtr, mSensorGroupLocations[i].c_str(), mSensorGroupLocations[i].length());
        writePtr += (MAX_GROUP_LOCATION_LENGTH + 1);

        memcpy(writePtr, mSensorGroupNames[i].c_str(), mSensorGroupNames[i].length());
        writePtr += (MAX_GROUP_NAME_LENGTH + 1);
    }

    memcpy(writePtr, mBrokerAddress.c_str(), mBrokerAddress.length());
    writePtr += (MAX_BROKER_LENGTH + 1);

    memcpy(writePtr, VALID_DATA_KEY, VALID_DATA_KEY_LENGTH);
    writePtr += (VALID_DATA_KEY_LENGTH + 1);

    return (writePtr - bytes);
}

bool UserData::serializeFromByteArray(const char *bytes, int bytesSize) {
    if(!bytes) {
        return false;
    }

    const char *readPtr = bytes;

    // Skip to our validation block and verify
    readPtr += (
        (MAX_SSID_LENGTH + 1) +
        (MAX_PSK_LENGTH + 1) +
        (MAX_HOST_NAME_LENGTH + 1) +
        ((MAX_GROUP_LOCATION_LENGTH + 1) * NUM_SENSOR_GROUPS) +
        ((MAX_GROUP_NAME_LENGTH + 1) * NUM_SENSOR_GROUPS) +
        (MAX_BROKER_LENGTH + 1)
    );
    // Check key
    size_t dataKeyLen = strlen(readPtr);
    if(!dataKeyLen) {
        // At the very least we should have a valid data key block before we set our data
        return false;
    }
    string dataKey = readPtr;
    if(dataKey != VALID_DATA_KEY) {
        // We had some key data but it looks corrupted
        return false;
    }

    // Rewind and read in user data
    readPtr = bytes;

    //      SERIALIZATION ORDER:
    //
    // - SSID
    // - WiFi PSK
    // - Hardware hostname
    // - <For each sensor group>
    //   - Group location
    //   - Group name
    // - Broker address
    // - Checksum/validation string
    mSSID = readPtr;
    readPtr += (MAX_SSID_LENGTH + 1);

    mPSK = readPtr;
    readPtr += (MAX_PSK_LENGTH + 1);

    mHostName = readPtr;
    readPtr += (MAX_HOST_NAME_LENGTH + 1);

    for(int i = 0; i < NUM_SENSOR_GROUPS; ++i) {
        mSensorGroupLocations[i] = readPtr;
        readPtr += (MAX_GROUP_LOCATION_LENGTH + 1);

        mSensorGroupNames[i] = readPtr;
        readPtr += (MAX_GROUP_NAME_LENGTH + 1);
    }

    mBrokerAddress = readPtr;

    return true;
}
