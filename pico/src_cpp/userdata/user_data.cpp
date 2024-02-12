#include "user_data.h"

#include "hardware/flash.h"     // For flash erasing/writing
#include "hardware/sync.h"      // For disabling/enabling interrupts
#include <cstring>


// We reserved 4k in our custom linker script (memmap_custom.ld)
// This is exposed to code as ADDR_PERSISTENT
extern uint32_t ADDR_PERSISTENT[];
#define ADDR_PERSISTENT_BASE_ADDR           (ADDR_PERSISTENT)

// Used for temporarily serializing/deserializing the class from flash
char scratchBlock[UserData::USER_DATA_FLASH_SIZE];


UserData::UserData() : 
    mLocationName(MAX_LOCATION_LENGTH, 0),
    mSensorName(MAX_SENSOR_NAME_LENGTH, 0),
    mSSID(MAX_SSID_LENGTH, 0),
    mPSK(MAX_PSK_LENGTH, 0),
    mBrokerAddress(MAX_BROKER_LENGTH, 0)
{}

bool UserData::hasNetworkUserData() {
    return (
        !mSSID.empty() &&
        !mPSK.empty() &&
        !mSensorName.empty()
    );
}

bool UserData::hasMQTTUserData() {
    return (
        !mBrokerAddress.empty() &&
        !mLocationName.empty() &&
        !mSensorName.empty()
    );
}

void UserData::setLocation(const char* location) {
    mLocationName = location;
}

void UserData::setSensorName(const char* name) {
    mSensorName = name;
}

void UserData::setSSID(const char* ssid) {
    mSSID = ssid;
}

void UserData:: setPSK(const char* psk) {
    mPSK = psk;
}

void UserData::setBrokerAddress(const char* brokerAddress) {
    mBrokerAddress = brokerAddress;
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
    serializeToByteArray(scratchBlock, USER_DATA_FLASH_SIZE);

    // Now copy into flash
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(offset, sectorBytesToErase);
    flash_range_program(offset, (uint8_t *) scratchBlock, pageBytesToWrite);
    
    restore_interrupts(interrupts);
}

bool UserData::readFromFlash() {
    const char* flashContents = (const char *) ADDR_PERSISTENT;
    return serializeFromByteArray(flashContents, USER_DATA_FLASH_SIZE);
}

int UserData::serializeToByteArray(char *bytes, int bytesSize) {
    if(!bytes || (bytesSize < USER_DATA_FLASH_SIZE)) {
        return 0;
    }

    char *writePtr = bytes;

    memset(writePtr, 0, USER_DATA_FLASH_SIZE);
    
    memcpy(writePtr, mLocationName.c_str(), mLocationName.length());
    writePtr += (mLocationName.length() + 1);

    memcpy(writePtr, mSensorName.c_str(), mSensorName.length());
    writePtr += (mSensorName.length() + 1);

    memcpy(writePtr, mSSID.c_str(), mSSID.length());
    writePtr += (mSSID.length() + 1);

    memcpy(writePtr, mPSK.c_str(), mPSK.length());
    writePtr += (mPSK.length() + 1);

    memcpy(writePtr, mBrokerAddress.c_str(), mBrokerAddress.length());
    writePtr += (mBrokerAddress.length() + 1);

    memcpy(writePtr, VALID_DATA_KEY, VALID_DATA_KEY_LENGTH);
    writePtr += (VALID_DATA_KEY_LENGTH + 1);

    return (writePtr - bytes);
}

bool UserData::serializeFromByteArray(const char *bytes, int bytesSize) {
    if(!bytes) {
        return false;
    }

    int locationNameLen, sensorNameLen, ssidLen, pskLen, brokerAddressLen, dataKeyLen = 0;
    const char *readPtr = bytes;

    // Each string in the byte array should be separated by a 0 so we should be able to repeatedly
    // call strlen on the supplied array to get each individual string
    locationNameLen = strlen(readPtr);
    readPtr += (locationNameLen + 1);

    sensorNameLen = strlen(readPtr);
    readPtr += (sensorNameLen + 1);

    ssidLen = strlen(readPtr);
    readPtr += (ssidLen + 1);

    pskLen = strlen(readPtr);
    readPtr += (pskLen + 1);

    brokerAddressLen = strlen(readPtr);
    readPtr += (brokerAddressLen + 1);

    // Check key
    dataKeyLen = strlen(readPtr);
    if(!dataKeyLen) {
        // At the very least we should have a valid data key block before we set our data
        return false;
    }
    string dataKey = readPtr;
    if(dataKey != VALID_DATA_KEY) {
        // We had some key data but it looks corrupted
        return false;
    }

    // Everything seems fine, go ahead and read in our blocks
    readPtr = bytes;
    
    mLocationName = readPtr;
    readPtr += (locationNameLen + 1);

    mSensorName = readPtr;
    readPtr += (sensorNameLen + 1);

    mSSID = readPtr;
    readPtr += (ssidLen + 1);

    mPSK = readPtr;
    readPtr += (pskLen + 1);

    mBrokerAddress = readPtr;
    readPtr += (brokerAddressLen + 1);

    return true;
}
