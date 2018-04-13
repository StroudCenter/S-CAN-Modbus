/*
 *scanModularSensorLinker.cpp
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *
 *This sets the s::can up as a sensor in the ModularSensors Library.
*/

#include "scanModularSensorLinker.h"

// The constructor - need the sensor type, modbus address, power pin, stream for data, and number of readings to average
scanSensor::scanSensor(byte modbusAddress, Stream* stream, int8_t enablePin,
                       String sensName, int numVariables)
    : Sensor(sensName, numVariables,
             SCAN_WARM_UP_TIME_MS, SCAN_STABILIZATION_TIME_MS, SCAN_MEASUREMENT_TIME_MS,
             -1, -1, 1)
{
    _modbusAddress = modbusAddress;
    _stream = stream;
    _RS485EnablePin = enablePin;
}
scanSensor::scanSensor(byte modbusAddress, Stream& stream, int8_t enablePin,
                       String sensName, int numVariables)
    : Sensor(sensName, numVariables,
             SCAN_WARM_UP_TIME_MS, SCAN_STABILIZATION_TIME_MS, SCAN_MEASUREMENT_TIME_MS,
             -1, -1, 1)
{
    _modbusAddress = modbusAddress;
    _stream = &stream;
    _RS485EnablePin = enablePin;
}


// The sensor installation location on the Mayfly
String scanSensor::getSensorLocation(void)
{
    String sensorLocation = F("modbus_0x");
    if (_modbusAddress< 16) sensorLocation += "0";
    sensorLocation += String(_modbusAddress, HEX);
    return sensorLocation;
}


bool scanSensor::setup(void)
{
    bool retVal = Sensor::setup();  // sets time stamp and status bits
    if (_RS485EnablePin > 0) pinMode(_RS485EnablePin, OUTPUT);

    #if defined(DEEP_DEBUGGING_SERIAL_OUTPUT)
        sensor.setDebugStream(&DEEP_DEBUGGING_SERIAL_OUTPUT);
    #endif

    retVal &= sensor.begin(_modbusAddress, _stream, _RS485EnablePin);

    return retVal;
}


// The function to wake up a sensor
// Different from the standard in that it waits for warm up and starts measurements
bool scanSensor::wake(void)
{
    bool success = sensor.wakeSpec();
    success += Sensor::wake();
    return success;
}



bool scanSensor::addSingleMeasurementResult(void)
{
    // Initialize float variables
    float values[8] = {-9999, -9999, -9999, -9999, -9999, -9999, -9999, -9999};

    if (_millisMeasurementRequested > 0)
    {
        // Get how many parms are currently being recorded
        int nparms = sensor.getParameterCount();
        MS_DBG(F("S::CAN is currently recording "), nparms, F(" values.\n"));

        // Get Values
        MS_DBG(F("Get Values:\n"));

        for (uint8_t i = 0; i < nparms; i++)
        {
            // Get the value
            values[i] = sensor.getParameterValue(i+1);
            // Fix not-a-number values
            if (isnan(values[i])) values[i] = -9999;

            MS_DBG(F("    "), sensor.getParameterName(i+1), F(": "),
                   values[i], sensor.getParameterUnits(i+1), F("\n"));

           // Put values into the array
           verifyAndAddMeasurementResult(i, values[i]);
        }
    }
    else MS_DBG(F("Sensor is not currently measuring!\n"));

    // Unset the time stamp for the beginning of this measurement
    _millisMeasurementRequested = 0;
    // Unset the status bit for a measurement having been requested (bit 5)
    _sensorStatus &= 0b11011111;
    // Set the status bit for measurement completion (bit 6)
    _sensorStatus |= 0b01000000;

    // Return true when finished
    return true;
}
