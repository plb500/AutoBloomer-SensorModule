#include "pico/stdlib.h"
#include "pico/multicore.h"

#include <stdio.h>

// Hardware defines
#define I2C_PORT                        (i2c1)
static const uint8_t I2C_SDA            = 6;
static const uint8_t I2C_SCL            = 3;
static const uint SENSOR_I2C_BAUDRATE   = (25 * 1000);



void sensor_pod_core_1_main() {
    multicore_lockout_victim_init();

    while(1) {
        printf("CORE 1 RUNNING....\n");
        sleep_ms(500);
    }
}
