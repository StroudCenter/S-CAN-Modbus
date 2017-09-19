# scan-Modbus
This library was created to communicate between an Arduino device and an [s::can](https://www.s-can.at/en/) [spectro::lyzer](https://www.s-can.at/en/products/spectrometer-probes) via [Modbus](https://en.wikipedia.org/wiki/Modbus)/[RS485](https://en.wikipedia.org/wiki/RS-485).  It is dependent on the [SensorModbusMaster](https://github.com/EnviroDIY/SensorModbusMaster) library.  It is NOT intended to replace ana::pro, ana::lyte, or moni::tools as not all of the necessary settings for the spectro::lyzer can be controlled or set using the modbus commands.  It is intended to allow spectro::lyzer data to be continuously uploaded to a network via wifi or cellular data streams without the very large power draw of a con::nect or con::lyte system.

In our arrangements, we are powering both the spectro::lyzer and the Arduino by way of an [s::can con:nect](http://www.s-can.at/en/products/terminals-software#).  The con::nect itself is powered by a deep cycle marine battery with an attached solar panel.  The con::nect is also necessary for the RS485 terminals.  The RS-485 A and B terminals on the con::nect are wired to a RS-485 to TTL board and from there to the serial port of the Arduino.  When looking for an adapter board, be mindful of the TTL voltage levels!  Because of the large quanitities of data going quickly between the Arduino and the spectro::lyzer, I very strongly reccommend using a hardware serial port on the Arduino.  Hardware serial is also the only connection method which supports the spectro::lyzer's default of _odd_ parity.

Please be cautious when using this library as much of the modbus mapping is not documented by s::can (including the registers containing the fingerprint data).  Some of the documented functions actually do not work as described, either.  The most completely mapping s::can provides of the modbus registers is in the manual for the con::cube, but this is still woefully incomplete.  The remainder of the mapping I figured out myself by repeatedly scanning all of the registers on the spectro::lyzer and comparing the results with the data available in ana::pro.  (I used the "[scanRegisters](https://github.com/EnviroDIY/SensorModbusMaster/blob/master/utils/scanRegisters/scanRegisters.ino)" utility in the [SensorModbusMaster](https://github.com/EnviroDIY/SensorModbusMaster) library for this.)  The [FullSpecModbusMap.xlsx](https://github.com/StroudCenter/S-CAN-Modbus/blob/master/FullSpecModbusMap.xlsx) in this folder is my best guess of the full register mapping.  There are still holes in my modbus map (which s::can has not been forthcoming about filling), so if you have any further information about the modbus mappings of the spectro::lyzer, PLEASE let me know.  All issues and pull requests are welcome.  NONE of this is in any way, shape, or form sanctioned by s::can!  Please do not blame me if you "brick" your spec!
_______

## Specto::lyzer settings "safely" changable via modbus
- Sensor modbus address
- Serial port settings (baudrate/parity)
- Initiating logging mode (undocumented)

## Specto::lyzer settings NOT changable via modbus
- Cleaning mode, interval, and duration
    - S::can's formal documentation provides registers with this information, but the spectro::lyzer does NOT properly respond to or commands to show or change these values
- Sleep mode settings
- Measurement parameters (flashrate, lamp voltage, etc)

## Further specto::lyzer settings accessible via modbus
These settings are accessible, and theoretically changable, via modbus, but I would strongly recommend using ana::pro to change them.
- Current scan::point name
- Current device time
- Curernt parameter setup and local calibration (partially documented)
- Current global calibration name (undocumented)
- Current reference information, including all measurement parameters and absorbances of all stored references (undocumented)

## Data/results available via modbus
This data can be accessed, but not changed
- Hardware meta-data (model, vendor, version, serial number)
- Device status
- Time most recent parameters were measured
- Status and values of each parameter (single most recent measurement, only when in logging mode)
- Fingerprint detector type, data source, and spectral path length (single most recent measurement, only when in logging mode, undocumented)
- Time and status of fingerprint measurements (single most recent measurement, only when in logging mode, undocumented)
- Full spectral values of 8 types of fingerprint data  (single most recent measurement, only when in logging mode, undocumented)
    - 0 … Fingerprint [Abs/m]
    - 1 … Turbidity compensated fingerprint [Abs/m]
    - 2 … First derivative of fingerprint [Abs/m/m]
    - 3 … Difference between current print and print in memory [Abs/m]
    - 4 … Percent Transmission [%/cm2]
    - 5 … First derivative of turbidity compensated fingerprint  [Abs/m/m]
    - 6 … Percent Transmission per 10cm2  [%/10cm2]
    - 7 … UNKNOWN

## Data NOT accessible via modbus
- Partial least squares components underlying global calibrations
- Sample data used for local calibrations
- "Historical" measurement data
    - only the single most recent data point is available
- Any data when the logger is not in "logging" mode
    - If the logger is connected to ana::pro, ana::lyte, or moni::tools and recording data in either manual or automatic mode, the data sent to the controller will not be accessible via modbus.
