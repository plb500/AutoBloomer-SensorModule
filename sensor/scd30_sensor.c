#include "scd30_sensor.h"

#include <stdio.h>


const uint8_t CRC_LOOKUP[] = {
    0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97, 0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F, 0x2E, 
    0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4, 0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F, 0x5C, 0x6D,
    0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11, 0x3F, 0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8,
    0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52, 0x7C, 0x4D, 0x1E, 0x2F, 0xB8, 0x89, 0xDA, 0xEB,
    0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA, 0x84, 0xB5, 0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13,
    0x7E, 0x4F, 0x1C, 0x2D, 0xBA, 0x8B, 0xD8, 0xE9, 0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50,
    0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C, 0x02, 0x33, 0x60, 0x51, 0xC6, 0xF7, 0xA4, 0x95,
    0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F, 0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6,
    0x7A, 0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC, 0xED, 0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54,
    0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F, 0xAE, 0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17,
    0xFC, 0xCD, 0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B, 0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2,
    0xBF, 0x8E, 0xDD, 0xEC, 0x7B, 0x4A, 0x19, 0x28, 0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91,
    0x47, 0x76, 0x25, 0x14, 0x83, 0xB2, 0xE1, 0xD0, 0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B, 0x58, 0x69,
    0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93, 0xBD, 0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A,
    0xC1, 0xF0, 0xA3, 0x92, 0x05, 0x34, 0x67, 0x56, 0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF,
    0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15, 0x3B, 0x0A, 0x59, 0x68, 0xFF, 0xCE, 0x9D, 0xAC
};

const uint8_t READ_DELAY_MS                             = 4;

const uint8_t MAX_SCD30_RESPONSE_WORDS                  = 16;   // Doesn't look like we'll ever receive more than 16 words in a single response
const uint8_t SCD30_RESPONSE_WORD_SIZE                  = 2;
const uint8_t SCD30_RESPONSE_WORD_BYTE_COUNT            = (SCD30_RESPONSE_WORD_SIZE + 1);  // +1 byte for the CRC

const uint16_t SCD30_CMD_START_CONTINUOUS_MEASUREMENT   = 0x0010;
const uint16_t SCD30_CMD_STOP_CONTINUOUS_MEASUREMENT    = 0x0104;
const uint16_t SCD30_CMD_READ_MEASUREMENT               = 0x0300;
const uint16_t SCD30_CMD_SET_MEASUREMENT_INTERVAL       = 0x4600;
const uint16_t SCD30_CMD_GET_DATA_READY                 = 0x0202;
const uint16_t SCD30_CMD_SET_TEMPERATURE_OFFSET         = 0x5403;
const uint16_t SCD30_CMD_SET_ALTITUDE                   = 0x5102;
const uint16_t SCD30_CMD_SET_FORCED_RECALIBRATION       = 0x5204;
const uint16_t SCD30_CMD_AUTO_SELF_CALIBRATION          = 0x5306;
const uint16_t SCD30_CMD_READ_FIRMWARE_VERSION          = 0xD100;
const uint16_t SCD30_CMD_READ_SERIAL                    = 0xD033;
const uint16_t SCD30_CMD_SOFT_RESET                     = 0xD304;

const uint16_t SCD30_NUM_SERIAL_WORDS                   = 16;
const uint16_t SCD30_SERIAL_BYTE_SIZE                   = (SCD30_NUM_SERIAL_WORDS * SCD30_RESPONSE_WORD_SIZE) + 1;


// Internal functions
uint8_t calc_crc(uint8_t *data, size_t len);
bool validate_bytes(uint8_t *data, size_t len, uint8_t checksum);
float bytes_to_float(uint8_t *data);
bool write_scd30_cmd(SCD30Sensor *sensor, uint16_t commandCode, uint16_t *args, uint8_t numArgs);
bool write_scd30_cmd_no_args(SCD30Sensor *sensor, uint16_t commandCode);
bool read_scd30_response_words_into_bytes(SCD30Sensor *sensor, uint8_t numWords, uint8_t *dst);
bool get_scd30_data_ready_status(SCD30Sensor *sensor);
// -- End internal functions

uint8_t calc_crc(uint8_t *data, size_t len) {
    uint8_t crc = 0xFF;
    for(int i = 0; i < len; i++) {
        crc = CRC_LOOKUP[crc ^ data[i]];
    }
    return crc;
}

bool validate_bytes(uint8_t *data, size_t len, uint8_t checksum) {
    return (calc_crc(data, len) == checksum);
}

float bytes_to_float(uint8_t *data) {
    float outputValue;

    uint32_t tmp = (
        (((uint32_t) data[0]) << 24) |
        (((uint32_t) data[1]) << 16) |
        (((uint32_t) data[2]) << 8) |
        ((uint32_t) data[3])
    );

    return *((float *) &tmp);
}

bool write_scd30_cmd(SCD30Sensor *sensor, uint16_t commandCode, uint16_t *args, uint8_t numArgs) {
    // Datasheet says max number of arguments is 1, but lets give ourselves space for 4, just in case
    if(numArgs > 4) {
        return false;
    }

    // The total number of bytes we could output is:
    // 2 bytes for command code
    // 3 bytes per argument (2 bytes for value, 1 byte for checksum)
    // Therefore max 4 arguments = (4 x 3) + 2 = 14 bytes
    // We'll allocate a 32 byte buffer for breathing room
    const uint8_t OUTPUT_BUFFER_SIZE = 32;
    uint8_t outputBuffer[OUTPUT_BUFFER_SIZE];
    uint8_t bufferIndex = 0;

    // Insert command code bytes
    outputBuffer[bufferIndex++] = (commandCode & 0xFF00) >> 8;
    outputBuffer[bufferIndex++] = (commandCode & 0x00FF);

    // Append arguments
    if(args) {
        for(int i = 0; i < numArgs; ++i) {
            uint8_t *argDataLocation = &outputBuffer[bufferIndex];
            uint16_t arg = args[i];

            outputBuffer[bufferIndex++] = (arg & 0xFF00) >> 8;
            outputBuffer[bufferIndex++] = (arg & 0x00FF);
            outputBuffer[bufferIndex++] = calc_crc(argDataLocation, 2);
        }
    }

    return write_i2c_data(sensor->mInterface, sensor->mAddress, outputBuffer, bufferIndex);
}

bool write_scd30_cmd_no_args(SCD30Sensor *sensor, uint16_t commandCode) {
    return write_scd30_cmd(sensor, commandCode, 0, 0);
}

bool read_scd30_response_words_into_bytes(SCD30Sensor *sensor, uint8_t numWords, uint8_t *dst) {
    uint8_t incomingBuffer[MAX_SCD30_RESPONSE_WORDS * SCD30_RESPONSE_WORD_BYTE_COUNT];
    uint16_t bytesToRead = (numWords * SCD30_RESPONSE_WORD_BYTE_COUNT);

    if(!read_from_i2c(sensor->mInterface, sensor->mAddress, incomingBuffer, bytesToRead)) {
        return false;
    }

    // Validate the CRC on each word
    for(int root = 0, dstIndex = 0; root < bytesToRead; root += SCD30_RESPONSE_WORD_BYTE_COUNT) {
        if(!validate_bytes(&incomingBuffer[root], SCD30_RESPONSE_WORD_SIZE, incomingBuffer[root + SCD30_RESPONSE_WORD_SIZE])) {
            return false;
        }
        dst[dstIndex++] = incomingBuffer[root];
        dst[dstIndex++] = incomingBuffer[root + 1];
    }

    return true;
}

bool write_and_confirm_cmd_args(SCD30Sensor *sensor, uint16_t commandCode, uint16_t commandParam) {
    uint8_t confirmValue[2];

    // Write value
    if(!write_scd30_cmd(sensor, commandCode, &commandParam, 1)) {
        return false;
    }

    // Read value back and confirm
    if(!write_scd30_cmd_no_args(sensor, commandCode)) {
        return false;
    }

    sleep_ms(READ_DELAY_MS);

    // Get response
    if(!read_scd30_response_words_into_bytes(sensor, 1, (uint8_t *) &confirmValue)) {
        return false;
    }

    return commandParam == (((uint16_t) confirmValue[0]) << 8) | confirmValue[1];
}


        // Public functions

bool trigger_scd30_continuous_measurement(SCD30Sensor *sensor, uint16_t pressureCompensation) {
    return write_scd30_cmd(sensor, SCD30_CMD_START_CONTINUOUS_MEASUREMENT, &pressureCompensation, 1);
}

bool stop_scd30_continuous_measurement(SCD30Sensor *sensor) {
    return write_scd30_cmd_no_args(sensor, SCD30_CMD_STOP_CONTINUOUS_MEASUREMENT);
}

bool set_scd30_measurement_interval(SCD30Sensor *sensor, uint16_t measurementInterval) {
    return write_and_confirm_cmd_args(sensor, SCD30_CMD_SET_MEASUREMENT_INTERVAL, measurementInterval);
}

bool get_scd30_data_ready_status(SCD30Sensor *sensor) {
    uint16_t response;
    uint8_t numResponseWords = 1;

    if(!write_scd30_cmd_no_args(sensor, SCD30_CMD_GET_DATA_READY)) {
        return false;
    }

    sleep_ms(READ_DELAY_MS);

    if(!read_scd30_response_words_into_bytes(sensor, numResponseWords, (uint8_t *) &response)) {
        return false;
    }

    return response;
}

SCD30SensorData get_scd30_reading(SCD30Sensor *sensor) {
    const int NUM_READING_RESPONSE_WORDS = 6;
    uint8_t dataBuffer[MAX_SCD30_RESPONSE_WORDS * SCD30_RESPONSE_WORD_SIZE];

    SCD30SensorData returnData = {
        .mValidReading          = false,
        .mCO2Reading            = -1.f,
        .mTemperatureReading    = -1.f,
        .mHumidityReading       = -1.f
    };

    // Check whether there is a reading available
    if(!get_scd30_data_ready_status(sensor)) {
        return returnData;
    }

    // Request reading
    if(!write_scd30_cmd_no_args(sensor, SCD30_CMD_READ_MEASUREMENT)) {
        return returnData;
    }

    sleep_ms(READ_DELAY_MS);

    // Get byte response
    if(!read_scd30_response_words_into_bytes(sensor, NUM_READING_RESPONSE_WORDS, dataBuffer)) {
        return returnData;
    }

    // Convert and store bytes
    returnData.mValidReading = true;
    returnData.mCO2Reading = bytes_to_float(&dataBuffer[0]);
    returnData.mTemperatureReading = bytes_to_float(&dataBuffer[4]);
    returnData.mHumidityReading = bytes_to_float(&dataBuffer[8]);

    return returnData;
}

bool set_scd30_automatic_self_calibration(SCD30Sensor *sensor, bool selfCalibrationOn) {
    uint16_t param = selfCalibrationOn ? 1 : 0;
    return write_and_confirm_cmd_args(sensor, SCD30_CMD_AUTO_SELF_CALIBRATION, param);
}

bool set_scd30_forced_recalibration_value(SCD30Sensor *sensor, uint16_t referenceValue) {
    return write_and_confirm_cmd_args(sensor, SCD30_CMD_AUTO_SELF_CALIBRATION, referenceValue);
}

bool set_scd30_temperature_offset(SCD30Sensor *sensor, uint16_t temperatureOffset) {
    return write_and_confirm_cmd_args(sensor, SCD30_CMD_SET_TEMPERATURE_OFFSET, temperatureOffset);
}

bool set_scd30_altitude_compensation(SCD30Sensor *sensor, uint16_t altitude) {
    return write_and_confirm_cmd_args(sensor, SCD30_CMD_SET_ALTITUDE, altitude);
}

bool read_scd30_firmware_version(SCD30Sensor *sensor, uint8_t *dst) {
    uint8_t numResponseWords = 1;

    if(!write_scd30_cmd_no_args(sensor, SCD30_CMD_READ_FIRMWARE_VERSION)) {
        return false;
    }

    sleep_ms(READ_DELAY_MS);

    if(!read_scd30_response_words_into_bytes(sensor, numResponseWords, dst)) {
        return false;
    }

    return true;
}

bool do_scd30_soft_reset(SCD30Sensor *sensor) {
    return write_scd30_cmd_no_args(sensor, SCD30_CMD_SOFT_RESET);
}

bool read_scd30_serial(SCD30Sensor *sensor, char *dst) {
    if(!write_scd30_cmd_no_args(sensor, SCD30_CMD_READ_SERIAL)) {
        sprintf(dst, "NO SERIAL");
        return false;
    }

    sleep_ms(READ_DELAY_MS);

    if(!read_scd30_response_words_into_bytes(sensor, SCD30_NUM_SERIAL_WORDS, dst)) {
        sprintf(dst, "NO SERIAL");
        return false;
    }

    dst[SCD30_SERIAL_BYTE_SIZE] = 0;
    return true;
}
