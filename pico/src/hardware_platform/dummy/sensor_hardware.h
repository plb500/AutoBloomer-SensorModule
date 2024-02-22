#ifndef _SENSOR_HARDWARE_H_
#define _SENSOR_HARDWARE_H_

#include "pico/types.h"

// Could not think of a better way of doing this. The data buffer sizes are naturally variable because
// our set of sensors is not fixed and depends on our current hardware implementation, but they also
// need to be known at compile time because when adding data to the Pico's queue implementation it 
// uses a straight memcpy and therefore won't copy across any dynamically allocated memory (i.e. byte
// arrays allocated with "new" etc), just the raw pointer values. Therefore the messages used in our
// queue need to contain fixed arrays of data. We could just pre-allocate large arrays and hope that
// it's enough that our sensors will never overflow them, and that's what I did for a while but it 
// seemed a bit gross (and wasteful of memory)
#include "sensors/dummy_sensor.h"

constexpr const uint32_t TOTAL_RAW_DATA_SIZE = (
    (DummySensor::RAW_DATA_SIZE + 2) * 2
);


#endif      // _SENSOR_HARDWARE_H_
