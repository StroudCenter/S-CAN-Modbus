/*****************************************************************************
SaveFingerprints.ino

This saves all of the available types fingerprint data (as well as parameter
data) to an SD card in formats set to match the files that would otherwise be
created by ana::lyte or ana::pro.

This does NOT set up the logging for the spectro::lyzer itself, it only requests
data from the spec at periodic intervals and records that data to a file.
You should set up the spectro::lyzer and start it logging using S::CAN's
ana::pro software.

At present, this example does NOT put the datalogger board to sleep between
readings.  It also relies on the spectro::lyzer itself to report the clock time
rather than using any other internal or external clock.
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <scanModbus.h>
#include <scanAnapro.h>
#include <SdFat.h> // To communicate with the SD card

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define how often you want to log
uint32_t logging_interval_minutes = 1L;
uint32_t delay_ms = 1000L*60L*logging_interval_minutes;

// Define the button you will press to begin the program
const uint8_t buttonPin = 21;

// Define the sensor's modbus address
byte modbusAddress = 0x01;  // The sensor's modbus address, or SlaveID
// The default address seems to be 0x04, at 38400 baud, 8 data bits, odd parity, 1 stop bit.

// Define pin number variables
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text

// Construct the S::CAN modbus instance
scan sensor;
// Construct the "ana::pro" instance for printing formatted strings
anapro sensorPr(&sensor);
static time_t lastSampleTime;

// A global variable for the filename
String fileName;
static time_t fileStartTime;

// Setting up the SD card
const int SDCardPin = 12;
SdFat sd;
File parFile;
static char parFileName[25];
File fpFile0, fpFile1, fpFile2, fpFile3, fpFile4, fpFile5, fpFile6, fpFile7;
static char fpFileName0[25], fpFileName1[25], fpFileName2[25], fpFileName3[25],
            fpFileName4[25], fpFileName5[25], fpFileName6[25],fpFileName7[25];

void startFile(File file, String extension, char filenameBuffer[], spectralSource source=fingerprint)
{
    time_t currentTime = sensor.getSystemTime();

    // Check if the start time is within a minute of the last file, to avoid having
    // many files with nearly-but-not-quite identical file names just a second or two off
    if (fileStartTime > 0 && currentTime - fileStartTime > 60) fileStartTime = currentTime;
    else if (fileStartTime == 0) fileStartTime = currentTime;

    String filename = "";
    filename += sensorPr.timeToStringDash(fileStartTime);
    if (extension == "fp")
    {
        filename += "_";
        filename += source;
    }
    filename += ".";
    filename += extension;
    filename.toCharArray(filenameBuffer, 25);
    Serial.print(F("Creating file "));
    Serial.println(filenameBuffer);

    // Create and open the file in write mode
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

    // Write the header
    if (extension == "fp")
    {
        sensorPr.printFingerprintHeader(file, "    ", source);
        sensorPr.printFingerprintHeader(Serial, "    ", source);
    }
    else
    {
        sensorPr.printParameterHeader(file);
        sensorPr.printParameterHeader(Serial);
    }

    //Close the file to save it
    file.close();
    Serial.print(F("   ... Success!\n"));
    Serial.println("=======================");
}

void writeToFile(File file, String extension, char filenameBuffer[], spectralSource source=fingerprint)
{
    // Make sure this is a new time
    time_t lastSampleTimeCheck = sensor.getParameterTime();
    if (lastSampleTimeCheck == lastSampleTime) return;
    else lastSampleTime = lastSampleTimeCheck;

    // Check if the file already exists, else create a new one
    if (!sd.exists(filenameBuffer)) startFile(file, extension, filenameBuffer);

    // Check that the file is less than 2MB, else create a new one
    else if (file.size() > 2000000L) startFile(file, extension, filenameBuffer);

    Serial.print(F("Writing to file "));
    Serial.println(filenameBuffer);

    time_t currentTime = sensor.getSystemTime();

    // Open the file in write mode
    file.open(filenameBuffer, O_WRITE | O_AT_END);
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

    // Write the data
    if (extension == "fp")
    {
        sensorPr.printFingerprintDataRow(file, "    ", source);
        sensorPr.printFingerprintDataRow(Serial, "    ", source);
    }
    else
    {
        sensorPr.printParameterDataRow(file);
        sensorPr.printParameterDataRow(Serial);
    }

    //Close the file to save it
    file.close();
    Serial.print(F("   ... Success!\n"));
    Serial.println("=======================");
}

// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{
    if (DEREPin > 0) pinMode(DEREPin, OUTPUT);
    if (buttonPin > 0) pinMode(buttonPin, INPUT_PULLUP);

    Serial.begin(57600);  // Main serial port for debugging via USB Serial Monitor
    Serial1.begin(38400, SERIAL_8O1);
    // The default baud rate for the spectro::lyzer is 38400, 8 data bits, odd parity, 1 stop bit

    // Start up the sensor
    // sensorPr.begin(modbusAddress, &modbusSerial, DEREPin);
    sensor.begin(modbusAddress, Serial1, DEREPin);

    // Turn on debugging
    // sensorPr.setDebugStream(&Serial);

    // Start up note
    Serial.println("S::CAN Spect::lyzer Data Recording");

    // Allow the sensor and converter to warm up
    if (buttonPin > 0)
    {
        Serial.print("Communication will begin after pushing the button connected to pin ");
        Serial.println(buttonPin);
        while(true) if (digitalRead(buttonPin) == HIGH) break;
        delay(500);
    }
    else
    {
        // Allow the sensor and converter to warm up
        Serial.println("Waiting for sensor and adapter to be ready.");
        delay(500);
    }

    // Print out all of the setup information
    sensor.wakeSpec();
    sensor.printSetup(Serial);
    Serial.println("=======================");

    // Print out the device status
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
        startFile(fpFile0, "fp", fpFileName0, fingerprint);
        startFile(fpFile1, "fp", fpFileName1, compensFP);
        startFile(fpFile2, "fp", fpFileName2, derivFP);
        startFile(fpFile3, "fp", fpFileName3, diff2oldorgFP);
        startFile(fpFile4, "fp", fpFileName4, transmission);
        startFile(fpFile5, "fp", fpFileName5, derivcompFP);
        startFile(fpFile6, "fp", fpFileName6, transmission10);
        startFile(fpFile7, "fp", fpFileName7, other);
    }
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{
    // Track how long the loop takes
    uint32_t startLoop = millis();

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

        sensor.wakeSpec();
        writeToFile(parFile, "par", parFileName);
        writeToFile(fpFile0, "fp", fpFileName0, fingerprint);
        writeToFile(fpFile1, "fp", fpFileName1, compensFP);
        writeToFile(fpFile2, "fp", fpFileName2, derivFP);
        writeToFile(fpFile3, "fp", fpFileName3, diff2oldorgFP);
        writeToFile(fpFile4, "fp", fpFileName4, transmission);
        writeToFile(fpFile5, "fp", fpFileName5, derivcompFP);
        writeToFile(fpFile6, "fp", fpFileName6, transmission10);
        writeToFile(fpFile7, "fp", fpFileName7, other);
    }

    // track how long the loop took
    Serial.print(F("Writing to the SD card took "));
    Serial.print(millis() - startLoop);
    Serial.println("ms");


    // Wait
    delay(delay_ms - (millis() - startLoop));

}
