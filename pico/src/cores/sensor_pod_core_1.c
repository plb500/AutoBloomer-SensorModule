#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#include <stdio.h>

#include "sensor/sensor_i2c_interface.h"
#include "sensor/sensor_pod.h"
#include "cores/sensor_multicore_utils.h"

// Hardware defines
#define I2C_PORT                        (i2c1)
static const uint8_t I2C_SDA            = 6;
static const uint8_t I2C_SCL            = 3;
static const uint SENSOR_I2C_BAUDRATE   = (25 * 1000);


extern queue_t sensorUpdateQueue;

I2CInterface mainInterface = {
    I2C_PORT,
    SENSOR_I2C_BAUDRATE,
    I2C_SDA,
    I2C_SCL,
};

SensorPod sensorPod = {
    .mInterface = &mainInterface,
    .mSCD30Address = SCD30_I2C_ADDRESS,
    .mSoilSensorAddress = SOIL_SENSOR_3_ADDRESS
};


void sensor_pod_core_1_main() {
    multicore_lockout_victim_init();

    // Setup I2C
    init_sensor_bus(&mainInterface);

    if(initialize_sensor_pod(&sensorPod)) {
        printf("Sensor pod initialized\n");
    } else {
        printf("Sensor pod initialization failed\n");
    }

    while(1) {
        update_sensor_pod(&sensorPod);
        if(sensor_pod_has_valid_data(&sensorPod)) {
            printf("  SCD30 CO2: %f\n", sensorPod.mCurrentData.mCO2Level);
            printf("  SCD30 Temperature: %f\n", sensorPod.mCurrentData.mTemperature);
            printf("  SCD30 Humidity: %f\n", sensorPod.mCurrentData.mHumidity);

            push_sensor_data_to_queue(&sensorUpdateQueue, &sensorPod.mCurrentData);
        }
        printf("-----------------------------------------------\n");

        sleep_ms(500);
    }
}
