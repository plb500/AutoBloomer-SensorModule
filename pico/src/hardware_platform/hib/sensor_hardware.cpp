#include "sensors/sensor_group.h"
#include "pico/stdlib.h"
#include "board_hardware/connection_io.h"
#include "board_hardware/hib_led_indicator.h"
#include "sensors/sensor_types/battery_sensor.h"
#include "sensors/sensor_types/sonar_sensor.h"
#include <vector>


using std::vector;


constexpr int BATTERY_SENSE_ENABLE_PIN              = 0;
constexpr int BATTERY_SENSE_PIN                     = 26;
constexpr uint RTC_BATTERY_ADC_PORT                 = 0;

constexpr int SONAR_SENSOR_L1_RX_PIN                = 4;
constexpr int SONAR_SENSOR_L1_TX_PIN                = 5;
constexpr int SONAR_SENSOR_L2_RX_PIN                = 6;
constexpr int SONAR_SENSOR_L2_TX_PIN                = 7;
constexpr int SONAR_SENSOR_R1_RX_PIN                = 27;
constexpr int SONAR_SENSOR_R1_TX_PIN                = 28;
constexpr int SONAR_SENSOR_R2_RX_PIN                = 21;
constexpr int SONAR_SENSOR_R2_TX_PIN                = 22;
constexpr int SONAR_SENSOR_BAUDRATE                 = 9600;

constexpr int HARDWARE_CONNECT_SR_LATCH_PIN         = 20;
constexpr int HARDWARE_CONNECT_SR_CLOCK_PIN         = 14;
constexpr int HARDWARE_CONNECT_SR_DATA_PIN          = 15;
constexpr int CONNECTION_INDICATOR_SR_LATCH_PIN     = 17;
constexpr int CONNECTION_INDICATOR_SR_CLOCK_PIN     = 16;
constexpr int CONNECTION_INDICATOR_SR_DATA_PIN      = 18;


constexpr int SONAR_SENSOR_L1_CONNECT_IDX           = 0;
constexpr int SONAR_SENSOR_L2_CONNECT_IDX           = 1;
constexpr int SONAR_SENSOR_R1_CONNECT_IDX           = 3;
constexpr int SONAR_SENSOR_R2_CONNECT_IDX           = 2;

constexpr int SONAR_SENSOR_L1_CONNECTED_IDX         = 0;
constexpr int SONAR_SENSOR_R1_CONNECTED_IDX         = 4;
constexpr int LED_L2_IDX                            = 1;
constexpr int LED_L3_IDX                            = 2;
constexpr int LED_L4_IDX                            = 3;
constexpr int LE_R2_IDX                             = 5;
constexpr int LED_R3_IDX                            = 6;
constexpr int LED_R4_IDX                            = 7;


ShiftRegister _hardwareConnectShiftRegister {
    HARDWARE_CONNECT_SR_DATA_PIN,
    HARDWARE_CONNECT_SR_LATCH_PIN,
    HARDWARE_CONNECT_SR_CLOCK_PIN,
    ShiftRegister::PISO_SHIFT_REGISTER,
    16
};

ShiftRegister _hardwareIndicateShiftRegister {
    CONNECTION_INDICATOR_SR_DATA_PIN,
    CONNECTION_INDICATOR_SR_LATCH_PIN,
    CONNECTION_INDICATOR_SR_CLOCK_PIN,
    ShiftRegister::SIPO_SHIFT_REGISTER,
    16
};

HIBLEDIndicator _ledIndicator {
    _hardwareIndicateShiftRegister,
    LED_L3_IDX
};

BatteryVoltageSensor _batterySensor(
    BATTERY_SENSE_ENABLE_PIN,
    BATTERY_SENSE_PIN,
    RTC_BATTERY_ADC_PORT
);

ConnectionIO _sonarL1ConnectionIO {
    _hardwareConnectShiftRegister, SONAR_SENSOR_L1_CONNECT_IDX,
    _hardwareIndicateShiftRegister, SONAR_SENSOR_L1_CONNECTED_IDX
};

ConnectionIO _sonarR1ConnectionIO {
    _hardwareConnectShiftRegister, SONAR_SENSOR_R1_CONNECT_IDX,
    _hardwareIndicateShiftRegister, SONAR_SENSOR_R1_CONNECTED_IDX
};

PIOWrapper _sonarPIOWrapper(pio0, 0, false);

SonarSensor _sonarSensorL1(
    _sonarPIOWrapper,
    0,
    SONAR_SENSOR_L1_TX_PIN,
    SONAR_SENSOR_L1_RX_PIN,
    SONAR_SENSOR_BAUDRATE,
    _sonarL1ConnectionIO
);

SonarSensor _sonarSensorR1(
    _sonarPIOWrapper,
    1,
    SONAR_SENSOR_R1_TX_PIN,
    SONAR_SENSOR_R1_RX_PIN,
    SONAR_SENSOR_BAUDRATE,
    _sonarR1ConnectionIO
);

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

extern const int NUM_SENSOR_GROUPS = 3;
WiFiIndicator* _wifiIndicator = &_ledIndicator;
