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


#include "StatusTools.hpp"

#include <tuple>
#include <cstdint>


namespace lr {


class I2CBus;


class SensirionSensor
{
public:
    /// A simple call status.
    ///
    using Status = CallStatus;

public:
    /// ctor
    ///
    SensirionSensor(uint8_t chipAddress, int i2cBus = 1, bool debuggingEnabled = false);

    /// dtor
    ///
    virtual ~SensirionSensor();

public:
    /// Open the bus
    ///
    /// Call this once before issuing any commands.
    ///
    /// @return The call status.
    ///
    Status openBus();

    /// Close the bus
    ///
    /// Call this once at the end of the communication.
    ///
    /// @return The call status.
    ///
    Status closeBus();

protected:
    /// A result with one value.
    ///
    using OneValueResult = StatusResult<uint16_t>;

    /// A result with two values.
    ///
    using TwoValuesResult = StatusResult<std::tuple<uint16_t, uint16_t>>;

    /// A result with three values.
    ///
    using ThreeValuesResult = StatusResult<std::tuple<uint16_t, uint16_t, uint16_t>>;

    /// Send a command.
    ///
    /// @param command The command code to send.
    /// @return The call status.
    ///
    Status sendRawCommand(uint16_t command);

    /// Send a command.
    ///
    /// @param command The command code to send.
    /// @param value The parameter value to send.
    /// @return The call status.
    ///
    Status sendRawCommand(uint16_t command, uint16_t value);

    /// Send a command.
    ///
    /// @param command The command code to send.
    /// @param value1 The first parameter value to send.
    /// @param value2 The first parameter value to send.
    /// @return The call status.
    ///
    Status sendRawCommand(uint16_t command, uint16_t value1, uint16_t value2);

    /// Read a one value result and check the CRC.
    ///
    OneValueResult readOneValueResult();

    /// Read a two value result and check the CRCs.
    ///
    TwoValuesResult readTwoValuesResult();

    /// Read a three value result and check the CRCs.
    ///
    ThreeValuesResult readThreeValuesResult();

    /// Calculate CRC-8 as specified in the datasheet.
    ///
    static uint8_t getCrc8(const uint8_t *data, int size);

private:
    /// Read and check a value from the given array.
    ///
    static StatusResult<uint16_t> readAndCheck(const uint8_t *data, int valueIndex);

protected:
    I2CBus *_bus; ///< The I2C bus used to access the sensor.
};


}



