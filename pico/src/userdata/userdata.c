#include "userdata.h"
#include <string.h>

#include "hardware/flash.h" // for the flash erasing and writing
#include "hardware/sync.h" // for the interrupts


#include <stdio.h>


// We reserved 4k in our custom linker script (memmap_custom.ld)
// This is exposed to code as ADDR_PERSISTENT
extern uint32_t ADDR_PERSISTENT[];
#define ADDR_PERSISTENT_BASE_ADDR           (ADDR_PERSISTENT)


void init_userdata(UserData *userData) {
    if(!userData) {
        return;
    }

    memset(userData, 0, sizeof(UserData));
    strncpy(userData->m_dataKey, VALID_DATA_KEY, VALID_DATA_KEY_LENGTH);
}

bool is_userdata_valid(UserData *userData) {
    if(!userData) {
        return false;
    }

    return (strncmp(userData->m_dataKey, VALID_DATA_KEY, VALID_DATA_KEY_LENGTH) == 0);
}

bool has_network_userdata(UserData *userData) {
    if(!userData) {
        return false;
    }

    return (
        strlen(userData->m_ssid) &&
        strlen(userData->m_psk) &&
        strlen(userData->m_sensorName)
    );
}

bool has_broker_details(UserData *userData) {
    if(!userData) {
        return false;
    }

    return strlen(userData->m_brokerAddress);
}


void write_userdata_to_flash(UserData *userData) {
    if(!userData) {
        return;
    }

    // Calculate the offset of flash memory at which our reserved memory area begins
    uint32_t persistentBaseAddress = (uint32_t) ADDR_PERSISTENT_BASE_ADDR;
    uint32_t offset = (persistentBaseAddress - XIP_BASE);


    int userDataSize = sizeof(UserData);
    // memcpy(flashBytes, userData, userDataSize);

    int writeSize = (userDataSize / FLASH_PAGE_SIZE) + 1;                       // How many flash pages our data requires
    int sectorCount = ((writeSize * FLASH_PAGE_SIZE) / FLASH_SECTOR_SIZE) + 1;  // How many flash sectors this takes up
        
    // Actual byte counts
    uint32_t sectorBytesToErase = (FLASH_SECTOR_SIZE * sectorCount);            
    uint32_t pageBytesToWrite = (FLASH_PAGE_SIZE * writeSize);

    uint32_t interrupts = save_and_disable_interrupts();

    flash_range_erase(offset, sectorBytesToErase);
    flash_range_program(offset, (uint8_t *) userData, pageBytesToWrite);
    
    restore_interrupts(interrupts);
}

bool read_userdata_from_flash(UserData *destUserData) {
    if(!destUserData) {
        return false;
    }

    const uint8_t* flashContents = (const uint8_t *) ADDR_PERSISTENT;
    memcpy(destUserData, flashContents, sizeof(UserData));

    return is_userdata_valid(destUserData);
}
