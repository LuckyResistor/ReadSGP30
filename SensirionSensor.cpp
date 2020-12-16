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


#include "I2CBus.hpp"

#include <iostream>
#include <cmath>


namespace lr {


SensirionSensor::SensirionSensor(uint8_t chipAddress, int i2cBus, bool debuggingEnabled)
{
    _bus = new I2CBus(chipAddress, i2cBus);
    _bus->setDebugging(debuggingEnabled);
}


SensirionSensor::~SensirionSensor()
{
    if (_bus) { // forgot to close the bus?
        _bus->closeBus();
    }
    delete _bus;
}


SensirionSensor::Status SensirionSensor::openBus()
{
    if (hasError(_bus->openBus())) {
        return Status::Error;
    }
    return CallStatus::Success;
}


SensirionSensor::Status SensirionSensor::closeBus()
{
    if (_bus != nullptr) {
        if (hasError(_bus->closeBus())) {
            return Status::Error;
        }
        delete _bus;
        _bus = nullptr;
    }
    return CallStatus::Success;
}


SensirionSensor::Status SensirionSensor::sendRawCommand(uint16_t command)
{
    const uint8_t data[] = {
            static_cast<uint8_t>(command >> 8),
            static_cast<uint8_t>(command & 0x00ffu)};
    if (hasError(_bus->writeData(data, 2))) {
        return Status::Error;
    }
    return Status::Success;
}


SensirionSensor::Status SensirionSensor::sendRawCommand(uint16_t command, uint16_t value)
{
    uint8_t data[5];
    data[0] = static_cast<uint8_t>(command >> 8);
    data[1] = static_cast<uint8_t>(command & 0x00ffu);
    data[2] = static_cast<uint8_t>(value >> 8);
    data[3] = static_cast<uint8_t>(value & 0x00ffu);
    data[4] = getCrc8(&data[2], 2);
    if (hasError(_bus->writeData(data, 5))) {
        return Status::Error;
    }
    return CallStatus::Success;
}


SensirionSensor::Status SensirionSensor::sendRawCommand(uint16_t command, uint16_t value1, uint16_t value2)
{
    uint8_t data[8];
    data[0] = static_cast<uint8_t>(command >> 8);
    data[1] = static_cast<uint8_t>(command & 0x00ffu);
    data[2] = static_cast<uint8_t>(value1 >> 8);
    data[3] = static_cast<uint8_t>(value1 & 0x00ffu);
    data[4] = getCrc8(&data[2], 2);
    data[5] = static_cast<uint8_t>(value2 >> 8);
    data[6] = static_cast<uint8_t>(value2 & 0x00ffu);
    data[7] = getCrc8(&data[5], 2);
    if (hasError(_bus->writeData(data, 8))) {
        return Status::Error;
    }
    return CallStatus::Success;
}


StatusResult<uint16_t> SensirionSensor::readAndCheck(const uint8_t *data, int valueIndex)
{
    const auto expectedCrc = getCrc8(data, 2);
    const auto actualCrc = data[2];
    if (expectedCrc != actualCrc) {
        std::cerr << "CRC value " << valueIndex << " does not match." << std::endl;
        return StatusResult<uint16_t>::error();
    }
    const uint16_t value = (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]);
    return StatusResult<uint16_t>::success(value);
}


SensirionSensor::OneValueResult SensirionSensor::readOneValueResult()
{
    uint8_t data[3];
    if (hasError(_bus->readData(data, 3))) {
        return OneValueResult::error();
    }
    const auto result = readAndCheck(data, 1);
    if (hasError(result)) {
        return OneValueResult::error();
    }
    return OneValueResult::success(result.getValue());
}


SensirionSensor::TwoValuesResult SensirionSensor::readTwoValuesResult()
{
    const int numberOfValues = 2;
    uint8_t data[numberOfValues * 3];
    if (hasError(_bus->readData(data, numberOfValues * 3))) {
        return TwoValuesResult::error();
    }
    uint16_t values[numberOfValues];
    for (int i = 0; i < numberOfValues; ++i) {
        const auto result = readAndCheck(data + (i * 3), i + 1);
        if (hasError(result)) {
            return TwoValuesResult::error();
        }
    }
    return TwoValuesResult::success(std::make_tuple(values[0], values[1]));
}


SensirionSensor::ThreeValuesResult SensirionSensor::readThreeValuesResult()
{
    const int numberOfValues = 3;
    uint8_t data[numberOfValues * 3];
    if (hasError(_bus->readData(data, numberOfValues * 3))) {
        return ThreeValuesResult::error();
    }
    uint16_t values[numberOfValues];
    for (int i = 0; i < numberOfValues; ++i) {
        const auto result = readAndCheck(data + (i * 3), i + 1);
        if (hasError(result)) {
            return ThreeValuesResult::error();
        }
    }
    return ThreeValuesResult::success(std::make_tuple(values[0], values[1], values[2]));
}


uint8_t SensirionSensor::getCrc8(const uint8_t *data, int size)
{
    const uint8_t polynomial(0x31);
    uint8_t result(0xFF);
    for (int j = size; j; --j) {
        result ^= *data++;
        for (int i = 8; i; --i) {
            result = (result & 0x80) ? (result << 1) ^ polynomial : (result << 1);
        }
    }
    return result;
}


}


