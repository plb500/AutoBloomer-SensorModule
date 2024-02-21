#include "sensors/sensor_group.h"
#include "pico/stdlib.h"
#include <vector>

using std::vector;

#include "sensors/scd30_sensor.h"
#include "sensors/stemma_soil_sensor.h"

#define SCD30_I2C_PORT                      (i2c0)
static const uint8_t SCD30_I2C_SDA_PIN      = 4;
static const uint8_t SCD30_I2C_SCL_PIN      = 5;
static const uint8_t SCD30_POWER_CTL_PIN    = 6;
static const uint SCD30_I2C_BAUDRATE        = (25 * 1000);

#define STEMMA_I2C_PORT                     (i2c1)
static const uint8_t STEMMA_I2C_SDA_PIN     = 2;
static const uint8_t STEMMA_I2C_SCL_PIN     = 3;
static const uint STEMMA_I2C_BAUDRATE       = (25 * 1000);

I2CInterface _scd30Interface = I2CInterface(
    SCD30_I2C_PORT,
    SCD30_I2C_BAUDRATE,
    SCD30_I2C_SDA_PIN,
    SCD30_I2C_SCL_PIN
);

I2CInterface _stemmaInterface = I2CInterface(
    STEMMA_I2C_PORT,
    STEMMA_I2C_BAUDRATE,
    STEMMA_I2C_SDA_PIN,
    STEMMA_I2C_SCL_PIN
);

SCD30Sensor _scd30Sensor(
    _scd30Interface,
    SCD30_POWER_CTL_PIN
);

StemmaSoilSensor _stemmaSensor(
    _stemmaInterface,
    StemmaSoilSensor::SOIL_SENSOR_3_ADDRESS
);

extern const int NUM_SENSOR_GROUPS = 1;
vector<SensorGroup> _SENSOR_GROUPS = {
    SensorGroup(
        {
            &_scd30Sensor,
            &_stemmaSensor
        }
    )
};
