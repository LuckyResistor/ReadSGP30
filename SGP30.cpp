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
#include "SGP30.hpp"


#include "I2CBus.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <iomanip>
#include <sstream>


namespace lr {


using namespace std::chrono;


SGP30::SGP30(int i2cBus, bool debuggingEnabled)
    : SensirionSensor(0x58, i2cBus, debuggingEnabled)
{
}


SGP30::Status SGP30::initializeMeasurements()
{
    const auto result = sendCommand(Command::sgp30_iaq_init);
    std::this_thread::sleep_for(10ms);
    return result;
}


SGP30::MeasurentResult SGP30::readMeasurements()
{
    if (hasError(sendCommand(Command::sgp30_measure_iaq))) {
        return MeasurentResult::error();
    }
    std::this_thread::sleep_for(12ms);
    auto result = readTwoValuesResult();
    if (hasError(result)) {
        return MeasurentResult::error();
    }
    return MeasurentResult::success(result.getValue());
}


SGP30::Status SGP30::setHumidityCompensation(double temperatureCelsius, double relativeHumidity)
{
    if (temperatureCelsius < -100.0 || temperatureCelsius > 100.0) {
        std::cerr << "Temperature out of range.";
        return Status::Error;
    }
    if (relativeHumidity < 0.0 || relativeHumidity > 100.0) {
        std::cerr << "Relative humidity out of range.";
        return Status::Error;
    }
    // Calculate g/m3 water from rel. humidity and temperature.
    // This formula is from the datasheet of the sensor.
    const double humidityFactor = (relativeHumidity / 100);
    const double temperatureFactor = std::exp((17.62 * temperatureCelsius)/(243.12 + temperatureCelsius));
    const double absoluteHumidity = 216.7 * ((humidityFactor * 6.112 * temperatureFactor) / (273.15 + temperatureCelsius));
    const auto fixedPointValue = static_cast<uint16_t>(absoluteHumidity * 256.0);
    if (hasError(sendCommand(Command::sgp30_set_absolute_humidity, fixedPointValue))) {
        return Status::Error;
    }
    std::this_thread::sleep_for(10ms);
    return Status::Success;
}


SGP30::BaselineResult SGP30::getIAQBaseline()
{
    if (hasError(sendCommand(Command::sgp30_get_iaq_baseline))) {
        return BaselineResult::error();
    }
    std::this_thread::sleep_for(10ms);
    auto result = readTwoValuesResult();
    if (hasError(result)) {
        return BaselineResult::error();
    }
    return BaselineResult::success(result.getValue());
}


SGP30::Status SGP30::setIAQBaseline(const SGP30::BaselineValues &baselineValues)
{
    if (hasError(sendCommand(Command::sgp30_set_iaq_baseline, std::get<0>(baselineValues), std::get<1>(baselineValues)))) {
        return Status::Error;
    }
    std::this_thread::sleep_for(10ms);
    return Status::Success;
}


SensirionSensor::Status SGP30::makeMeasurementTest()
{
    if (hasError(sendCommand(Command::sgp30_measure_test))) {
        return Status::Success;
    }
    std::this_thread::sleep_for(220ms);
    auto result = readOneValueResult();
    if (hasError(result)) {
        return Status::Error;
    }
    if (result.getValue() != 0xd400) {
        std::cerr << "The measurement test returned: ";
        std::cerr << std::hex << std::setw(4) << std::setfill('0') << result.getValue();
        std::cerr << " expected 0xd400" << std::endl;
        return Status::Error;
    }
    return CallStatus::Success;
}


SGP30::SerialNumberResult SGP30::readSerialNumber()
{
    if (hasError(sendCommand(Command::sgp30_read_serial_number))) {
        return SerialNumberResult::error();
    }
    std::this_thread::sleep_for(10ms);
    auto result = readThreeValuesResult();
    if (hasError(result)) {
        return SerialNumberResult::error();
    }
    auto [value0, value1, value2] = result.getValue();
    std::stringstream serialString;
    serialString << std::hex << std::setw(4) << std::setfill('0') << value0 << value1 << value2;
    return SerialNumberResult::success(serialString.str());
}


SensirionSensor::Status SGP30::softReset()
{
    const uint8_t data[1] = {0x06};
    if (hasError(_bus->writeData(0x00, data, 1))) {
        return Status::Error;
    }
    return Status::Success;
}


}

