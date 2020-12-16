#pragma once
//
// (c)2020 by Lucky Resistor. See LICENSE for details.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//


#include "SensirionSensor.hpp"


namespace lr {


/// A class to access the SGP30 sensor.
///
class SGP30 : public SensirionSensor
{
public:
    /// The measurement values.
    ///
    using MeasurentResult = StatusResult<std::tuple<uint16_t, uint16_t>>;

    /// The baseline values.
    ///
    using BaselineValues = std::tuple<uint16_t, uint16_t>;

    /// The read baseline result.
    ///
    using BaselineResult = StatusResult<BaselineValues>;

    /// The serial number result.
    ///
    using SerialNumberResult = StatusResult<std::string>;

public:
    /// Create a new access object for the SHT32 sensor.
    ///
    /// @param i2cBus The I2C bus to use.
    /// @param debuggingEnabled If debugging messages shall be enabled.
    ///
    SGP30(int i2cBus = 1, bool debuggingEnabled = false);

    /// dtor
    ///
    ~SGP30() override = default;

public:
    /// Initialize measurements.
    ///
    /// After a reset of the sensor or power cycle, you have to call this method once to initialize
    /// measurements on the chip. It will take up to 15 seconds, until the sensor will return valid values.
    ///
    /// @return The call status.
    ///
    Status initializeMeasurements();

    /// Read a measurement.
    ///
    /// You should poll this method in regular intervals of one second.
    ///
    /// @return The first value is the CO2 equivalent in PPM, the second value is TVOC in PPB.
    ///
    MeasurentResult readMeasurements();

    /// Get the iAQ baseline.
    ///
    /// Use this function to read the current iAQ baseline. The idea is to store these values
    /// in regular intervals and write them back after a reset to speed up the the calculation
    /// of the readings. See sensor datasheet for details.
    ///
    /// @return The baseline values.
    ///
    BaselineResult getIAQBaseline();

    /// Set the iAQ baseline.
    ///
    /// Set the stored iAQ baseline values after a reset to speed-up the calculations.
    /// See sensor datasheet for details.
    ///
    /// @param baselineValues The baseline values.
    /// @return The call status.
    ///
    Status setIAQBaseline(const BaselineValues &baselineValues);

    /// Set humidity compensation.
    ///
    /// This will set the humidity compensation for the chip.
    ///
    /// @param temperatureCelsius The temperature in celsius.
    /// @param relativeHumidity The relative humidity in percent.
    /// @return The call status.
    ///
    Status setHumidityCompensation(double temperatureCelsius, double relativeHumidity);

    /// Make a measure test.
    ///
    /// @return Success if the test was successful.
    ///
    Status makeMeasurementTest();

    /// Read the serial number.
    ///
    SerialNumberResult readSerialNumber();

    /// Make a soft reset.
    ///
    /// This will affect all sensors on the bus.
    ///
    Status softReset();

private:
    /// The commands
    ///
    enum class Command {
        sgp30_reset = 0x0006,
        sgp30_iaq_init = 0x2003,
        sgp30_measure_iaq = 0x2008,
        sgp30_get_iaq_baseline = 0x2015,
        sgp30_set_iaq_baseline = 0x201e,
        sgp30_set_absolute_humidity = 0x2061,
        sgp30_measure_test = 0x2032,
        sgp30_get_feature_set = 0x202f,
        sgp30_measure_raw = 0x2050,
        sgp30_get_tvoc_inceptive_baseline = 0x20b3,
        sgp30_set_tvoc_baseline = 0x2077,
        sgp30_read_serial_number = 0x3682,
    };

    // command wrapper methods.
    inline Status sendCommand(Command command) {
        return sendRawCommand(static_cast<uint16_t>(command));
    }
    inline Status sendCommand(Command command, uint16_t value) {
        return sendRawCommand(static_cast<uint16_t>(command), value);
    }
    inline Status sendCommand(Command command, uint16_t value1, uint16_t value2) {
        return sendRawCommand(static_cast<uint16_t>(command), value1, value2);
    }
};


}

