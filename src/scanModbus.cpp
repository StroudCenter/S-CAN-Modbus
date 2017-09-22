/*
 *scanModbus.cpp
*/

#include "scanModbus.h"

//----------------------------------------------------------------------------
//                          GENERAL USE FUNCTIONS
//----------------------------------------------------------------------------


// This function sets up the communication
// It should be run during the arduino "setup" function.
// The "stream" device must be initialized and begun prior to running this.
bool scan::begin(byte modbusSlaveID, Stream *stream, int enablePin)
{
    _slaveID = modbusSlaveID;
    return modbus.begin(modbusSlaveID, stream, enablePin);
}
bool scan::begin(byte modbusSlaveID, Stream &stream, int enablePin)
{return begin(modbusSlaveID, &stream, enablePin);}


// This prints out all of the setup information to the selected stream
bool scan::printSetup(Stream *stream)
{
    // Wake up the spec if it was sleeping
    stream->println("------------------------------------------");
    wakeSpec();

    // When returning a bunch of registers, as here, to get
    // the byte location in the frame of the desired register use:
    // (3 bytes of Modbus header + (2 bytes/register x (desired register - start register))

    // Get the holding registers
    stream->println("------------------------------------------");
    if (modbus.getRegisters(0x03, 0, 27))
    {
        // Setup information from holding registers
        stream->print("Communication mode setting is: ");
        stream->print(modbus.int16FromFrame(bigEndian, 5));
        stream->print(" (");
        stream->print(parseCommunicationMode(modbus.int16FromFrame(bigEndian, 5)));
        stream->println(")");

        stream->print("Baud Rate setting is: ");
        stream->print(modbus.int16FromFrame(bigEndian, 7));
        stream->print(" (");
        stream->print(parseBaudRate(modbus.int16FromFrame(bigEndian, 7)));
        stream->println(")");

        stream->print("Parity setting is: ");
        stream->print(modbus.int16FromFrame(bigEndian, 9));
        stream->print(" (");
        stream->print(parseParity(modbus.int16FromFrame(bigEndian, 9)));
        stream->println(")");

        stream->print("Private configuration begins sometime after register ");
        stream->print(modbus.pointerFromFrame(bigEndian, 13));
        stream->print(", which is type ");
        stream->print(modbus.pointerTypeFromFrame(bigEndian, 13));
        stream->print(" (");
        stream->print(parseRegisterType(modbus.pointerTypeFromFrame(bigEndian, 13)));
        stream->println(")");

        stream->print("Current s::canpoint is: ");
        stream->println(modbus.StringFromFrame(12, 15));

        stream->print("Cleaning mode setting is: ");
        stream->print(modbus.int16FromFrame(bigEndian, 27));
        stream->print(" (");
        stream->print(parseCleaningMode(modbus.int16FromFrame(bigEndian, 27)));
        stream->println(")");

        stream->print("Cleaning interval is: ");
        stream->print(modbus.int16FromFrame(bigEndian, 29));
        stream->println(" measurements between cleanings");

        stream->print("Cleaning time is: ");
        stream->print(modbus.int16FromFrame(bigEndian, 31));
        stream->println(" seconds");

        stream->print("Wait time between cleaning and sampling is: ");
        stream->print(modbus.int16FromFrame(bigEndian, 33));
        stream->println(" seconds");

        stream->print("Current System Time is: ");
        stream->print((uint32_t)(modbus.TAI64FromFrame(35)));
        stream->println(" seconds past Jan 1, 1970");

        stream->print("Measurement interval is: ");
        stream->print(modbus.int16FromFrame(bigEndian, 47));
        stream->println(" seconds");

        stream->print("Logging mode setting is: ");
        stream->print(modbus.int16FromFrame(bigEndian, 49));
        stream->print(" (");
        stream->print(parseLoggingMode(modbus.int16FromFrame(bigEndian, 49)));
        stream->println(")");

        stream->print("Logging interval is: ");
        stream->print(modbus.int16FromFrame(bigEndian, 51));
        stream->println(" seconds");

        stream->print(modbus.int16FromFrame(bigEndian, 53));
        stream->println(" results have been logged so far");

        stream->print("Index device status is: ");
        stream->println(modbus.int16FromFrame(bigEndian, 53));
    }
    else return false;

    // Get some of the register input that it's a pain to pull up separtely
    stream->println("------------------------------------------");

    stream->print("Modbus Version is: ");
    stream->println(getModbusVersion());
    stream->print("Hardware Version is: ");
    stream->println(getHWVersion());
    stream->print("Software Version is: ");
    stream->println(getSWVersion());

    // Get rest of the input registers
    int parmCount;
    if (modbus.getRegisters(0x04, 0, 25))
    {
        // Setup information from input registers

        stream->print("Instrument model is: ");
        stream->println(modbus.StringFromFrame(20, 9));

        stream->print("Instrument Serial Number is: ");
        stream->println(modbus.StringFromFrame(8, 29));

        stream->print("Hardware has been restarted: ");
        stream->print(modbus.int16FromFrame(bigEndian, 45));
        stream->println(" times");

        stream->print("There are ");
        parmCount = modbus.int16FromFrame(bigEndian, 47);
        stream->print(parmCount);
        stream->println(" parameters being measured");

        stream->print("The data type of the parameters is: ");
        stream->print(modbus.int16FromFrame(bigEndian, 49));
        stream->print(" (");
        stream->print(parseParameterType(modbus.int16FromFrame(bigEndian, 49)));
        stream->println(")");

        stream->print("The parameter scale factor is: ");
        stream->println(modbus.int16FromFrame(bigEndian, 51));
    }
    else return false;

    // Get the parameter info
    for (int i = 1; i < parmCount+1; i++)
    {
        stream->println("------------------------------------------");
        stream->print("Parameter Number ");
        stream->print(i);
        stream->print(" is ");
        stream->print(getParameterName(i));
        stream->print(" and has units of ");
        stream->print(getParameterUnits(i));
        stream->print(". The lower limit is ");
        stream->print(getParameterLowerLimit(i));
        stream->print(" and the upper limit is ");
        stream->print(getParameterUpperLimit(i));
        stream->println(".");
    }

    // if all passed, return true
    stream->println("------------------------------------------");
    return true;
}
bool scan::printSetup(Stream &stream) {return printSetup(&stream);}


// Reset all settings to default
bool scan::resetSettings(void)
{
    return modbus.uint16ToRegister(4, 1, bigEndian);
}


// This sets a new modbus slave ID
bool scan::setSlaveID(byte newSlaveID)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = newSlaveID;
    return modbus.setRegisters(0, 1, byteToSend);
}


// This returns the current device status as a bitmap
int scan::getDeviceStatus(void)
{return modbus.uint16FromRegister(0x04, 120);}
// This parses the device status bitmask and prints out from the codes
void scan::printDeviceStatus(uint16_t bitmask, Stream *stream)
{
    // b15
    if ((bitmask & 32768) == 32768)
        stream->println("Device maintenance required");
    // b14
    if ((bitmask & 16384) == 16384)
        stream->println("Device cleaning required");
    // b13
    if ((bitmask & 8192) == 8192)
        stream->println("Device busy");
    // b3
    if ((bitmask & 8) == 8)
        stream->println("Data logger error, no readings can be stored because datalogger is full");
    // b2
    if ((bitmask & 4) == 4)
        stream->println("Missing or devective component detected");
    // b1
    if ((bitmask & 2) == 2)
        stream->println("Probe misuse, operation outside the specified temperature range");
    // b0
    if ((bitmask & 1) == 1)
        stream->println("s::can device reports error during internal check");
    // No Codes
    if (bitmask == 0)
        stream->println("Device is operating normally");
}
void scan::printDeviceStatus(uint16_t bitmask, Stream &stream)
{printDeviceStatus(bitmask, &stream);}


void scan::printSystemStatus(uint16_t bitmask, Stream *stream)
{
    // b6
    if ((bitmask & 64) == 64)
        stream->println("mA signal is outside of the allowed input range");
    // b5
    if ((bitmask & 32) == 32)
        stream->println("Validation results are not available");
    // b1
    if ((bitmask & 2) == 2)
        stream->println("Invalid probe/sensor; serial number of probe/sensor is different");
    // b0
    if ((bitmask & 1) == 1)
        stream->println("No communication between probe/sensor and controller");
    // No Codes
    if (bitmask == 0)
        stream->println("System is operating normally");
}
void scan::printSystemStatus(uint16_t bitmask, Stream &stream)
{printSystemStatus(bitmask, &stream);}


// This sends three requests for a single register
// If the spectro::lyzer is sleeping, it will not respond until the third one
// Alternately, we could send " Weckzeichen" (German for "Ringtone") three
// times before each command, which is what ana::lyte and ana::pro do.
// " Weckzeichen" = 0x20, 0x57, 0x65, 0x63, 0x6b, 0x7a, 0x65, 0x69, 0x63, 0x68, 0x65, 0x6e
bool scan::wakeSpec(void)
{
    // _debugStream->println("------>Checking if spectro::lyzer is awake.<------");
    if (modbus.uint16FromRegister(0x03, 0) > 0)
    {
        // _debugStream->println("------>Spectro::lyser is now awake.<------");
        return true;
    }
    else
    {
        // _debugStream->println("------>No response from spectro::lyzer!<------");
        return false;
    }
}




//----------------------------------------------------------------------------
//           FUNCTIONS TO RETURN THE ACTUAL SAMPLE TIMES AND VALUES
//----------------------------------------------------------------------------

// Last measurement time as a 32-bit count of seconds from Jan 1, 1970
// System time is in input registers 104-109
// (96-bit timestamp in TAI64N format - in this case, ignoring the nanoseconds)
uint32_t scan::getParameterTime(void)
{return modbus.TAI64FromRegister(0x04, 104);}
// This gets any general errors regarding the measured parameters (parameter status public)
uint16_t scan::getParameterStatus(int parmNumber)
{
    int startingReg = 120 + 8*parmNumber;
    // Get the register data
    return modbus.uint16FromRegister(0x04, startingReg, bigEndian);
}
void scan::printParameterStatus(uint16_t bitmask, Stream *stream)
{
    // b15 - 8xxx
    if ((bitmask & 32768) == 32768)
        stream->println("Parameter reading out of measuring range");
    // b14 - 4xxx - ALARM10
    if ((bitmask & 16384) == 16384)
        stream->println("Status of alarm Parameter is 'WARNING'");
    // b13 - 2xxx - ALARM01
    if ((bitmask & 8192) == 8192)
        stream->println("Status of alarm Parameter is 'ALARM'");
    // b12 - 1xx
    if ((bitmask & 4096) == 4096)
        stream->println("Data is marked as non fully trustworthy by data validation algorithm");
    // b11 - x8xx
    if ((bitmask & 2048) == 2048)
        stream->println("Maintenance necessary");
    // b5 - xx2x
    if ((bitmask & 32) == 32)
        stream->println("Parameter not ready or not available");
    // b4 - xx1x
    if ((bitmask & 16) == 16)
        stream->println("Incorrect calibration, at least one calibration coefficient invalid");
    // b3 - xxx8
    if ((bitmask & 8) == 8)
        stream->println("Parameter error, the sensor is outside of the medium or in incorrect medium");
    // b2 - xxx4
    if ((bitmask & 4) == 4)
        stream->println("Parameter error, calibration error");
    // b1 - xxx2
    if ((bitmask & 2) == 2)
        stream->println("Parameter error, hardware error");
    // b0 - xxx1
    if ((bitmask & 1) == 1)
        stream->println("General parameter error, at least one internal parameter check failed");
    // No Codes
    if (bitmask == 0)
        stream->println("Parameter is operating normally");
}
void scan::printParameterStatus(uint16_t bitmask, Stream &stream)
{printParameterStatus(bitmask, &stream);}
// This gets any specific errors for the spectrometer itself (sensor status private)
uint16_t scan::getSpecStatus(int parmNumber)
{
    int startingReg = 121 + 8*parmNumber;
    // Get the register data
    return modbus.uint16FromRegister(0x04, startingReg, bigEndian);
}
void scan::printSpecStatus(uint16_t bitmask, Stream *stream)
{
    // b15 - 8xxx
    if ((bitmask & 32768) == 32768)
        stream->println("Probe compensation failure - above upper limit");
    // b14 - 4xxx
    if ((bitmask & 16384) == 16384)
        stream->println("Probe compensation failure - below lower limit");
    // b13 - 2xxx
    if ((bitmask & 8192) == 8192)
        stream->println("Probe compensation failure - overflow");
    // b12 - 1xx
    if ((bitmask & 4096) == 4096)
        stream->println("Probe energy failure - overflow");
    // b11 - x8xx
    if ((bitmask & 2048) == 2048)
        stream->println("Probe compensation failure - standard deviation too high");
    // b10 - x4xx
    if ((bitmask & 1024) == 1024)
        stream->println("Probe energy failure - dark noise too high");
    // b9 - x2xx
    if ((bitmask & 512) == 512)
        stream->println("PROBE MISUSE - Medium temperature too high");
    // b8 - x1xx
    if ((bitmask & 256) == 256)
        stream->println("PROBE MISUSE - Medium temperature too low");
    // b7 - xx8x
    if ((bitmask & 128) == 128)
        stream->println("Internal humidity is too high");
    // b6 - xx4x
    if ((bitmask & 64) == 64)
        stream->println("Internal humidity is too low");
    // b5 - xx2x
    if ((bitmask & 32) == 32)
        stream->println("PROBE MISUSE - Voltage too high");
    // b4 - xx1x
    if ((bitmask & 16) == 16)
        stream->println("PROBE MISUSE - Voltage too low");
    // b3 - xxx8
    if ((bitmask & 8) == 8)
        stream->println("PROBE MISUSE - Medium pressure too high");
    // b2 - xxx4
    if ((bitmask & 4) == 4)
        stream->println("PROBE MISUSE - Medium pressure too low");
    // b1 - xxx2
    if ((bitmask & 2) == 2)
        stream->println("Data logger is full.  Measurements on the probe are stopped");
    // b0 - xxx1
    if ((bitmask & 1) == 1)
        stream->println("Actual used reference measurement is invalid");
    // No Codes
    if (bitmask == 0)
        stream->println("Spectro::lyzer is operating normally");
}
void scan::printSpecStatus(uint16_t bitmask, Stream &stream)
{printSpecStatus(bitmask, &stream);}
// This gets calibrated data value
float scan::getParameterValue(int parmNumber)
{
    int startingReg = 120 + 8*parmNumber + 2;
    // Get the register data
    return modbus.float32FromRegister(0x04, startingReg, bigEndian);
}


// Last measurement time as a 32-bit count of seconds from Jan 1, 1970
// (96-bit timestamp in TAI64N format - in this case, ignoring the nanoseconds)
uint32_t scan::getFingerprintTime(spectralSource source)
{
    int startingReg = 512 + 512*source;
    return modbus.TAI64FromRegister(0x04, startingReg);
}
// This returns detector type used for the fingerprint
detectorType scan::getFingerprintDetectorType(spectralSource source)
{
    int startingReg = 518 + 512*source;
    int detect;
    detect = modbus.uint16FromRegister(0x04, startingReg, bigEndian);
    return (detectorType)detect;
}
// This returns the spectral source type used for the fingerprint
spectralSource scan::getFingerprintSource(spectralSource source)
{
    int startingReg = 519 + 512*source;
    int src;
    src = modbus.uint16FromRegister(0x04, startingReg, bigEndian);
    return (spectralSource)src;
}
// This returns the spectral source type used for the fingerprint
int scan::getFingerprintPathLength(spectralSource source)
{
    int startingReg = 520 + 512*source;
    return modbus.uint16FromRegister(0x04, startingReg, bigEndian);
}
// This returns the parameter status for the fingerprint
uint16_t scan::getFingerprintStatus(spectralSource source)
{
    // A total and complete WAG as to the location of the status (521)
    // My other guess is that the status is in 508
    int startingReg = 521 + 512*source;
    return modbus.uint16FromRegister(0x04, startingReg, bigEndian);
}
// This gets spectral values from the sensor and puts them into a previously
// initialized float array.  The array must have space for 221 values!
// The actual return from the function is an integer which is a bit-mask
// describing the fingerprint status (or, well, it would be if I could figure
// out which register that value lived in).
// int scan::getFingerprintData(float fpArray[], spectralSource source)
// {
//     int startingReg = 522 + 512*source;
//
//     // Get the register data in several batches
//     int valuesPerCall = 30;
//     int totalValues = 221;
//     for (int j = 0; j < totalValues;)
//     {
//         int valuesRemaining = totalValues - j;
//         int regToCall;
//         if (valuesRemaining < valuesPerCall) regToCall = valuesRemaining*2;
//         else regToCall = valuesPerCall*2;
//         modbus.getRegisters(0x04, startingReg + j*2, regToCall);
//         for (int i = 0; i < valuesPerCall; i++)
//         {
//             float pointVal = modbus.float32FromFrame(bigEndian, (i*4 + 3));
//             fpArray[i+j] = pointVal;
//             j++;
//         }
//     }
//
//     // A total and complete WAG as to the location of the status (521)
//     // My other guess is that the status is in 508 (startingReg-14)
//     Serial.println("Status");
//     return modbus.uint16FromRegister(0x04, startingReg-1, bigEndian);
// }
// This prints the fingerprint data as delimeter separated data.
// By default, the delimeter is a TAB (\t, 0x09), as expected by the s::can/ana::xxx software.
// This includes the fingerprint timestamp and status
// NB:  You can use this to print to a file on a SD card!
void scan::printFingerprintData(Stream *stream, const char *dlm, spectralSource source)
{
        int startingReg = 522 + 512*source;

        // Get the register data in several batches
        int valuesPerCall = 30;
        int totalValues = 221;
        for (int j = 0; j < totalValues;)
        {
            int valuesRemaining = totalValues - j;
            int regToCall;
            if (valuesRemaining < valuesPerCall) regToCall = valuesRemaining*2;
            else regToCall = valuesPerCall*2;
            modbus.getRegisters(0x04, startingReg + j*2, regToCall);
            for (int i = 0; i < valuesPerCall; i++)
            {
                float pointVal = modbus.float32FromFrame(bigEndian, (i*4 + 3));
                if (i+j < totalValues+1) stream->print(pointVal, 4);
                if (i+j < totalValues) stream->print(dlm);
                j++;
            }
        }
        stream->println();
}
void scan::printFingerprintData(Stream &stream, const char *dlm, spectralSource source)
{printFingerprintData(&stream, dlm, source);}


//----------------------------------------------------------------------------
//              FUNCTIONS TO GET AND CHANGE DEVICE CONFIGURATIONS
//----------------------------------------------------------------------------

// Functions for the communication mode
// The Communication mode is in holding register 1 (1 uint16 register)
int scan::getCommunicationMode(void)
{return modbus.uint16FromRegister(0x03, 1);}
bool scan::setCommunicationMode(specCommMode mode)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = mode;
    return modbus.setRegisters(1, 1, byteToSend);
}
String scan::parseCommunicationMode(uint16_t code)
{
    switch (code)
    {
        case 0: return "Modbus RTU";
        case 1: return "Modbus ASCII";
        case 2: return "Modbus TCP";
        default: return "Unknown";
    }
}


// Functions for the serial baud rate (iff communication mode = modbus RTU or modbus ASCII)
// Baud rate is in holding register 2 (1 uint16 register)
int scan::getBaudRate(void)
{return modbus.uint16FromRegister(0x03, 2);}
bool scan::setBaudRate(specBaudRate baud)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = baud;
    return modbus.setRegisters(2, 1, byteToSend);
}
uint16_t scan::parseBaudRate(uint16_t code)
{
    String baud;
    switch (code)
    {
        case 0: baud = "9600"; break;
        case 1: baud = "19200"; break;
        case 2: baud = "38400"; break;
        default: baud = "0"; break;
    }
    uint16_t baudr = baud.toInt();
    return baudr;
}


// Functions for the serial parity (iff communication mode = modbus RTU or modbus ASCII)
// Parity is in holding register 3 (1 uint16 register)
int scan::getParity(void)
{return modbus.uint16FromRegister(0x03, 3);}
bool scan::setParity(specParity parity)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = parity;
    return modbus.setRegisters(3, 1, byteToSend);
}
String scan::parseParity(uint16_t code)
{
    switch (code)
    {
        case 0: return "no parity";
        case 1: return "even parity";
        case 2: return "odd parity";
        default: return "Unknown";
    }
}

// Functions to get a pointer to the private configuration register
// Pointer to the private configuration is in holding register 5
// This is read only
int scan::getprivateConfigRegister(void)
{return modbus.pointerFromRegister(0x03, 5);}
int scan::getprivateConfigRegisterType(void)
{return modbus.pointerTypeFromRegister(0x03, 5);}
String scan::parseRegisterType(uint16_t code)
{
    switch (code)
    {
        case 0: return "Holding register";  // 0b00 - read by command 0x03, written by 0x06 or 0x10
        case 1: return "Input register";  // 0b01 - read by command 0x04
        case 2: return "Discrete input register";  // 0b10 - read by command 0x02
        case 3: return "Coil";  // 0b10) - read by command 0x01, written by 0x05
        default: return "Unknown";
    }
}

// This reads the global calibration name from the private registers
// NB This is NOT documented
// NB In theory, can get the location from the pointer to private setup which
// is in the holding registers.  In reality, it seems alwasy to start at 1180
// for the spectrolyzer and to be in for ana::gate, so I'll just call there
// and save data transfer time.
String scan::getCurrentGlobalCal(void)
{
    if (getModelType() == 0x0101) return modbus.StringFromRegister(0x03, 1180, 12);
    else return modbus.StringFromRegister(0x03, 964, 12);
    /*
    byte regType;
    switch (getprivateConfigRegisterType())
    {
        case 0: regType = 0x03; break;
        case 1: regType = 0x04; break;
        case 2: regType = 0x02; break;
        case 3: regType = 0x05; break;
        default: regType = 0x00; break;
    }
    // Because the address of the private config could be shifted, we have to
    // search for the actual start of the global calibration name
    int testVal;
    int actualReg;
    for (int i = 0; i < 256; i++)
    {
        actualReg = regNum + i;
        testVal = modbus.uint16FromRegister(regType, actualReg);
        if (testVal != 0) break;
    }
    return modbus.StringFromRegister(regType, actualReg, 12);
    */
}


// Functions for the "s::canpoint" of the device
// Device Location (s::canpoint) is registers 6-11 (char[12])
// This is read only
String scan::getScanPoint(void)
{return modbus.StringFromRegister(0x03, 6, 12);}
bool scan::setScanPoint(char charScanPoint[12])
{return modbus.charToRegister(6, charScanPoint, 12);}


// Functions for the cleaning mode configuration
// Cleaning mode is in holding register 12 (1 uint16 register)
int scan::getCleaningMode(void)
{return modbus.uint16FromRegister(0x03, 12);}
bool scan::setCleaningMode(cleaningMode mode)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = mode;
    return modbus.setRegisters(12, 1, byteToSend);
}
String scan::parseCleaningMode(uint16_t code)
{
    switch (code)
    {
        case 0: return "no cleaning supported";
        case 1: return "manual";
        case 2: return "automatic";
        default: return "Unknown";
    }
}


// Functions for the cleaning interval (ie, number of samples between cleanings)
// Cleaning interval is in holding register 13 (1 uint16 register)
int scan::getCleaningInterval(void)
{return modbus.uint16FromRegister(0x03, 13);}
bool scan::setCleaningInterval(uint16_t intervalSamples)
{return modbus.uint16ToRegister(13, intervalSamples, bigEndian);}

// Functions for the cleaning duration in seconds
// Cleaning duration is in holding register 14 (1 uint16 register)
int scan::getCleaningDuration(void)
{return modbus.uint16FromRegister(0x03, 14);}
bool scan::setCleaningDuration(uint16_t secDuration)
{return modbus.uint16ToRegister(14, secDuration, bigEndian);}

// Functions for the waiting time between end of cleaning
// and the start of a measurement
// Cleaning wait time is in holding register 15 (1 uint16 register)
int scan::getCleaningWait(void)
{return modbus.uint16FromRegister(0x03, 15);}
bool scan::setCleaningWait(uint16_t secDuration)
{return modbus.uint16ToRegister(15, secDuration, bigEndian);}

// Functions for the current system time in seconds from Jan 1, 1970
// System time is in holding registers 16-21
// (64-bit timestamp in  in TAI64N format - in this case, ignoring the nanoseconds)
uint32_t scan::getSystemTime(void)
{return modbus.TAI64FromRegister(0x03, 16);}
bool scan::setSystemTime(uint32_t currentUnixTime)
{return modbus.TAI64ToRegister(16, currentUnixTime);}

// Functions for the measurement interval in seconds (0 - as fast as possible)
// Measurement interval is in holding register 22 (1 uint16 register)
int scan::getMeasInterval(void)
{return modbus.uint16FromRegister(0x03, 22);}
bool scan::setMeasInterval(uint16_t secBetween)
{return modbus.uint16ToRegister(22, secBetween, bigEndian);}

// Functions for the logging Mode (0 = on; 1 = off)
// Logging Mode (0 = on; 1 = off) is in holding register 23 (1 uint16 register)
int scan::getLoggingMode(void)
{return modbus.uint16FromRegister(0x03, 23);}
bool scan::setLoggingMode(uint8_t mode)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = mode;
    return modbus.setRegisters(23, 1, byteToSend);
}
String scan::parseLoggingMode(uint16_t code)
{
    switch (code)
    {
        case 0: return "Logging On";
        default: return "Logging Off";
    }
}


// Functions for the logging interval for data logger in minutes
// (0 = no logging active)
// Logging interval is in holding register 24 (1 uint16 register)
int scan::getLoggingInterval(void)
{return modbus.uint16FromRegister(0x03, 24);}
bool scan::setLoggingInterval(uint16_t interval)
{return modbus.uint16ToRegister(24, interval, bigEndian);}

// Available number of logged results in datalogger since last clearing
// Available number of logged results is in holding register 25 (1 uint16 register)
int scan::getNumLoggedResults(void)
{return modbus.uint16FromRegister(0x03, 25);}

// "Index device status public + private & parameter results from logger
// storage to Modbus registers.  If no stored results are available,
// results are NaN, Device status bit3 is set."
// I'm really not sure what this means...
// "Index device status" is in holding register 26 (1 uint16 register)
int scan::getIndexLogResult(void)
{return modbus.uint16FromRegister(0x03, 26);}



//----------------------------------------------------------------------------
//           FUNCTIONS TO GET AND CHANGE PARAMETER CONFIGURATIONS
//----------------------------------------------------------------------------

// This returns a string with the parameter measured.
// The information on the first parameter is in register 120
// The next parameter begins 120 registers after that.
// The spectro::lyzer supports up to 8 parameters, ana::gate supports 32.
String scan::getParameterName(int parmNumber)
{
    int startingReg = 120*parmNumber;
    return modbus.StringFromRegister(0x03, startingReg, 8);
}

// This returns a string with the measurement units.
// This begins 4 registers after the parameter name
String scan::getParameterUnits(int parmNumber)
{
    int startingReg = 120*parmNumber + 4;
    return modbus.StringFromRegister(0x03, startingReg, 8);
}

// This gets the upper limit of the parameter
// This begins 8 registers after the parameter name
float scan::getParameterUpperLimit(int parmNumber)
{
    int startingReg = 120*parmNumber + 8;
    return modbus.float32FromRegister(0x03, startingReg, bigEndian);
}

// This gets the lower limit of the parameter
// This begins 10 registers after the parameter name
float scan::getParameterLowerLimit(int parmNumber)
{
    int startingReg = 120*parmNumber + 10;
    return modbus.float32FromRegister(0x03, startingReg, bigEndian);
}

// This gets the offset of the local calibration
float scan::getParameterCalibOffset(int parmNumber)
{
    int startingReg = 120*parmNumber + 14;
    return modbus.float32FromRegister(0x03, startingReg, bigEndian);
}

// This gets the slope of the local calibration
float scan::getParameterCalibSlope(int parmNumber)
{
    int startingReg = 120*parmNumber + 16;
    return modbus.float32FromRegister(0x03, startingReg, bigEndian);
}

// This gets the x2 coefficient of the slope of the local calibration
float scan::getParameterCalibX2(int parmNumber)
{
    int startingReg = 120*parmNumber + 18;
    return modbus.float32FromRegister(0x03, startingReg, bigEndian);
}

// This gets the x3 coefficient of the slope of the local calibration
float scan::getParameterCalibX3(int parmNumber)
{
    int startingReg = 120*parmNumber + 20;
    return modbus.float32FromRegister(0x03, startingReg, bigEndian);
}

// This gets the measurement precision of the parameter
// Totally a wag as to the location
uint16_t scan::getParameterPrecision(int parmNumber)
{
    int startingReg = 120*parmNumber + 26;
    return modbus.uint16FromRegister(0x03, startingReg, bigEndian);
}




//----------------------------------------------------------------------------
//           FUNCTIONS TO GET AND CHANGE REFERENCE CONFIGURATIONS
//----------------------------------------------------------------------------
// NB - NONE of this is documented in s::can manuals

// This returns the index number of the reference in use.
int16_t scan::getCurrentReferenceNumber(void)
{return modbus.int16FromRegister(0x03, 1507, bigEndian);}

// This returns a pretty string with the name of the reference currently in use
String scan::getCurrentReferenceName(void)
{return modbus.StringFromRegister(0x03, 1508, 8);}

// This returns the index number of the reference in use.
uint32_t scan::getCurrentReferenceTime(void)
{return modbus.TAI64FromRegister(0x03, 1512);}

// This returns a pretty string with the Reference measured.
String scan::getReferenceName(int refNumber)
{
    int startingReg = 1519 + 536*refNumber;
    return modbus.StringFromRegister(0x03, startingReg, 8);
}

// This returns the amount of "dark noise" when the reference was taken
float scan::getReferenceDarkNoise(int refNumber)
{
    int startingReg = 1523 + 536*refNumber;
    return modbus.float32FromRegister(0x03, startingReg, bigEndian);
}

// This returns the average "K" value when the reference was taken
int16_t scan::getReferenceAvgK(int refNumber)
{
    int startingReg = 1525 + 536*refNumber;
    return modbus.int16FromRegister(0x03, startingReg, bigEndian);
}

// This returns the average "M" value when the reference was taken
int16_t scan::getReferenceAvgM(int refNumber)
{
    int startingReg = 1526 + 536*refNumber;
    return modbus.int16FromRegister(0x03, startingReg, bigEndian);
}

// This returns the flash rate in Hz when the reference was taken
int16_t scan::getReferenceFlashRate(int refNumber)
{
    int startingReg = 1527 + 536*refNumber;
    return modbus.int16FromRegister(0x03, startingReg, bigEndian);
}

// This returns the lamp voltage during the reference measurement
int16_t scan::getReferenceLampVoltage(int refNumber)
{
    int startingReg = 1528 + 536*refNumber;
    return modbus.int16FromRegister(0x03, startingReg, bigEndian);
}

// This returns the detector type used to take the reference
//  0 = UV, 1 = UV-Vis
detectorType scan::getReferenceDetectorType(int refNumber)
{
    int startingReg = 1529 + 536*refNumber;
    int detect;
    detect = modbus.uint16FromRegister(0x03, startingReg, bigEndian);
    return (detectorType)detect;
}

// This returns the "Number of max repetitions" when the reference was taken
// I have no clue what that means, but that's what this value is
int16_t scan::getReferenceRepetitions(int refNumber)
{
    int startingReg = 1530 + 536*refNumber;
    return modbus.int16FromRegister(0x03, startingReg, bigEndian);
}

// This returns true if the Lp filter was on when the reference was taken, else false
bool scan::getReferenceLpFilter(int refNumber)
{
    int startingReg = 1531 + 536*refNumber;
    return modbus.int16FromRegister(0x03, startingReg, bigEndian);
}

// This returns the frequency lower limit in Hertz
int16_t scan::getReferenceFUG(int refNumber)
{
    int startingReg = 1532 + 536*refNumber;
    return modbus.int16FromRegister(0x03, startingReg, bigEndian);
}

// This returns the reference type (but I don't know what the return means)
int16_t scan::getReferenceType(int refNumber)
{
    int startingReg = 1533 + 536*refNumber;
    return modbus.int16FromRegister(0x03, startingReg, bigEndian);
}

// This returns the reference offset in abs/m
int16_t scan::getReferenceOffset(int refNumber)
{
    int startingReg = 1534 + 536*refNumber;
    return modbus.int16FromRegister(0x03, startingReg, bigEndian);
}

// This returns the Unix timestamp when the reference was recorded
uint32_t scan::getReferenceTime(int refNumber)
{
    int startingReg = 1536 + 536*refNumber;
    return modbus.TAI64FromRegister(0x03, startingReg);
}

// This gets abssorbance values in Abs/m for the reference and puts them
// into a previously initialized float array.  The array must have space
// for 256 values!
// void scan::getReferenceValues(float refArray[], int refNumber)
// {
//     int startingReg = 1542 + 536*refNumber;
//
//     // Get the register data in several batches
//     int valuesPerCall = 30;
//     int totalValues = 256;
//     for (int j = 0; j < totalValues;)
//     {
//         int valuesRemaining = totalValues - j;
//         int regToCall;
//         if (valuesRemaining < valuesPerCall) regToCall = valuesRemaining*2;
//         else regToCall = valuesPerCall*2;
//         modbus.getRegisters(0x04, startingReg + j*2, regToCall);
//         for (int i = 0; i < valuesPerCall; i++)
//         {
//             float pointVal = modbus.float32FromFrame(bigEndian, (i*4 + 3));
//             refArray[i+j] = pointVal;
//             j++;
//         }
//     }
// }

// This prints the reference data as delimeter separated data.
// By default, the delimeter is a TAB (\t, 0x09).
// NB:  You can use this to print to a file on a SD card!
void scan::printReferenceData(int refNumber, Stream *stream, const char *dlm)
{
    int startingReg = 1542 + 536*refNumber;

    // Get the register data in several batches
    int valuesPerCall = 30;
    int totalValues = 221;
    for (int j = 0; j < totalValues;)
    {
        int valuesRemaining = totalValues - j;
        int regToCall;
        if (valuesRemaining < valuesPerCall) regToCall = valuesRemaining*2;
        else regToCall = valuesPerCall*2;
        modbus.getRegisters(0x04, startingReg + j*2, regToCall);
        for (int i = 0; i < valuesPerCall; i++)
        {
            float pointVal = modbus.float32FromFrame(bigEndian, (i*4 + 3));
            if (i+j < totalValues+1) stream->print(pointVal, 4);
            if (i+j < totalValues) stream->print(dlm);
            j++;
        }
    }
    stream->println();
}
void scan::printReferenceData(int refNumber, Stream &stream, const char *dlm)
{printReferenceData(refNumber, &stream, dlm);}





//----------------------------------------------------------------------------
//          FUNCTIONS TO GET SETUP INFORMATION FROM THE INPUT REGISTERS
//----------------------------------------------------------------------------
// This information can be read, but cannot be changed

// Get the version of the modbus mapping protocol
// The modbus version is in input register 0
float scan::getModbusVersion(void)
{
    modbus.getRegisters(0x04, 0, 1);
    float mjv = modbus.byteFromFrame(3);
    float mnv = modbus.byteFromFrame(4);
    mnv = mnv/100;
    float version = mjv + mnv;
    return version;
}

// This returns a byte with the model type
uint16_t scan::getModelType(void)
{return modbus.uint16FromRegister(0x04, 2);}

// This returns a pretty string with the model information
String scan::getModel(void)
{return modbus.StringFromRegister(0x04, 3, 20);}

// This gets the instrument serial number as a String
String scan::getSerialNumber(void)
{return modbus.StringFromRegister(0x04, 13, 8);}

// This gets the hardware version of the sensor
float scan::getHWVersion(void)
{
    modbus.getRegisters(0x04, 17, 2);
    String _model = modbus.StringFromFrame(4);
    float mjv = _model.substring(0,2).toFloat();
    float mnv = (_model.substring(2,4).toFloat())/100;
    float version = mjv + mnv;
    return version;
}

// This gets the software version of the sensor
float scan::getSWVersion(void)
{
    modbus.getRegisters(0x04, 19, 2);
    String _model = modbus.StringFromFrame(4);
    float mjv = _model.substring(0,2).toFloat();
    float mnv = (_model.substring(2,4).toFloat())/100;
    float version = mjv + mnv;
    return version;
}

// This gets the number of times the spec has been rebooted
// (Device rebooter counter)
int scan::getHWStarts(void)
{return modbus.uint16FromRegister(0x04, 21);}

// This gets the number of parameters the spectro::lyzer is set to measure
int scan::getParameterCount(void)
{return modbus.uint16FromRegister(0x04, 22);}

// This gets the datatype of the parameters and parameter limits
// This is a check for compatibility
int scan::getParameterType(void)
{return modbus.uint16FromRegister(0x04, 23);}

// This returns the parameter type as a string
String scan::parseParameterType(uint16_t code)
{
    switch (code)
    {
        case 0: return "uint16?";
        case 1: return "enum?";
        case 2: return "bitmask?";
        case 3: return "char?";
        case 4: return "float?";
        default: return "Unknown";
    }
}

// This gets the scaling factor for all parameters which depend on eParameterType
int scan::getParameterScale(void)
{return modbus.uint16FromRegister(0x04, 24);}

// This returns the spectral path length in mm
// NB This is not documented - I'm guessing based on register values
float scan::getPathLength(void)
{
    int path = modbus.uint16FromRegister(0x04, 520);
    float pathmm = path/10;  // Convert to mm
    return pathmm;
}
