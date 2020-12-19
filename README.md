
# ReadSGP30

A simple command line tool for Raspberry-Pi to read the values from an SGP30 sensor over the I2C bus.

## Basic Usage

There are a number of arguments you can use:

```
$ read_sgp30 -h
Usage: read_sgp30 [arguments]
 -h --help    Display this help.
 -v --version Display the application version.
 -r           Read the measurements (default).
 -i           Initialize the measurements.
 -t           Perform a measurement test.
 -s           Read serial number.
 -z           Reset the sensor (and other sensors on the same bus!).
 -xs          Store the iAQ baseline.
 -xr          Restore the iAQ baseline.
 -b0 -b1      Select the bus. 1 is the default.
 -d           Show debugging messages.
```

If you call the command, you will get JSON output:

```
$ read_sgp30
{ "co2_ppm": 5392, "tvoc_ppb": 48772 }
```

The idea is to call this command from your script and parse the returned JSON output.

## Important Notes

The following important notes are taken from the datasheet. Please read the sensor datasheet for details.

- You need to initialize the sensor once (using `-i`) after each reset or power cycle before you can do measurements.
- Call `-xs` every hour to store the baseline value in a file. After a power-loss or reset, restore this baseline with
  `-xr`. This will speedup the baseline calculation, which can otherwise take up to 48h.
- The soft reset function uses a general call address, which may also reset other sensors on the same bus.

## How to Compile and Install the Tool

In order to compile and install the tool on your Raspberry-Pi, you need to install the compiler,
CMake and the I2C-Tools first:

```
sudo apt install gcc cmake i2c-tools
```

Download the files of this git repository to a sub directory in your home directory. In this example, I downloaded the
files to the directory `/home/pi/read_sgp30`.

Now create a separate build directory. In this example, I create the directory `/home/pi/build_read_sgp30`.

Next switch to the build directory, execute cmake to create the build script. Build the executable with make and
install the executable in your system with sudo make install.

```
cd build_read_sht31
cmake ../read_sht31
make
sudo make install
```

## License (GPL v3)

Copyright (c) 2020 by Lucky Resistor.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see
https://www.gnu.org/licenses/.
