#ifdef SENSOR_HARDWARE

#include "sensors/sensor_group.h"
#include <vector>

using std::vector;

// Dummy sensors (testing)
#if SENSOR_HARDWARE == DUMMY
#include "sensors/dummy_sensor.h"

extern const int NUM_SENSOR_GROUPS = 1;

DummySensor _dummySensor;
vector<SensorGroup> _SENSOR_GROUPS = {
    SensorGroup(
        {
            &_dummySensor
        }
    )
};

// Sensor Pod sensor collection (SCD30 + Stemma Soil Sensor)
#elif SENSOR_HARDWARE == SENSOR_POD
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

// Hardware Interface Board sensor collection (RTC battery, feed level sensors)
#elif SENSOR_HARDWARE == HIB
#else
extern const int NUM_SENSOR_GROUPS = 1;
vector<SensorGroup> _SENSOR_GROUPS = {};
#endif

#endif      // SENSOR_HARDWARE
