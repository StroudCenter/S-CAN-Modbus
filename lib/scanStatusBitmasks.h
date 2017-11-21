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
