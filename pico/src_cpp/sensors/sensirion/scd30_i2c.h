/*
 * THIS FILE IS AUTOMATICALLY GENERATED
 *
 * Generator:    sensirion-driver-generator 0.9.1
 * Product:      scd30
 * Version:      None
 */
/*
 * Copyright (c) 2022, Sensirion AG
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

#ifndef SCD30_I2C_H
#define SCD30_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sensirion_config.h"
#define SCD30_I2C_ADDR_61 0x61

typedef enum {
    START_PERIODIC_MEASUREMENT_CMD_ID = 0x10,
    STOP_PERIODIC_MEASUREMENT_CMD_ID = 0x104,
    SET_MEASUREMENT_INTERVAL_CMD_ID = 0x4600,
    GET_MEASUREMENT_INTERVAL_CMD_ID = 0x4600,
    GET_DATA_READY_CMD_ID = 0x202,
    READ_MEASUREMENT_DATA_CMD_ID = 0x300,
    ACTIVATE_AUTO_CALIBRATION_CMD_ID = 0x5306,
    GET_AUTO_CALIBRATION_STATUS_CMD_ID = 0x5306,
    FORCE_RECALIBRATION_CMD_ID = 0x5204,
    GET_FORCE_RECALIBRATION_STATUS_CMD_ID = 0x5204,
    SET_TEMPERATURE_OFFSET_CMD_ID = 0x5403,
    GET_TEMPERATURE_OFFSET_CMD_ID = 0x5403,
    GET_ALTITUDE_COMPENSATION_CMD_ID = 0x5102,
    SET_ALTITUDE_COMPENSATION_CMD_ID = 0x5102,
    READ_FIRMWARE_VERSION_CMD_ID = 0xd100,
    SOFT_RESET_CMD_ID = 0xd304,
} cmd_id_t;

/**
 * @brief Initialize i2c address of driver
 *
 * @param[in] i2c_address Used i2c address
 *
 */
void init_driver(uint8_t i2c_address);

/**
 * @brief Poll the data ready flag.
 *
 * Repeatedly call the get_data_ready() until the ready flag is set to 1. A the
 * minimal measuremnt interval is 2s we iterate at most 200 times.  Note that
 * this is blocking the system for a considerable amount of time!
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_await_data_ready();

/**
 * @brief Block until data is available and return measurement results.
 *
 * This is a convenience method that combines polling the data ready flag and
 * reading out the data.  Note that this is blocking the system for a
 * considerable amount of time!
 *
 * @param[out] co2_concentration
 * @param[out] temperature
 * @param[out] humidity
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_blocking_read_measurement_data(float* co2_concentration,
                                             float* temperature,
                                             float* humidity);

/**
 * @brief Start continuous measurement of CO2, relative humidity and
 * temperature.
 *
 * Starts continuous measurement of the SCD30 to measure CO₂ concentration,
 * humidity and temperature. Measurement data which is not read from the sensor
 * will be overwritten. The CO₂ measurement value can be compensated for ambient
 * pressure by feeding the pressure value in mBar to the sensor. Setting  the
 * ambient  pressure  will  overwrite  previous  settings  of  altitude
 * compensation. Setting  the  argument  to  zero  will deactivate the ambient
 * pressure compensation(default ambient pressure = 1013.25 mBar). For setting a
 * new ambient pressure when continuous measurement is running the whole command
 * has to be written to SCD30.
 *
 * @param[in] ambient_pressure Ambient pressure in millibar.
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_start_periodic_measurement(uint16_t ambient_pressure);

/**
 * @brief Stops continuous measurements of the sensor.
 *
 * Stops the continuous measurement of the SCD30.
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_stop_periodic_measurement();

/**
 * @brief Sets the interval used to measure in continuous measurement.
 *
 * Sets the interval used byt he SCD30 sensor to measure in continuous
 * measurement mode. Initial value is 2s.The chosen measurement interval is
 * saved in non-volatile memory and thus is not reset to its initial value after
 * power up.
 *
 * @param[in] interval Measurement interval in seconds.
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_set_measurement_interval(uint16_t interval);

/**
 * @brief Read the configured measurement interval.
 *
 * Reads out the active measurement interval.
 *
 * @param[out] interval Configured measurement interval
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_get_measurement_interval(uint16_t* interval);

/**
 * @brief Query if data is ready for readout.
 *
 * Data ready command is used  to  determine if a measurement can be read from
 * the sensor’s buffer. Whenever there is a measurement available from the
 * internal buffer this command returns 1 and 0 otherwise. As soon as the
 * measurement has been read by SCD30 the return value changes to 0.
 *
 * @param[out] data_ready_flag Data ready flag
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_get_data_ready(uint16_t* data_ready_flag);

/**
 * @brief Read out the measurement values
 *
 * Allows to read new measurement data if data is available.
 *
 * @param[out] co2_concentration
 * @param[out] temperature
 * @param[out] humidity
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_read_measurement_data(float* co2_concentration,
                                    float* temperature, float* humidity);

/**
 * @brief Activates or deactivates continuous automatic self calibration.
 *
 * Continuous automatic self-calibration (ASC) can be (de-)activated with this
 * command. When activated for the first time a period of minimum 7 days is
 * needed so that the algorithm can find its initial parameter set for ASC. The
 * sensor has to be exposed to fresh air for at least 1 hour every day. Also
 * during that period, the sensor may not be disconnected from the power supply.
 * Otherwise the procedure to find calibration parameters is aborted and has to
 * be restarted from the beginning. The successfully calculated parameters are
 * stored in non-volatile memory of the SCD30 having the effect that after a
 * restart the previously found parameters for ASC are still present.
 *
 * @param[in] do_activate Set activate flag.
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_activate_auto_calibration(uint16_t do_activate);

/**
 * @brief scd30_get_auto_calibration_status
 *
 * Read out the status of the active self calibration.
 *
 * @param[out] is_active Indication if automatic calibration is active
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_get_auto_calibration_status(uint16_t* is_active);

/**
 * @brief Forces recalibration with a new value for the CO2 concentration.
 *
 * Forced recalibration (FRC) is used to compensate for sensor drifts when a
 * reference value of the CO₂ concentration in close proximity to the SCD30 is
 * available. For best results, the sensor has to be run in a stable environment
 * in continuous mode at a measurement rate of 2s for at least two minutes
 * before applying the FRC commandand sending the reference value. Setting a
 * reference CO₂ concentration by the method described here will always
 * supersede corrections from the ASC (see command activate_auto_calibration)
 * and vice-versa. The reference CO₂ concentration has to be within the range
 * 400 ppm ≤ cref(CO₂) ≤ 2000 ppm. The FRC method imposes a permanent update of
 * the CO₂ calibration curve which persists after repowering the sensor. The
 * most recently used reference value is retained in volatile memory and can be
 * read out with the command sequence given below. After repowering the sensor,
 * the command will return the standard reference value of 400 ppm.
 *
 * @param[in] co2_ref_concentration New CO2 reference concentration.
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_force_recalibration(uint16_t co2_ref_concentration);

/**
 * @brief scd30_get_force_recalibration_status
 *
 * Read out the CO₂ reference concentration.
 *
 * @param[out] co2_ref_concentration Currently used CO2 reference concentration.
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_get_force_recalibration_status(uint16_t* co2_ref_concentration);

/**
 * @brief Set the temperature offset. Unit C * 100
 *
 * The on-board RH/T sensor is influenced by thermal self-heating of SCD30 and
 * other electrical components. Design-in alters the thermal properties of SCD30
 * such that temperature and humidity offsets may occur when operating the
 * sensor in end-customer devices. Compensation of those effects is achievable
 * by writing the temperature offset found in continuous operation of the device
 * into the sensor. Temperature offset value is saved in non-volatile memory.
 * The last set value will be used for temperature offset compensation after
 * repowering
 *
 * @param[in] temperature_offset
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_set_temperature_offset(uint16_t temperature_offset);

/**
 * @brief Get the temperature offset. Unit ℃ * 100.
 *
 * Read out the actual temperature offset. The result can be converted to ℃ by
 * dividing it by 100.
 *
 * @param[out] temperature_offset
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_get_temperature_offset(uint16_t* temperature_offset);

/**
 * @brief Get the configured altitude (height over sea level in m).
 *
 * Read out the configured altitude (height in [m] over sea level).
 *
 * @param[out] altitude
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_get_altitude_compensation(uint16_t* altitude);

/**
 * @brief Set a new value for altitude.
 *
 * Measurements of CO₂ concentration based on the NDIR principle are influenced
 * by altitude. SCD30 offers to compensate deviations due to altitude by using
 * this command. Setting altitude is disregarded when an ambient pressure is
 * given to the sensor (see command start_periodic_measurement). Altitude value
 * is saved in non-volatile memory. The last set value will be used for altitude
 * compensation after repowering.
 *
 * @param[in] altitude
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_set_altitude_compensation(uint16_t altitude);

/**
 * @brief Read the version of the current firmware.
 *
 * Read the version of the current firmware.
 *
 * @param[out] major Major version number.
 * @param[out] minor Minor version number.
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_read_firmware_version(uint8_t* major, uint8_t* minor);

/**
 * @brief scd30_soft_reset
 *
 * The SCD30 provides a soft reset mechanism that forces the sensor into the
 * same state as after powering up without the need for removing the power-
 * supply. It does so by restarting its system controller. After soft reset the
 * sensor will reload all calibrated data. However, it is worth noting that the
 * sensor reloads calibration data prior to every measurement by default. This
 * includes previously set reference values from ASC or FRC as well as
 * temperature offset values last setting. The sensor is able to receive the
 * command at any time, regardless of its internal state.
 *
 * @return error_code 0 on success, an error code otherwise.
 */
int16_t scd30_soft_reset();

#ifdef __cplusplus
}
#endif
#endif  // SCD30_I2C_H
