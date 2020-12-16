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
#include "Application.hpp"


namespace lr {


void Application::showHelp()
{
    std::cerr << "Usage: read_sgp30 [arguments]" << std::endl;
    std::cerr << " -h|--help   Display this help." << std::endl;
    std::cerr << " -r          Read the measurements (default)." << std::endl;
    std::cerr << " -i          Initialize the measurements." << std::endl;
    std::cerr << " -t          Perform a measurement test." << std::endl;
    std::cerr << " -s          Read serial number." << std::endl;
    std::cerr << " -z          Reset the sensor." << std::endl;
    std::cerr << " -b0 -b1     Select the bus. 1 is the default." << std::endl;
    std::cerr << " -d          Show debugging messages." << std::endl;
}


bool Application::parseCommandLine(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
        const auto arg = std::string(argv[i]);
        if (arg == "-h" || arg == "--help") {
            showHelp();
            return false;
        } else if (arg == "-d") {
            _debuggingEnabled = true;
        } else if (arg == "-r") {
            _action = Action::ReadMeasurements;
        } else if (arg == "-i") {
            _action = Action::InitializeMeasurements;
        } else if (arg == "-t") {
            _action = Action::MeasurementTest;
        } else if (arg == "-s") {
            _action = Action::ReadSerialNumber;
        } else if (arg == "-z") {
            _action = Action::SoftReset;
        } else if (arg == "-b0") {
            _bus = 0;
        } else if (arg == "-b1") {
            _bus = 1;
        } else {
            std::cerr << "Unknown argument \"" << arg << "\"." << std::endl;
            showHelp();
            return false;
        }
    }
    return true;
}


int Application::run(int argc, char **argv)
{
    if (!parseCommandLine(argc, argv)) {
        return 1;
    }
    lr::SGP30 sgpAccess(_bus, _debuggingEnabled);
    if (hasError(sgpAccess.openBus())) {
        return 1;
    }
    if (_action == Action::InitializeMeasurements) {
        const auto readResult = sgpAccess.initializeMeasurements();
        if (hasError(readResult)) {
            return 1;
        }
        // Output the values as JSON
        std::cout << R"({ "status": "init_success" })" << std::endl;
    } else if (_action == Action::ReadMeasurements) {
        const auto readResult = sgpAccess.readMeasurements();
        if (hasError(readResult)) {
            return 1;
        }
        // Output the values as JSON
        const auto [co2, tvoc] = readResult.getValue();
        std::cout << "{ \"co2_ppm\": " << co2 << ", \"tvoc_ppb\": " << tvoc << " }" << std::endl;
    } else if (_action == Action::MeasurementTest) {
        const auto readResult = sgpAccess.makeMeasurementTest();
        // Output the values as JSON
        if (hasError(readResult)) {
            std::cout << R"({ "status": "test_failure" })" << std::endl;
        } else {
            std::cout << R"({ "status": "test_success" })" << std::endl;
        }
    } else if (_action == Action::ReadSerialNumber) {
        const auto readResult = sgpAccess.readSerialNumber();
        if (hasError(readResult)) {
            return 1;
        }
        std::cout << R"({ "serial_number": ")" << readResult.getValue() << "\" }" << std::endl;
    } else if (_action == Action::SoftReset) {
        const auto readResult = sgpAccess.softReset();
        if (hasError(readResult)) {
            return 1;
        }
        std::cout << R"({ "status": "reset_successful" })" << std::endl;
    }
    // Close the bus.
    sgpAccess.closeBus();
    // Success
    if (_debuggingEnabled) {
        std::cout << "# Success." << std::endl;
    }
    return 0;
}


}

