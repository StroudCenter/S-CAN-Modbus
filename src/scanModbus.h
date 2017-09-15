/*
 *scanModbus.h
*/

#ifndef scanModbus_h
#define scanModbus_h

#include <Arduino.h>
#include <SensorModbusMaster.h>  // For modbus communication
#include <TimeLib.h>  // for dealing with the TAI64/Unix time


//----------------------------------------------------------------------------
//                        ENUMERATIONS FOR CONFIGURING DEVICE
//----------------------------------------------------------------------------

// The communcations modes
typedef enum specCommMode
{
    modbusRTU = 0,
    modbusASCII,
    modbusTCP
} specCommMode;

// The possible serial baud rates
typedef enum specBaudRate
{
    b9600 = 0,
    b19200,
    b38400
} specBaudRate;

// The possible serial parities
typedef enum specParity
{
    noParity = 0,
    even,
    odd
} specParity;

// The possible cleaning modes
typedef enum cleaningMode
{
    noCleaning = 0,
    manual,
    automatic
} cleaningMode;

// The possible spectral sources
typedef enum spectralSource
{
    fingerprint = 0,  // The absorption spectrum as it is measured (fingerprint)[Abs/m]
    compensFP = 1,  // The turbidity-compensated fingerprint [Abs/m]
    derivFP = 2,  // The first derivative of the measured fingerprint (i.e. gradient)[Abs/m]
    diff2oldorgFP = 3,  // The difference between the current fingerprint and the previous one in memory [Abs/m]
    transmission = 4,  // The percent transmission - NOT linear wrt concentrations [%/cm2]
    derivcompFP = 5,  // The first derivative of the turbidity-compensated fingerprint [Abs/m]
    transmission10 = 6,  // The percent transmission per 10 cm2 [%/10cm2]
    other = 7  // I don't know what this is, but the modbus registers on the spec have 8 groups of fingerprints..
} spectralSource;

// The possible spectral sources
typedef enum detectorType
{
    UV = 0,
    UVVis = 1
} detectorType;


//*****************************************************************************
//*****************************************************************************
//*****************************The S::CAN class********************************
//*****************************************************************************
//*****************************************************************************
class scan
{

public:

//----------------------------------------------------------------------------
//                          GENERAL USE FUNCTIONS
//----------------------------------------------------------------------------

    // This function sets up the communication
    // It should be run during the arduino "setup" function.
    // The "stream" device must be initialized prior to running this.
    // The default baud rate for a spectro::lyser or s::can controller is 34800
    // The default parity is **ODD**
    // Per modbus specifications, there is:
    //    - 1 start bit
    //    - 8 data bits, least significant bit sent first
    //    - 1 stop bit if parity is used-2 bits if no parity
    // Note that neither SoftwareSerial, AltSoftSerial, nor NeoSoftwareSerial
    // will support the default odd parity!  This means you must either use a
    // corretnly set up HARDWARE serial port on your Arduino or change the
    // parity setting of the s::can using some other program before connecting
    // it to your Arduino.
    bool begin(byte modbusSlaveID, Stream *stream, int enablePin = -1);
    bool begin(byte modbusSlaveID, Stream &stream, int enablePin = -1);

    // This prints out all of the setup information to the selected stream
    bool printSetup(Stream *stream);
    bool printSetup(Stream &stream);

    // This resets all settings to default
    // Please note that after this you will most likely have to re-begin
    // your stream and sensor after running this function because the
    // baud rate and parity may have changed.  Again, keep in mind that the
    // default parity is ODD, which is not supported by SoftwareSerial.
    bool resetSettings(void);

    // This just returns the slave ID that was entered in the begin function.
    // If you don't know your slave ID, you must find it some other way
    byte getSlaveID(void){return _slaveID;}

    // This sets a new modbus slave ID
    bool setSlaveID(byte newSlaveID);

    // This returns the current device status as a bitmap
    int getDeviceStatus(void);
    // This parses the device status bitmap and prints the resuts to the stream
    void printDeviceStatus(uint16_t bitmask, Stream *stream);
    void printDeviceStatus(uint16_t bitmask, Stream &stream);

    // This returns the current system status as a bitmap
    // It would be nice if I knew what register this data was in...
    int getSystemStatus(void){return 0;}
    // Prints out the current system status
    void printSystemStatus(uint16_t bitmask, Stream *stream);
    void printSystemStatus(uint16_t bitmask, Stream &stream);

    // This "wakes" the spectro::lyzer so it's ready to communicate"
    bool wakeSpec(void);

//----------------------------------------------------------------------------
//           FUNCTIONS TO RETURN THE ACTUAL SAMPLE TIMES AND VALUES
//----------------------------------------------------------------------------

    // Last measurement time as a 32-bit count of seconds from Jan 1, 1970
    uint32_t getParameterTime(void);
    // This prints out the sample time, formatted as YYYY.MM.DD hh:mm:ss
    void printParameterTime(Stream *stream, bool addNL=true);
    void printParameterTime(Stream &stream, bool addNL=true);
    // This gets values back from the sensor and puts them into a previously
    // initialized float variable.  The actual return from the function is an
    // integer which is a bit-mask describing the parameter status.
    int getParameterValue(int parmNumber, float &value);
    // This parses the parameter status bitmap and prints the resuts to the stream
    void printParameterStatus(uint16_t bitmask, Stream *stream);
    void printParameterStatus(uint16_t bitmask, Stream &stream);
    // This prints the data from ALL parameters as delimeter separated data.
    // By default, the delimeter is a TAB (\t, 0x09), as expected by the s::can/ana::xxx software.
    // This includes the parameter timestamp and status.
    // NB:  You can use this to print to a file on a SD card!
    void printParameterData(Stream *stream, const char *dlm="    ");
    void printParameterData(Stream &stream, const char *dlm="    ");

    // Last measurement time as a 32-bit count of seconds from Jan 1, 1970
    uint32_t getFingerprintTime(spectralSource source=fingerprint);
    // This prints out the sample time, formatted as YYYY.MM.DD hh:mm:ss
    void printFingerprintTime(Stream *stream, bool addNL=true, spectralSource source=fingerprint);
    void printFingerprintTime(Stream &stream, bool addNL=true, spectralSource source=fingerprint);
    // This returns detector type used for the fingerprint
    detectorType getFingerprintDetectorType(spectralSource source=fingerprint);
    // This returns the spectral source type used for the fingerprint
    spectralSource getFingerprintSource(spectralSource source=fingerprint);
    // This returns the spectral source type used for the fingerprint
    int getFingerprintPathLength(spectralSource source=fingerprint);
    // This gets spectral values from the sensor and puts them into a previously
    // initialized float array.  The array must have space for 221 values!
    // The actual return from the function is an integer which is a bit-mask
    // describing the fingerprint status (or, well, it would be if I could figure
    // out which register that value lived in).
    int getFingerprintData(float fpArray[], spectralSource source=fingerprint);
    // This parses the parameter status bitmap and prints the resuts to the stream
    // That is, pending me figuring out the right register for that data...
    void printFingerprintStatus(uint16_t bitmask, Stream *stream);
    void printFingerprintStatus(uint16_t bitmask, Stream &stream);
    // This prints the fingerprint data as delimeter separated data.
    // By default, the delimeter is a TAB (\t, 0x09), as expected by the s::can/ana::xxx software.
    // This includes the fingerprint timestamp and status
    // NB:  You can use this to print to a file on a SD card!
    void printFingerprintData(Stream *stream, const char *dlm="    ",
                              spectralSource source=fingerprint);
    void printFingerprintData(Stream &stream, const char *dlm="    ",
                              spectralSource source=fingerprint);

    // This is for the first line of both headers (below)
    void printHeader(Stream *stream);
    void printHeader(Stream &stream);
    // This prints out a header for a "par" file ini the format that the
    // s::can/ana::xxx software is expecting
    // The delimeter is changable, but if you use anything other than the
    // default TAB (\t, 0x09) the s::can/ana::xxx software will not read it.
    void printParameterHeader(Stream *stream, const char *dlm="    ");
    void printParameterHeader(Stream &stream, const char *dlm="    ");
    // This prints out a header for a "fp" file ini the format that the
    // s::can/ana::xxx software is expecting
    void printFingerprintHeader(Stream *stream, const char *dlm="    ", spectralSource source=fingerprint);
    void printFingerprintHeader(Stream &stream, const char *dlm="    ", spectralSource source=fingerprint);



//----------------------------------------------------------------------------
//              FUNCTIONS TO GET AND CHANGE DEVICE CONFIGURATIONS
//----------------------------------------------------------------------------
// I cannot promise that your device will actually accept any changes from
// these set commands.  It is better to use s::can's software to make any
// changes to the logger configurations.

    // Functions for the communication mode
    int getCommunicationMode(void);
    bool setCommunicationMode(specCommMode mode);
    String parseCommunicationMode(uint16_t code);

    // Functions for the serial baud rate
    // (iff communication mode = modbus RTU or modbus ASCII)
    int getBaudRate(void);
    bool setBaudRate(specBaudRate baud);
    uint16_t parseBaudRate(uint16_t code);

    // Functions for the serial parity
    // (iff communication mode = modbus RTU or modbus ASCII)
    int getParity(void);
    bool setParity(specParity parity);
    String parseParity(uint16_t code);

    // Functions for the pointer to the private configuration register
    int getprivateConfigRegister(void);
    int getprivateConfigRegisterType(void);
    String parseRegisterType(uint16_t code);

    // This reads the global calibration name from the private registers
    // NB This is NOT documented
    String getCurrentGlobalCal(void);

    // Functions for the "s::canpoint" (ie, current installation site) of the device
    String getScanPoint(void);
    bool setScanPoint(char charScanPoint[12]);

    // Functions for the cleaning mode configuration
    // My spec does NOT respond properly to this command.
    int getCleaningMode(void);
    bool setCleaningMode(cleaningMode mode);
    String parseCleaningMode(uint16_t code);

    // Functions for the cleaning interval (ie, number of samples between cleanings)
    // My spec does NOT respond properly to this command.
    int getCleaningInterval(void);
    bool setCleaningInterval(uint16_t intervalSamples);

    // Functions for the cleaning duration in seconds
    int getCleaningDuration(void);
    bool setCleaningDuration(uint16_t secDuration);

    // Functions for the waiting time between end of cleaning
    // and the start of a measurement
    int getCleaningWait(void);
    bool setCleaningWait(uint16_t secDuration);

    // Functions for the current system time in seconds from Jan 1, 1970
    uint32_t getSystemTime(void);
    bool setSystemTime(uint32_t currentUnixTime);

    // Functions for the measurement interval in seconds
    // (0 - as fast as possible)
    int getMeasInterval(void);
    bool setMeasInterval(uint16_t secBetween);

    // Functions for the logging Mode (0 = on; 1 = off)
    int getLoggingMode(void);
    bool setLoggingMode(uint8_t mode);
    String parseLoggingMode(uint16_t code);

    // Functions for the ogging interval for data logger in minutes
    // (0 = no logging active)
    // My spec is not responding to the set command at this time
    int getLoggingInterval(void);
    bool setLoggingInterval(uint16_t interval);

    // Available number of logged results in datalogger since last clearing
    int getNumLoggedResults(void);

    // "Index device status public + private & parameter results from logger
    // storage to Modbus registers.  If no stored results are available,
    // results are NaN, Device status bit3 is set."
    // I'm really not sure what this means...
    int getIndexLogResult(void);



//----------------------------------------------------------------------------
//           FUNCTIONS TO GET AND CHANGE PARAMETER CONFIGURATIONS
//----------------------------------------------------------------------------
// I cannot promise that your device will actually accept any changes from
// these set commands.  It is better to use s::can's software to make any
// changes to the logger configurations.

    // This returns a pretty string with the parameter measured.
    String getParameterName(int parmNumber);

    // This returns a pretty string with the measurement units.
    String getParameterUnits(int parmNumber);

    // This gets the upper limit of the parameter
    // The float variable must be initialized prior to calling this function.
    float getParameterUpperLimit(int parmNumber);

    // This gets the lower limit of the parameter
    // The float variable must be initialized prior to calling this function.
    float getParameterLowerLimit(int parmNumber);

    // This gets the offset of the local calibration
    float getParameterCalibOffset(int parmNumber);

    // This gets the slope of the local calibration
    float getParameterCalibSlope(int parmNumber);

    // This gets the x2 coefficient of the slope of the local calibration
    float getParameterCalibX2(int parmNumber);

    // This gets the x3 coefficient of the slope of the local calibration
    float getParameterCalibX3(int parmNumber);



//----------------------------------------------------------------------------
//           FUNCTIONS TO GET AND CHANGE REFERENCE CONFIGURATIONS
//----------------------------------------------------------------------------
// NB - NONE of this is documented in s::can manuals

    // This returns a pretty string with the name of the reference currently in use
    String getCurrentReferenceName(void);

    // This returns the index number of the reference in use.
    int16_t getCurrentReferenceNumber(void);

    // This returns the index number of the reference in use.
    uint32_t getCurrentReferenceTime(void);

    // This returns a pretty string with the Reference measured.
    String getReferenceName(int refNumber);

    // This returns the amount of "dark noise" when the reference was taken
    float getReferenceDarkNoise(int refNumber);

    // This returns the average "K" value when the reference was taken
    int16_t getReferenceAvgK(int refNumber);

    // This returns the average "M" value when the reference was taken
    int16_t getReferenceAvgM(int refNumber);

    // This returns the flash rate in Hz when the reference was taken
    int16_t getReferenceFlashRate(int refNumber);

    // This returns the detector type used to take the reference
    //  0 = UV, 1 = UV-Vis
    detectorType getReferenceDetectorType(int refNumber);

    // This returns the "Number of max repetitions" when the reference was taken
    // I have no clue what that means, but that's what this value is
    int16_t getReferenceRepetitions(int refNumber);

    // This returns true if the Lp filter was on when the reference was taken, else false
    bool getReferenceLpFilter(int refNumber);

    // This returns the frequency lower limit in Hertz
    int16_t getReferenceFUG(int refNumber);

    // This returns the reference offset in abs/m
    int16_t getReferenceOffset(int refNumber);

    // This returns the Unix timestamp when the reference was recorded
    uint32_t getReferenceTime(int refNumber);

    // This gets abssorbance values in Abs/m for the reference and puts them
    // into a previously initialized float array.  The array must have space
    // for 256 values!
    bool getReferenceValues(float fpArray[], spectralSource source=fingerprint);

    // This prints the reference data as delimeter separated data.
    // By default, the delimeter is a TAB (\t, 0x09).
    // NB:  You can use this to print to a file on a SD card!
    void printReferenceData(int refNumber, Stream *stream, const char *dlm="    ");
    void printReferenceData(int refNumber, Stream &stream, const char *dlm="    ");



//----------------------------------------------------------------------------
//          FUNCTIONS TO GET SETUP INFORMATION FROM THE INPUT REGISTERS
//----------------------------------------------------------------------------
// This information can be read, but cannot be changed

    // Get the version of the modbus mapping protocol
    float getModbusVersion(void);

    // This returns a byte with the model type
    uint16_t getModelType(void);

    // This returns a pretty string with the model information
    String getModel(void);

    // This gets the instrument serial number as a String
    String getSerialNumber(void);

    // This gets the hardware version of the sensor
    float getHWVersion(void);

    // This gets the software version of the sensor
    float getSWVersion(void);

    // This gets the number of times the spec has been rebooted
    // (Device rebooter counter)
    int getHWStarts(void);

    // This gets the number of parameters the spectro::lyzer is set to measure
    int getParameterCount(void);

    // This gets the datatype of the parameters and parameter limits
    // This is a check for compatibility
    int getParameterType(void);
    String parseParameterType(uint16_t code);

    // This gets the scaling factor for all parameters which depend on eParameterType
    int getParameterScale(void);

    // This returns the spectral path length in mm
    // NB This is not documented - I'm guessing based on register VALUES
    float getPathLength(void);



//----------------------------------------------------------------------------
//                       PURELY DEBUGGING FUNCTIONS
//----------------------------------------------------------------------------

    // This sets a stream for debugging information to go to;
    void setDebugStream(Stream *stream){modbus.setDebugStream(stream);}

    // This sets a stream for debugging information to go to;
    void stopDebugging(void){modbus.stopDebugging();}



//----------------------------------------------------------------------------
//                            PRIVATE FUNCTIONS
//----------------------------------------------------------------------------

    modbusMaster modbus;
    byte _slaveID;
};

#endif
