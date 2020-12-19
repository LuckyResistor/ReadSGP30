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


#include "SGP30.hpp"

#include <iostream>
#include <string>
#include <filesystem>
#include <vector>


namespace lr {


/// The application logic.
///
class Application
{
private:
    /// The actions
    ///
    enum class Action {
        None,
        InitializeMeasurements,
        ReadMeasurements,
        MeasurementTest,
        ReadSerialNumber,
        SoftReset,
        StoreIAQBaseline,
        RestoreIAQBaseline,
    };

    /// The action handler.
    ///
    using ActionHandler = std::string (Application::*)();

    /// Action definitions.
    ///
    struct ActionDefinition {
        Action action;
        std::string command;
        ActionHandler handler;
        std::string description;
    };

    /// The action definition list.
    ///
    using ActionDefinitionList = std::vector<ActionDefinition>;

    /// The argument parser status.
    ///
    enum class ParsingStatus {
        Success, ///< Quit the program successfully.
        Failure, ///< Quit the program with a failure.
        RunAction, ///< Run the configured action.
    };

public:
    /// ctor
    ///
    Application();

    /// Run the application.
    ///
    /// @param argc The argument count from the `main` function.
    /// @param argv The argument array from the `main` function.
    /// @return The return code of the program.
    ///
    int run(int argc, char **argv);

private:
    /// Show the help.
    ///
    static void showHelp();

    /// Parse the command line parameters.
    ///
    /// @param argc The argument count from the `main` function.
    /// @param argv The argument array from the `main` function.
    /// @return The status of the parsing.
    ///
    ParsingStatus parseCommandLine(int argc, char *argv[]);

    /// Handle the initialize measurement action.
    ///
    /// @return The JSON data to display, or empty string on any error.
    ///
    std::string handleInitializeMeasurements();

    /// Handle the read measurement action.
    ///
    /// @return The JSON data to display, or empty string on any error.
    ///
    std::string handleReadMeasurements();

    /// Handle the measurement test action.
    ///
    /// @return The JSON data to display, or empty string on any error.
    ///
    std::string handleMeasurementTest();

    /// Handle the read serial number action.
    ///
    /// @return The JSON data to display, or empty string on any error.
    ///
    std::string handleReadSerialNumber();

    /// Handle the soft reset action.
    ///
    /// @return The JSON data to display, or empty string on any error.
    ///
    std::string handleSoftReset();

    /// Handle the store iAQ baseline action.
    ///
    /// @return The JSON data to display, or empty string on any error.
    ///
    std::string handleStoreIAQBaseline();

    /// Handle the restore iAQ baseline action.
    ///
    /// @return The JSON data to display, or empty string on any error.
    ///
    std::string handleRestoreIAQBaseline();

    /// Get the directory to store sensor data.
    ///
    /// @return The path to the directory where data is stored.
    ///
    static std::filesystem::path getStorageDir();

    /// Get the path to the baseline file.
    ///
    /// @return The path to the baseline storage file.
    ///
    static std::filesystem::path getBaselineFile();

private:
    static ActionDefinitionList _actionDefinitions; ///< Action definitions.
    bool _debuggingEnabled; ///< If debugging shall be enabled.
    Action _action; ///< The requested _action.
    int _bus; ///< The I2C bus to use.
    lr::SGP30 *_sgp; ///< The sgp object used by all handler methods.
};


}

