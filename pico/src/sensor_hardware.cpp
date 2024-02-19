#ifdef SENSOR_HARDWARE

#include "sensors/sensor_group.h"
#include "pico/stdlib.h"
#include <vector>

using std::vector;

// Dummy sensors (testing)
#if SENSOR_HARDWARE == DUMMY
#include "sensors/dummy_sensor.h"

extern const int DEBUG_UART_TX_PIN      = 0;
extern const int DEBUG_UART_RX_PIN      = 1;
extern const int STDIO_UART_BAUDRATE    = 57600;
uart_inst_t* STDIO_UART                 = uart0;


DummySensor _dummySensorA;
DummySensor _dummySensorB;
extern const int NUM_SENSOR_GROUPS      = 2;
vector<SensorGroup> _SENSOR_GROUPS = {
    SensorGroup(
        {
            &_dummySensorA
        }
    ),
    SensorGroup(
        {
            &_dummySensorB
        }
    )
};

// Sensor Pod sensor collection (SCD30 + Stemma Soil Sensor)
#elif SENSOR_HARDWARE == SENSOR_POD
#include "sensors/scd30_sensor.h"
#include "sensors/stemma_soil_sensor.h"

extern const int DEBUG_UART_TX_PIN      = 0;
extern const int DEBUG_UART_RX_PIN      = 1;
extern const int STDIO_UART_BAUDRATE    = 57600;
uart_inst_t* STDIO_UART                 = uart0;

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
#include "sensors/battery_sensor.h"
#include "sensors/sonar_sensor.h"

extern const int DEBUG_UART_TX_PIN      = 0;
extern const int DEBUG_UART_RX_PIN      = 1;
extern const int STDIO_UART_BAUDRATE    = 57600;
uart_inst_t* STDIO_UART                 = uart0;

constexpr int BATTERY_SENSE_ENABLE_PIN  = 0;
constexpr int BATTERY_SENSE_PIN         = 26;
constexpr uint RTC_BATTERY_ADC_PORT     = 0;

constexpr int SONAR_SENSOR_L1_RX_PIN    = 4;
constexpr int SONAR_SENSOR_L1_TX_PIN    = 5;
constexpr int SONAR_SENSOR_L2_RX_PIN    = 6;
constexpr int SONAR_SENSOR_L2_TX_PIN    = 7;
constexpr int SONAR_SENSOR_R1_RX_PIN    = 27;
constexpr int SONAR_SENSOR_R1_TX_PIN    = 28;
constexpr int SONAR_SENSOR_R2_RX_PIN    = 21;
constexpr int SONAR_SENSOR_R2_TX_PIN    = 22;
constexpr int SONAR_SENSOR_BAUDRATE     = 9600;

BatteryVoltageSensor _batterySensor(
    BATTERY_SENSE_ENABLE_PIN,
    BATTERY_SENSE_PIN,
    RTC_BATTERY_ADC_PORT
);

PIOWrapper _sonarPIOWrapper(pio0, 0, false);

SonarSensor _sonarSensorL1(
    _sonarPIOWrapper,
    0,
    SONAR_SENSOR_L1_TX_PIN,
    SONAR_SENSOR_L1_RX_PIN,
    SONAR_SENSOR_BAUDRATE
);

SonarSensor _sonarSensorR1(
    _sonarPIOWrapper,
    1,
    SONAR_SENSOR_R1_TX_PIN,
    SONAR_SENSOR_R1_RX_PIN,
    SONAR_SENSOR_BAUDRATE
);

extern const int NUM_SENSOR_GROUPS = 3;
vector<SensorGroup> _SENSOR_GROUPS = {
    SensorGroup(
        {
            &_batterySensor
        }
    ),
    SensorGroup(
        {
            &_sonarSensorL1
        }
    ),
    SensorGroup(
        {
            &_sonarSensorR1
        }
    )
};

// Anything else (default state)
#else
extern const int NUM_SENSOR_GROUPS = 1;
vector<SensorGroup> _SENSOR_GROUPS = {};
#endif

#endif      // SENSOR_HARDWARE
