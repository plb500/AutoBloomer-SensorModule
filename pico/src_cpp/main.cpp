#include "util/debug_io.h"
#include "userdata/user_data.h"
#include "messaging/multicore_mailbox.h"
#include "cores/core_0_executor.h"
#include "cores/core_1_executor.h"

#include "pico/multicore.h"
#include <optional>


using std::nullopt;
using std::optional;


// Sensor hardware definitions ////////////////////////////////////////
#define SCD30_I2C_PORT                      (i2c0)
static const uint8_t SCD30_I2C_SDA_PIN      = 4;
static const uint8_t SCD30_I2C_SCL_PIN      = 5;
static const uint8_t SCD30_POWER_CTL_PIN    = 6;
static const uint SCD30_I2C_BAUDRATE        = (25 * 1000);

optional<I2CInterface> scd30Interface = I2CInterface(
    SCD30_I2C_PORT,
    SCD30_I2C_BAUDRATE,
    SCD30_I2C_SDA_PIN,
    SCD30_I2C_SCL_PIN
);

optional<StemmaSoilSensor> soilSensor;
///////////////////////////////////////////////////////////////////////

MulticoreMailbox multicoreMailbox;

// Core data wrappers
Core0Executor dataCore0(multicoreMailbox);
Core1Executor dataCore1(
    multicoreMailbox,
    soilSensor,
    scd30Interface
);


int main() {
    // Setup stdio
    DEBUG_PRINT_INIT()

    sleep_ms(3000);

    // Initialize cores
    dataCore0.initialize();
    Core0Executor::setExecutor(dataCore0);

    dataCore1.initialize();
    Core1Executor::setExecutor(dataCore1);

    // Run our separate cores
    multicore_launch_core1(Core1Executor::loop);
    Core0Executor::loop();

    return 1;
}