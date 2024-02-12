/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "sensirion_i2c_hal.h"
#include "sensirion_common.h"
#include "sensirion_config.h"

// Raspberry Pico includes
#include "hardware/i2c.h"
#include "hardware/gpio.h"

/*
 * INSTRUCTIONS
 * ============
 *
 * Implement all functions where they are marked as IMPLEMENT.
 * Follow the function specification in the comments.
 */

/*
 * I2C parameters which will be used in the sensirion functions
 *
 * Values MUST be populated prior to using below functions
 * e.g. by declaring them extern and setting them:
 * 
 * extern i2c_inst_t *sensirion_i2c_inst;
 * extern int sensirion_i2c_baud;
 * extern int sensirion_sda_pin;
 * extern int sensirion_scl_pin;
 *
 * void main() {
 *     sensirion_i2c_inst = i2c1;
 *     sensirion_i2c_baud = 100 * 1000;
 *     sensirion_sda_pin = 2;
 *     sensirion_scl_pin = 3;
 * }
 * 
 */
i2c_inst_t *sensirion_i2c_inst = NULL;
int sensirion_i2c_baud = 100 * 1000;
int sensirion_sda_pin = -1;
int sensirion_scl_pin = -1;

#define SENSIRION_I2C_PARAMS_VALID  (   \
    (sensirion_i2c_inst != NULL) &&         \
    (sensirion_sda_pin != -1) &&          \
    (sensirion_scl_pin != -1)             \
)


/**
 * Select the current i2c bus by index.
 * All following i2c operations will be directed at that bus.
 *
 * THE IMPLEMENTATION IS OPTIONAL ON SINGLE-BUS SETUPS (all sensors on the same
 * bus)
 *
 * @param bus_idx   Bus index to select
 * @returns         0 on success, an error code otherwise
 */
int16_t sensirion_i2c_hal_select_bus(uint8_t bus_idx) {
    /* TODO:IMPLEMENT or leave empty if all sensors are located on one single
     * bus
     */
    return NOT_IMPLEMENTED_ERROR;
}

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
void sensirion_i2c_hal_init(void) {
    if(!SENSIRION_I2C_PARAMS_VALID) {
        return;
    }

    i2c_init(sensirion_i2c_inst, sensirion_i2c_baud);
    gpio_set_function(sensirion_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(sensirion_scl_pin, GPIO_FUNC_I2C);

    gpio_pull_up(sensirion_sda_pin);
    gpio_pull_up(sensirion_scl_pin);
}

/**
 * Release all resources initialized by sensirion_i2c_hal_init().
 */
void sensirion_i2c_hal_free(void) {
    i2c_deinit(sensirion_i2c_inst);
}

/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count) {
    if(!SENSIRION_I2C_PARAMS_VALID) {
        return NOT_IMPLEMENTED_ERROR;
    }

    if(count) {
        int r = i2c_read_blocking(sensirion_i2c_inst, address, data, count, false);
        return (r > 0) ? !(r == count) : r;
    }
    return 0;
}

/**
 * Execute one write transaction on the I2C bus, sending a given number of
 * bytes. The bytes in the supplied buffer must be sent to the given address. If
 * the slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_hal_write(uint8_t address, const uint8_t* data,
                               uint16_t count) {
    if(!SENSIRION_I2C_PARAMS_VALID) {
        return NOT_IMPLEMENTED_ERROR;
    }

    int r = i2c_write_blocking(sensirion_i2c_inst, address, data, count, false); 
    return (r >= 0) ? 0 : r;
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * Despite the unit, a <10 millisecond precision is sufficient.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_i2c_hal_sleep_usec(uint32_t useconds) {
    sleep_us(useconds);
}
