/*****************************************************************************
getParameterValues.ino

This prints basic meta-data about a sensor to the first serial port and then
begins taking measurements from the sensor.

The sensor model and address can easily be modified to use this sketch with any
Yosemitech modbus sensor.
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <scanModbus.h>
#include <SdFat.h> // To communicate with the SD card

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define the sensor's modbus address
byte modbusAddress = 0x04;  // The sensor's modbus address, or SlaveID
// Yosemitech ships sensors with a default ID of 0x01.

// Define pin number variables
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text

// Construct the Yosemitech modbus instance
scan sensor;
bool success;

// A global variable for the filename
String fileName;

// Setting up the SD card
SdFat sd;
SdFile file;

void createFiles(void)
{

}

// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{
    if (DEREPin > 0) pinMode(DEREPin, OUTPUT);

    Serial.begin(57600);  // Main serial port for debugging via USB Serial Monitor
    Serial1.begin(38400, SERIAL_8N2);
    // modbusSerial.begin(38400);  // The modbus serial stream
    // The default baud rate for the spectro::lyzer is 38400

    // Start up the sensor
    // sensor.begin(modbusAddress, &modbusSerial, DEREPin);
    sensor.begin(modbusAddress, Serial1, DEREPin);

    // Turn on debugging
    // sensor.setDebugStream(&Serial);

    // Start up note
    Serial.println("S::CAN Spect::lyzer Data Recording");

    // Allow the sensor and converter to warm up
    Serial.println("Waiting for sensor and adapter to be ready.");
    delay(500);

    // Print out all of the setup information
    sensor.printSetup(Serial);
    sensor.printParameterHeader(Serial);
    sensor.printFingerprintHeader(Serial);

    // Print out the device status
    uint16_t status;
    status = sensor.getDeviceStatus();
    Serial.print("Current device status is: ");
    Serial.println(status, BIN);
    sensor.printDeviceStatus(status, Serial);
    Serial.println("=======================");
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{
    sensor.printParameterData(Serial);
    sensor.printFingerprintData(Serial);

    // Wait 2 minutes
    delay(120000);
}
