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

// Define how often you want to log
uint32_t logging_interval_minutes = 1L;
uint32_t delay_ms = 1000L*60L*logging_interval_minutes;

// Define the sensor's modbus address
byte modbusAddress = 0x01;  // The sensor's modbus address, or SlaveID
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
File parFile;
static char parFileName[23];
File fpFile;
static char fpFileName[23];
const int SDCardPin = 12;

void startFile(File file, String extension, char filenameBuffer[])
{
    uint32_t currentTime = sensor.getSystemTime();

    String filename = "";
    filename += year(currentTime);
    filename += "-";
    filename += month(currentTime);
    filename += "-";
    filename += day(currentTime);
    filename += "_";
    filename += hour(currentTime);
    filename += "-";
    filename += minute(currentTime);
    filename += "-";
    filename += second(currentTime);
    filename += ".";
    filename += extension;
    filename.toCharArray(filenameBuffer, 23);

    // Open the file in write mode (and create it if it did not exist)
    file.open(filenameBuffer, O_CREAT | O_WRITE | O_AT_END);
    // Set creation date time
    file.timestamp(T_CREATE, year(currentTime),
                             month(currentTime),
                             day(currentTime),
                             hour(currentTime),
                             minute(currentTime),
                             second(currentTime));
    // Set write/modification date time
    file.timestamp(T_WRITE, year(currentTime),
                            month(currentTime),
                            day(currentTime),
                            hour(currentTime),
                            minute(currentTime),
                            second(currentTime));
    // Set access date time
    file.timestamp(T_ACCESS, year(currentTime),
                             month(currentTime),
                             day(currentTime),
                             hour(currentTime),
                             minute(currentTime),
                             second(currentTime));
    Serial.print(F("   ... Files created!\n"));
}

// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{
    if (DEREPin > 0) pinMode(DEREPin, OUTPUT);

    Serial.begin(57600);  // Main serial port for debugging via USB Serial Monitor
    Serial1.begin(38400, SERIAL_8O1);
    // modbusSerial.begin(38400);  // The modbus serial stream
    // The default baud rate for the spectro::lyzer is 38400, 8 data bits, odd parity, 1 stop bit

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
    Serial.println("=======================");
    sensor.printParameterHeader(Serial);
    Serial.println("=======================");
    sensor.printFingerprintHeader(Serial);
    Serial.println("=======================");
    //
    // // Print out the device status
    uint16_t status;
    status = sensor.getDeviceStatus();
    Serial.print("Current device status is: ");
    Serial.println(status, BIN);
    sensor.printDeviceStatus(status, Serial);
    Serial.println("=======================");


    // Initialise the SD card
    if (!sd.begin(SDCardPin, SPI_FULL_SPEED))
    {
        Serial.println(F("Error: SD card failed to initialize or is missing."));
        Serial.println(F("Data will not be saved!."));
    }
    else  // skip everything else if there's no SD card, otherwise it might hang
    {
        Serial.print(F("Successfully connected to SD Card with card/slave select on pin "));
        Serial.println(SDCardPin);

        startFile(parFile, "par", parFileName);
        // Add header information
        sensor.printParameterHeader(parFile);
        //Close the file to save it
        parFile.close();


        startFile(fpFile, "fp", fpFileName);
        // Add header information
        sensor.printFingerprintHeader(fpFile);
        //Close the file to save it
        fpFile.close();
    }
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{
    // Wait
    delay(delay_ms);

    sensor.printParameterDataRow(Serial);
    Serial.println("=======================");
    sensor.printFingerprintDataRow(Serial);
    Serial.println("=======================");
}
