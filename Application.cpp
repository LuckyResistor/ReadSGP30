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


#include "Configuration.hpp"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>


namespace lr {


namespace fs = std::filesystem;


#define LR_AD(ID, CMD, DESC) \
    {Application::Action::ID, std::string(CMD), &Application::handle##ID, DESC}

Application::ActionDefinitionList Application::_actionDefinitions = {
    LR_AD(ReadMeasurements, "-r", "Read the measurements (default)."),
    LR_AD(InitializeMeasurements, "-i", "Initialize the measurements."),
    LR_AD(MeasurementTest, "-t", "Perform a measurement test."),
    LR_AD(ReadSerialNumber, "-s", "Read serial number."),
    LR_AD(SoftReset, "-z", "Reset the sensor (and other sensors on the same bus!)."),
    LR_AD(StoreIAQBaseline, "-xs", "Store the iAQ baseline."),
    LR_AD(RestoreIAQBaseline, "-xr", "Restore the iAQ baseline."),
};


Application::Application()
:
    _debuggingEnabled(false),
    _action(Action::None),
    _bus(1),
    _sgp(nullptr)
{
}


void Application::showHelp()
{
    std::cerr << "Usage: read_sgp30 [arguments]\n";
    std::cerr << " -h --help    Display this help.\n";
    std::cerr << " -v --version Display the application version.\n";
    for (const auto &actionDefinition : _actionDefinitions) {
        std::cerr << " " << std::setw(12) << std::left << actionDefinition.command;
        std::cerr << " " << std::setw(0) << actionDefinition.description << '\n';
    }
    std::cerr << " -b0 -b1      Select the bus. 1 is the default.\n";
    std::cerr << " -d           Show debugging messages." << std::endl;
}


Application::ParsingStatus Application::parseCommandLine(int argc, char **argv)
{
    // Get an action definition.
    auto getActionDefinition = [](const std::string &arg) -> ActionDefinitionList::const_iterator {
        return std::find_if(
                _actionDefinitions.cbegin(),
                _actionDefinitions.cend(),
                [arg](const ActionDefinition &ad) {
                    return ad.command == arg;
                });
    };
    for (int i = 1; i < argc; ++i) {
        const auto arg = std::string(argv[i]);
        if (arg == "-h" || arg == "--help") {
            showHelp();
            return ParsingStatus::Success;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << R"({"app_version": ")" << cAppVersion << "\"}" << std::endl;
            return ParsingStatus::Success;
        } else if (arg == "-l" || arg == "--license") {
            std::cout << "Copyright (C)2020 Lucky Resistor\n\n"
                 "This program is free software: you can redistribute it and/or modify\n"
                 "it under the terms of the GNU General Public License as published by\n"
                 "the Free Software Foundation, either version 3 of the License, or\n"
                 "(at your option) any later version.\n\n"
                 "This program is distributed in the hope that it will be useful,\n"
                 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                 "GNU General Public License for more details.\n\n"
                 "You should have received a copy of the GNU General Public License\n"
                 "along with this program.  If not, see <https://www.gnu.org/licenses/>.\n" << std::endl;
            return ParsingStatus::Success;
        } else if (arg == "-d") {
            _debuggingEnabled = true;
        } else if (arg == "-b0") {
            _bus = 0;
        } else if (arg == "-b1") {
            _bus = 1;
        } else if (auto it = getActionDefinition(arg); it != _actionDefinitions.cend()) {
            if (_action != Action::None) {
                std::cerr << "You can only specify one action." << std::endl;
                return ParsingStatus::Failure;
            }
            _action = it->action;
        } else {
            std::cerr << "Unknown argument \"" << arg << "\"." << std::endl;
            showHelp();
            return ParsingStatus::Failure;
        }
    }
    if (_action == Action::None) {
        _action = Action::ReadMeasurements;
    }
    return ParsingStatus::RunAction;
}


int Application::run(int argc, char **argv)
{
    if (const auto result = parseCommandLine(argc, argv); result != ParsingStatus::RunAction) {
        return (result == ParsingStatus::Success) ? 0 : 1;
    }
    _sgp = new lr::SGP30(_bus, _debuggingEnabled);
    if (hasError(_sgp->openBus())) {
        return 1;
    }
    std::string result;
    const auto actionIt = std::find_if(
            _actionDefinitions.cbegin(),
            _actionDefinitions.cend(),
            [=](const ActionDefinition &ad) {
                return ad.action == _action;
            });
    if (actionIt != _actionDefinitions.cend()) {
        result = (this->*(actionIt->handler))();
    }
    if (result.empty()) {
        return 1;
    }
    std::cout << result << std::endl;
    _sgp->closeBus();
    delete _sgp;
    _sgp = nullptr;
    if (_debuggingEnabled) {
        std::cout << "# Success." << std::endl;
    }
    return 0;
}


std::string Application::handleInitializeMeasurements()
{
    const auto readResult = _sgp->initializeMeasurements();
    if (hasError(readResult)) {
        return std::string();
    }
    // Output the values as JSON
    return std::string(R"({ "status": "init_success" })");
}


std::string Application::handleReadMeasurements()
{
    const auto readResult = _sgp->readMeasurements();
    if (hasError(readResult)) {
        return std::string();
    }
    // Output the values as JSON
    const auto [co2, tvoc] = readResult.getValue();
    std::stringstream result;
    result << "{ \"co2_ppm\": " << co2 << ", \"tvoc_ppb\": " << tvoc << " }";
    return result.str();
}


std::string Application::handleMeasurementTest()
{
    const auto readResult = _sgp->makeMeasurementTest();
    // Output the values as JSON
    if (hasError(readResult)) {
        return std::string(R"({ "status": "test_failure" })");
    } else {
        return std::string(R"({ "status": "test_success" })");
    }
}


std::string Application::handleReadSerialNumber()
{
    const auto readResult = _sgp->readSerialNumber();
    if (hasError(readResult)) {
        return std::string();
    }
    std::stringstream result;
    result << R"({ "serial_number": ")" << readResult.getValue() << "\" }";
    return result.str();
}


std::string Application::handleSoftReset()
{
    const auto readResult = _sgp->softReset();
    if (hasError(readResult)) {
        return std::string();
    }
    return std::string(R"({ "status": "reset_successful" })");
}


std::string Application::handleStoreIAQBaseline()
{
    const auto readResult = _sgp->getIAQBaseline();
    if (hasError(readResult)) {
        return std::string();
    }
    const auto [a, b] = readResult.getValue();
    if (_debuggingEnabled) {
        std::cout << "# Read the baseline values 0x"
            << std::hex << std::setw(4) << std::setfill('0') << a
            << " and 0x" << b << " from the sensor." << std::endl;
    }
    try {
        fs::create_directories(getStorageDir());
    } catch (const fs::filesystem_error&) {
        // ignore any errors from this.
    }
    const auto baselineFile = getBaselineFile();
    auto tmpFile = baselineFile;
    if (_debuggingEnabled) {
        std::cout << "# Open file for write: " << tmpFile.string() << std::endl;
    }
    tmpFile.replace_extension(fs::path(".tmp"));
    std::ofstream fs(tmpFile);
    if (!fs.is_open()) {
        std::cerr << "Failed to open the storage file: " << baselineFile << std::endl;
        return std::string(R"({ "status": "store_failed" })");
    }
    fs << a << '\n' << b << '\n';
    fs.close();
    if (_debuggingEnabled) {
        std::cout << "# Rename file: " << tmpFile.string() << " => " << baselineFile.string() << std::endl;
    }
    try {
        fs::rename(tmpFile, baselineFile);
    } catch (const fs::filesystem_error &fse) {
        std::cerr << "Failed to rename the temporary storage file: " << tmpFile.string()
            << " Error: " << fse.what() << std::endl;
        return std::string(R"({ "status": "store_failed" })");
    }
    return std::string(R"({ "status": "store_successful" })");
}


std::string Application::handleRestoreIAQBaseline()
{
    const auto baselineFile = getBaselineFile();
    std::ifstream fs(baselineFile);
    if (_debuggingEnabled) {
        std::cout << "# Open file for read: " << baselineFile.string() << std::endl;
    }
    if (!fs.is_open()) {
        std::cerr << "Failed to open the storage file: " << baselineFile << std::endl;
        return std::string(R"({ "status": "restore_failed" })");
    }
    uint16_t a;
    uint16_t b;
    fs >> std::skipws >> a;
    fs >> std::skipws >> b;
    if (!fs.good()) {
        std::cerr << "Failed to read the values from the storage file: " << baselineFile << std::endl;
        return std::string(R"({ "status": "restore_failed" })");
    }
    if (_debuggingEnabled) {
        std::cout << "# Read the baseline values 0x"
                << std::hex << std::setw(4) << std::setfill('0')
                << a << " and 0x" << b << " from the file." << std::endl;
    }
    if (hasError(_sgp->setIAQBaseline(std::make_tuple(a, b)))) {
        std::cerr << "Failed to set the baseline values." << std::endl;
        return std::string(R"({ "status": "restore_failed" })");
    }
    return std::string(R"({ "status": "restore_failed" })");
}


fs::path Application::getStorageDir()
{
    fs::path result = std::getenv("HOME");
    result.append(".lr_read_sgp30");
    return result;
}


fs::path Application::getBaselineFile()
{
    auto result = getStorageDir();
    result.append("baseline.txt");
    return result;
}


}

