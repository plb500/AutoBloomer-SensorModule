#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"

// Menu files
#include "menu/menu_interface.h"
#include "menu/main_menu.h"

// Hardware files
#include "sensor/sensor_i2c_interface.h"
#include "sensor_pod.h"
#include "hardware/shift_register.h"


// Hardware defines
#define I2C_PORT                        (i2c1)
static const uint8_t I2C_SDA            = 10;
static const uint8_t I2C_SCL            = 11;
static const uint SENSOR_I2C_BAUDRATE   = (100 * 1000);

static const uint8_t LED_PIN            = 25;

static const uint8_t UART_TX_PIN        = 12;
static const uint8_t UART_RX_PIN        = 13;

static const uint8_t PISO_LATCH_PIN     = 21;
static const uint8_t PISO_CLOCK_PIN     = 14;
static const uint8_t PISO_DATA_PIN      = 15;



I2CInterface mainInterface = {
    I2C_PORT,
    SENSOR_I2C_BAUDRATE,
    I2C_SDA,
    I2C_SCL,
    DEFAULT_MULTIPLEXER_ADDRESS
};

SensorPod sensorPod = {
    .mInterface = &mainInterface,
    .mI2CChannel = I2C_CHANNEL_0,
    .mSCD30Address = SCD30_I2C_ADDRESS,
    .mSoilSensorAddress = SOIL_SENSOR_3_ADDRESS
};

ShiftRegister shiftRegister = {
    .mLatchPin = PISO_LATCH_PIN,
    .mClockPin = PISO_CLOCK_PIN,
    .mDataPin = PISO_DATA_PIN,
    .mType = PISO_SHIFT_REGISTER
};


int main() {
    char scd30Serial[SCD30_SERIAL_BYTE_SIZE];
    uint8_t scd30Firmware[2];

    // Setup UART
#if LIB_PICO_STDIO_UART
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
#endif

    // Setup I2C
    init_sensor_bus(&mainInterface);

    // Initialise I/O
    stdio_init_all(); 

    // Initialise LED GPIO
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Initialise shift register
    init_shift_register(&shiftRegister);

    sleep_ms(2000);

    // Initialize soil sensor
    printf("Initializing soil sensor....");
    if(init_soil_sensor(&sensorPod)) {
        if(reset_soil_sensor(&sensorPod)) {
            uint32_t ver = get_soil_sensor_version(&sensorPod);
            printf("\nSoil sensor initialized. Version: 0x%04X\n", ver);
        } else {
            printf(("Failed to reset soil sensor\n"));
        }
    } else {
        printf(("Failed to init soil sensor\n"));
    }
    printf("\n");

    // Initialize SCD30 sensor
    printf("Initializing SCD30....");
    if(read_scd30_serial(&sensorPod, scd30Serial) && read_scd30_firmware_version(&sensorPod, scd30Firmware)) {
        printf("\nSCD30 initialized\n");
        printf("  Serial: %s\n", scd30Serial);
        printf("  Firmware: 0x%02X  : 0x%02X\n", scd30Firmware[0], scd30Firmware[1]);
        printf("\n");
    } else {
        printf(("Failed to init SCD30\n"));
    }


    MainMenuObject menuObject = {
        .mLEDPin = LED_PIN,
        .mSensorPod = &sensorPod,
        .mShiftRegister = &shiftRegister
    };

    // Instantiate main menu and loop
    while(1) {
        do_menu(MAIN_MENU_TEXT, main_menu_handler, &menuObject);
    }
}
