/*
 *scanAnapro.cpp
*/

#include "scanAnapro.h"


//----------------------------------------------------------------------------
//          FUNCTIONS TO CREATE PRINTOUTS THAT WILL READ INTO ANA::XXX
//----------------------------------------------------------------------------

// This creates the first line of an s::can/ana::xxx header
void anapro::printFirstLine(Stream *stream)
{
    stream->print(_scanMB->getSerialNumber());
    stream->print("_");
    stream->print(_scanMB->getPathLength()*10, 0);
    stream->print("_0x0");
    stream->print(_scanMB->getModelType(), HEX);
    stream->print("_");
    stream->print(_scanMB->getModel());
    stream->print("_");
    stream->println(_scanMB->getCurrentGlobalCal());

}
void anapro::printFirstLine(Stream &stream){printFirstLine(&stream);}

// This prints out a header for a "par" file in the format that the
// s::can/ana::xxx software is expecting
// The delimeter is changable, but if you use anything other than the
// default TAB (\t, 0x09) the s::can/ana::xxx software will not read it.
void anapro::printParameterHeader(Stream *stream, const char *dlm)
{
    printFirstLine(stream);
    stream->print("Date/Time");
    stream->print(dlm);
    stream->print("Status");
    stream->print(dlm);
    int nparms = _scanMB->getParameterCount();
    for (int i = 0; i < nparms; i++)
    {
        stream->print(_scanMB->getParameterName(i+1));
        stream->print("[");
        stream->print(_scanMB->getParameterUnits(i+1));
        stream->print("]");
        stream->print(_scanMB->getParameterLowerLimit(i+1));
        stream->print("-");
        stream->print(_scanMB->getParameterUpperLimit(i+1));
        stream->print("_");
        stream->print(_scanMB->getParameterPrecision(i+1));
        stream->print(dlm);
        stream->print(_scanMB->getParameterName(i+1));
        stream->print("_");
        stream->print(_scanMB->getParameterCalibOffset(i+1));
        stream->print("_");
        stream->print(_scanMB->getParameterCalibSlope(i+1));
        stream->print("_");
        stream->print(_scanMB->getParameterCalibX2(i+1));
        stream->print("_");
        stream->print(_scanMB->getParameterCalibX3(i+1));
        if (i < nparms-1) stream->print(dlm);
    }
    stream->println();
}
void anapro::printParameterHeader(Stream &stream, const char *dlm)
{printParameterHeader(&stream, dlm);}

// This prints the data from ALL parameters as delimeter separated data.
// By default, the delimeter is a TAB (\t, 0x09), as expected by the s::can/ana::xxx software.
// This includes the parameter timestamp and status.
// NB:  You can use this to print to a file on a SD card!
void anapro::printParameterDataRow(Stream *stream, const char *dlm)
{
    // Print out the timestamp
    stream->print(timeToStringDot(_scanMB->getParameterTime()));
    stream->print(dlm);
    // Get and print the system status
    int sysStat = _scanMB->getSystemStatus();
    if (sysStat == 0) {stream->print("Ok"); stream->print(dlm);}
    else {stream->print("Error"); stream->print(dlm);}
    // Get the number of parameters that are being recorded
    int nparms = _scanMB->getParameterCount();
    for (int i = 0; i < nparms; i++)
    {
        float value = _scanMB->getParameterValue(i+1);
        stream->print(value, 3);
        stream->print(dlm);
        stream->print(sysStat);
        if (i < nparms-1) stream->print(dlm);
    }
    stream->println();
}
void anapro::printParameterDataRow(Stream &stream, const char *dlm)
{printParameterDataRow(&stream, dlm);}

// This prints out a header for a "fp" file in the format that the
// s::can/ana::xxx software is expecting
void anapro::printFingerprintHeader(Stream *stream, const char *dlm, spectralSource source)
{
    printFirstLine(stream);
    stream->print("Date/Time");
    stream->print(dlm);
    stream->print("Status");
    stream->print("_");
    stream->print(source);
    for (float i = 200.00; i <= 750.00; i+=2.5)
    {
        stream->print(dlm);
        stream->print(i, 2);
    }
    stream->println();
}
void anapro::printFingerprintHeader(Stream &stream, const char *dlm, spectralSource source)
{printFingerprintHeader(&stream, dlm, source);}

// This is as above, but includes the fingerprint timestamp and status
// NB:  You can use this to print to a file on a SD card!
void anapro::printFingerprintDataRow(Stream *stream, const char *dlm, spectralSource source)
{
    // Print out the timestamp
    stream->print(timeToStringDot(_scanMB->getFingerprintTime(source)));
    stream->print(dlm);
    // Get and print the system status
    if (_scanMB->getSystemStatus() == 0) {stream->print("Ok"); stream->print(dlm);}
    else {stream->print("Error"); stream->print(dlm);}
    // Print out the data values
    _scanMB->printFingerprintData(stream, dlm, source);
}
void anapro::printFingerprintDataRow(Stream &stream, const char *dlm, spectralSource source)
{printFingerprintDataRow(&stream, dlm, source);}

// This just makes a two digit number with a preceeding 0 if necessary
String anapro::Str2Digit(int value)
{
    String str = "";
    if (value < 10) str += "0";
    str += String(value);
    return str;
}

// This converts a unix timestamp to a string formatted as YYYY.MM.DD  hh:mm:ss
String anapro::timeToStringDot(time_t time)
{
    String strTime = "";
    // Print it out in the right format
    strTime += year(time);
    strTime += ".";
    strTime += Str2Digit(month(time));
    strTime += ".";
    strTime += Str2Digit(day(time));
    strTime += "  ";
    strTime += Str2Digit(hour(time));
    strTime += ":";
    strTime += Str2Digit(minute(time));
    strTime += ":";
    strTime += Str2Digit(second(time));
    return strTime;
}

// This converts a unix timestamp to a string formatted as YYYY.MM.DD hh:mm:ss
String anapro::timeToStringDash(time_t time)
{
    String strTime = "";
    // Print it out in the right format
    strTime += year(time);
    strTime += "-";
    strTime += Str2Digit(month(time));
    strTime += "-";
    strTime += Str2Digit(day(time));
    strTime += "_";
    strTime += Str2Digit(hour(time));
    strTime += "-";
    strTime += Str2Digit(minute(time));
    strTime += "-";
    strTime += Str2Digit(second(time));
    return strTime;
}
