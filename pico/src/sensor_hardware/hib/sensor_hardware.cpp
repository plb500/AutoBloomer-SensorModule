#include "sensors/sensor_group.h"
#include "pico/stdlib.h"
#include "sensors/battery_sensor.h"
#include "sensors/sonar_sensor.h"
#include <vector>


using std::vector;


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
