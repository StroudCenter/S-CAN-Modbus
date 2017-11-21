/*
 *scanAnapro.h
*/

#ifndef scanAnapro_h
#define scanAnapro_h

#include <scanModbus.h>  // For modbus communication
#include <TimeLib.h>  // for dealing with the TAI64/Unix time


//----------------------------------------------------------------------------
//          FUNCTIONS TO CREATE PRINTOUTS THAT WILL READ INTO ANA::XXX
//----------------------------------------------------------------------------
class anapro
{

public:

    anapro(scan *scanMB){_scanMB = scanMB;}

    // This is for the first line of both headers (below)
    void printFirstLine(Stream *stream);
    void printFirstLine(Stream &stream);

    // This prints out a header for a "par" file ini the format that the
    // s::can/ana::xxx software is expecting
    // The delimeter is changable, but if you use anything other than the
    // default TAB (\t, 0x09) the s::can/ana::xxx software will not read it.
    void printParameterHeader(Stream *stream, const char *dlm="\t");
    void printParameterHeader(Stream &stream, const char *dlm="\t");

    // This prints the data from ALL parameters as delimeter separated data.
    // By default, the delimeter is a TAB (\t, 0x09), as expected by the s::can/ana::xxx software.
    // This includes the parameter timestamp and status.
    // NB:  You can use this to print to a file on a SD card!
    void printParameterDataRow(Stream *stream, const char *dlm="\t");
    void printParameterDataRow(Stream &stream, const char *dlm="\t");

    // This prints out a header for a "fp" file ini the format that the
    // s::can/ana::xxx software is expecting
    void printFingerprintHeader(Stream *stream, const char *dlm="\t", spectralSource source=fingerprint);
    void printFingerprintHeader(Stream &stream, const char *dlm="\t", spectralSource source=fingerprint);

    // This prints a fingerprint data rew
    // NB:  You can use this to print to a file on a SD card!
    void printFingerprintDataRow(Stream *stream, const char *dlm="\t",
    spectralSource source=fingerprint);
    void printFingerprintDataRow(Stream &stream, const char *dlm="\t",
    spectralSource source=fingerprint);

    // This converts a unix timestamp to a string formatted as YYYY.MM.DD hh:mm:ss
    static String timeToStringDot(time_t time);

    // This converts a unix timestamp to a string formatted as YYYY.MM.DD hh:mm:ss
    static String timeToStringDash(time_t time);

private:
    // This just makes a two digit number with a preceeding 0 if necessary
    static String Str2Digit(int value);

    // The internal link to the s::can class instance
    scan *_scanMB;
};

#endif
