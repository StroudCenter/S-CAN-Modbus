/*
 *scanModularSensorLinker.h
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *
 *This sets the s::can up as a sensor in the ModularSensors Library.
*/

#ifndef scanModularSensorLinker_h
#define scanModularSensorLinker_h

#include <Arduino.h>

// #define DEBUGGING_SERIAL_OUTPUT Serial
// #define DEEP_DEBUGGING_SERIAL_OUTPUT Serial
// #include <ModSensorDebugger.h>

#include "scanModbus.h"
#include <SensorBase.h>
#include <VariableBase.h>

#define SCAN_NUM_VARIABLES 10
#define SCAN_WARM_UP_TIME_MS 3000  // Takes up to 3 seconds to be ready to respond
#define SCAN_STABILIZATION_TIME_MS 0  // This is the spec's problem
#define SCAN_MEASUREMENT_TIME_MS 0  // This is the spec's problem

#define SCAN_RESOLUTION 2

#define SCAN_TURB_VAR_NUM 0
#define SCAN_NO3_VAR_NUM 1
#define SCAN_TOC_VAR_NUM 2
#define SCAN_DOC_VAR_NUM 3
#define SCAN_SAC254T_VAR_NUM 4
#define SCAN_SAC436T_VAR_NUM
#define SCAN_TEMP_VAR_NUM 6
#define SCAN_VOLT_VAR_NUM 7
#define SCAN_DOPPM_VAR_NUM 9
#define SCAN_DOTEMP_VAR_NUM 8

// The main class for the Yosemitech Sensors
class scanSensor : public Sensor
{
public:
    scanSensor(byte modbusAddress, Stream* stream, int8_t enablePin = -1,
               String sensName = "S::CAN", int numVariables = 2);
    scanSensor(byte modbusAddress, Stream& stream, int8_t enablePin = -1,
               String sensName = "S::CAN", int numVariables = 2);

    String getSensorLocation(void) override;

    virtual bool setup(void) override;
    virtual bool wake(void) override;

    virtual bool addSingleMeasurementResult(void);

private:
    byte _modbusAddress;
    Stream* _stream;
    int _RS485EnablePin;
    scan sensor;
};


class SCAN_Turbidity : public Variable
{
public:
    SCAN_Turbidity(Sensor *parentSense,
                  String UUID = "", String customVarCode = "")
     : Variable(parentSense, SCAN_TURB_VAR_NUM,
                F("turbidity"), F("nephelometricTurbidityUnit"),
                SCAN_RESOLUTION,
                F("SCANDOmgL"), UUID, customVarCode)
    {}
};


class SCAN_NO3 : public Variable
{
public:
    SCAN_NO3(Sensor *parentSense,
                  String UUID = "", String customVarCode = "")
      : Variable(parentSense, SCAN_NO3_VAR_NUM,
                 F("nitrogenDissolvedNitrate_NO3"), F("milligramPerLiter"),
                 SCAN_RESOLUTION,
                 F("SCANNO3"), UUID, customVarCode)
    {}
};


class SCAN_TOC : public Variable
{
public:
    SCAN_TOC(Sensor *parentSense,
                  String UUID = "", String customVarCode = "")
      : Variable(parentSense, SCAN_TOC_VAR_NUM,
                 F("carbonDissolvedTotal"), F("milligramPerLiter"),
                 SCAN_RESOLUTION,
                 F("SCANTOC"), UUID, customVarCode)
    {}
};


class SCAN_DOC : public Variable
{
public:
    SCAN_DOC(Sensor *parentSense,
                  String UUID = "", String customVarCode = "")
      : Variable(parentSense, SCAN_DOC_VAR_NUM,
                 F("carbonDissolvedOrganic"), F("milligramPerLiter"),
                 SCAN_RESOLUTION,
                 F("SCANDOC"), UUID, customVarCode)
    {}
};


class SCAN_SAC254T : public Variable
{
public:
    SCAN_SAC254T(Sensor *parentSense,
                  String UUID = "", String customVarCode = "")
      : Variable(parentSense, SCAN_SAC254T_VAR_NUM,
                 F("SUVA254"), F("AbsorbanceUnitPerMeter"),
                 SCAN_RESOLUTION,
                 F("SCANA254"), UUID, customVarCode)
    {}
};


class SCAN_SAC436T : public Variable
{
public:
    SCAN_SAC436T(Sensor *parentSense,
                  String UUID = "", String customVarCode = "")
      : Variable(parentSense, SCAN_SAC436T_VAR_NUM,
                 F("SAC436T"), F("AbsorbanceUnitPerMeter"),
                 SCAN_RESOLUTION,
                 F("SCANC436"), UUID, customVarCode)
    {}
};


class SCAN_Temperature : public Variable
{
public:
    SCAN_Temperature(Sensor *parentSense,
                  String UUID = "", String customVarCode = "")
      : Variable(parentSense, SCAN_TEMP_VAR_NUM,
                 F("temperature"), F("degreeCelsius"),
                 SCAN_RESOLUTION,
                 F("SCANtemp"), UUID, customVarCode)
    {}
};


class SCAN_Voltage : public Variable
{
public:
    SCAN_Voltage(Sensor *parentSense,
                  String UUID = "", String customVarCode = "")
      : Variable(parentSense, SCAN_VOLT_VAR_NUM,
                 F("batteryVoltage"), F("volt"),
                 SCAN_RESOLUTION,
                 F("SCANVotage"), UUID, customVarCode)
    {}
};


// Defines the Dissolved Oxygen Concentration
class SCAN_DOmgL : public Variable
{
public:
    SCAN_DOmgL(Sensor *parentSense,
               String UUID = "", String customVarCode = "")
     : Variable(parentSense, SCAN_DOPPM_VAR_NUM,
                F("oxygenDissolved"), F("milligramPerLiter"),
                SCAN_RESOLUTION,
                F("SCANDOmgL"), UUID, customVarCode)
    {}
};


// Defines the DO Temperature Variable
class SCAN_DOTemp : public Variable
{
public:
    SCAN_DOTemp(Sensor *parentSense,
                String UUID = "", String customVarCode = "")
     : Variable(parentSense, SCAN_DOTEMP_VAR_NUM,
                F("temperature"), F("degreeCelsius"),
                SCAN_RESOLUTION,
                F("SCANDOtemp"), UUID, customVarCode)
    {}
};

#endif
