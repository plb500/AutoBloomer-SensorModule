#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#include "util/debug_io.h"

#include "messages/multicore_mailbox.h"

#include "sensor/sensirion/sensirion_i2c_hal.h"
#include "sensor/sensirion/scd30_i2c.h"
#include "sensor/sensor_pod.h"



// Hardware defines for the SCD30
#define SCD30_I2C_PORT                  (i2c1)
static const uint8_t SCD30_I2C_SDA_PIN  = 6;
static const uint8_t SCD30_I2C_SCL_PIN  = 3;
static const uint SCD30_I2C_BAUDRATE    = (25 * 1000);

extern MulticoreMailbox coreMailbox;

I2CInterface scd30Interface = {
    .mI2C = SCD30_I2C_PORT,
    .mBaud = SCD30_I2C_BAUDRATE,
    .mSDA = SCD30_I2C_SDA_PIN,
    .mSCL = SCD30_I2C_SCL_PIN
};

SensorPod sensorPod = {
    .mSoilSensor = 0,
    .mSCD30Interface = &scd30Interface
};

void handle_set_temperature_offset_command(SensorPod *s, const char *commandParam) {
    double val;
    char *end;

    val = strtod(commandParam, &end);
    if(end == commandParam) {
        // Could not convert supplied value
        DEBUG_PRINT("Conversion error while setting temperature offset.");
        return;
    }

    set_sensor_pod_temperature_offset(s, val);
}

void handle_set_frc_command(SensorPod *s, const char *commandParam) {
    long val;
    char *end;

    val = strtol(commandParam, &end, 10);
    if(end == commandParam) {
        // Could not convert supplied value
        DEBUG_PRINT("Conversion error while setting FRC.");
        return;
    }

    set_sensor_pod_forced_recalibration_value(s, val);
}

// TODO: Should the queue be part of the SensorPod?
void process_sensor_control_commands(SensorPod *s, MulticoreMailbox *mailbox) {
    SensorControlMessage msg;
    bool msgRead = false;
    double temp;
    uint16_t frc;

    do {
        msgRead = get_waiting_sensor_control_message(mailbox, &msg);

        if(msgRead) {
            switch(msg.mCommand) {
                case SCD30_SET_TEMP_OFFSET:
                    handle_set_temperature_offset_command(s, msg.mCommandParams);
                    break;

                case SCD30_SET_FRC:
                    handle_set_frc_command(s, msg.mCommandParams);
                    break;
            }
        }
    } while(msgRead);
}


void sensor_pod_core_1_main() {
    multicore_lockout_victim_init();

    // Initialize the main container
    if(initialize_sensor_pod(&sensorPod)) {
        DEBUG_PRINT("Sensor pod initialized");
        start_sensor_pod(&sensorPod);
    } else {
        DEBUG_PRINT("Sensor pod initialization failed");
    }

    while(1) {
        // Check for sensor control messages
        process_sensor_control_commands(&sensorPod, &coreMailbox);

        // Update sensors and push any data to core0 if necessary
        update_sensor_pod(&sensorPod);
        
        if(sensor_pod_has_valid_data(&sensorPod)) {
            DEBUG_PRINT("+--------------------------------+");
            DEBUG_PRINT("|         SCD30 CO2: %04.2f PPM |", sensorPod.mCurrentData.mCO2Level);
            DEBUG_PRINT("| SCD30 Temperature: %02.2fC      |", sensorPod.mCurrentData.mTemperature);
            DEBUG_PRINT("|    SCD30 Humidity: %02.2f%%      |", sensorPod.mCurrentData.mHumidity);
            DEBUG_PRINT("+--------------------------------+\n");

            send_sensor_data_to_core0(&coreMailbox, &sensorPod.mCurrentData);
        } else {
            DEBUG_PRINT("+---------+");
            DEBUG_PRINT("| NO DATA |", sensorPod.mCurrentData.mCO2Level);
            DEBUG_PRINT("+---------+\n");
        }

        sleep_ms(500);
    }
}
