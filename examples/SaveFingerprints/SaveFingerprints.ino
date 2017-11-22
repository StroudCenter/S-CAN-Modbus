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
#include <SdFat.h> // To communicate with the SD card
#include <scanModbus.h>
#include <scanAnapro.h>

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define how often you want to log
bool startLogger = false;  // if you want to use this program to start logging
uint32_t logging_interval_minutes = 2L;
uint16_t logging_interval_seconds = round(logging_interval_minutes*60);
uint32_t delay_ms = 1000L*60L*logging_interval_minutes;


// Define the cleaning parameters
uint16_t cleaning_interval_minutes = 360;  // how frequently cleaning should happen
uint16_t cleaning_interval_seconds = round(cleaning_interval_minutes*60);
uint8_t cleaning_duration_seconds = 1;  // the amount of time the cleaning valve is open
uint8_t cleaning_wait_seconds = 20;  // the delay between the air blast and the measurement

// Define the button you will press to begin the program
const uint8_t buttonPin = 21;

// Define enable pin
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)

// Define the spectro::lyzer's modbus address
byte specModbusAddress = 0x04;
// The default address seems to be 0x04, at 38400 baud, 8 data bits, odd parity, 1 stop bit.

// Construct the S::CAN modbus instance
scan spectro;
// Construct the "ana::pro" instance for printing formatted strings
anapro spectroPr(&spectro);
bool isSpec;  // as opposed to a controller with ana::gate

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
    time_t currentTime = spectro.getSystemTime();

    // Check if the start time is within a minute of the last file, to avoid having
    // many files with nearly-but-not-quite identical file names just a second or two off
    if (fileStartTime > 0 && currentTime - fileStartTime > 60) fileStartTime = currentTime;
    else if (fileStartTime == 0) fileStartTime = currentTime;

    String filename = "";
    filename += spectroPr.timeToStringDash(fileStartTime);
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
        spectroPr.printFingerprintHeader(file, "\t", source);
        spectroPr.printFingerprintHeader(Serial, "\t", source);
    }
    else
    {
        spectroPr.printParameterHeader(file);
        spectroPr.printParameterHeader(Serial);
    }

    //Close the file to save it
    file.close();
    Serial.print(F("   ... Success!\n"));
    Serial.println("=======================");
}

void writeToFile(File file, String extension, char filenameBuffer[], spectralSource source=fingerprint)
{
    // Check if the file already exists, else create a new one
    if (!sd.exists(filenameBuffer)) startFile(file, extension, filenameBuffer);

    // Check that the file is less than 2MB, else create a new one
    else if (file.size() > 2000000L) startFile(file, extension, filenameBuffer);

    Serial.print(F("Writing to file "));
    Serial.println(filenameBuffer);

    time_t currentTime = spectro.getSystemTime();

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
        spectroPr.printFingerprintDataRow(file, "\t", source);
        spectroPr.printFingerprintDataRow(Serial, "\t", source);
    }
    else
    {
        spectroPr.printParameterDataRow(file);
        spectroPr.printParameterDataRow(Serial);
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
    spectro.begin(specModbusAddress, Serial1, DEREPin);

    // Turn on debugging
    // spectro.setDebugStream(&Serial);

    // Start up note
    Serial.println("S::CAN Spect::lyzer Data Recording");

    // Allow the RS485 adapter to warm up
    if (buttonPin > 0)
    {
        Serial.print("Communication will begin after pushing the button connected to pin ");
        Serial.println(buttonPin);
        while(true) if (digitalRead(buttonPin) == HIGH) break;
        delay(500);
    }
    else
    {
        Serial.println("Waiting for sensor and adapter to be ready.");
        delay(500);
    }

    if (spectro.getModelType() == 0x0603) isSpec = false;
    else isSpec = true;

    if (isSpec && startLogger)
    {
        // Turn off logging just in case it had been on.
        // The set-up cannot be changed while in logging mode.
        spectro.setLoggingMode(1);

        // Set the logging interval
        Serial.println("Set the measurement interval to ");
        Serial.print(logging_interval_minutes);
        Serial.println(" minute[s]");
        spectro.setMeasInterval(logging_interval_seconds);
        delay(500);

        // Set the cleaning interval
        Serial.println("Set the cleaning interval to ");
        Serial.print(cleaning_interval_minutes);
        Serial.println(" minute[s]");
        spectro.setCleaningInterval(cleaning_interval_seconds);
        delay(500);

        // Set the cleaning duration (the amount of time the valve is open)
        Serial.println("Set the cleaning duration to ");
        Serial.print(cleaning_duration_seconds);
        Serial.println(" second[s]");
        spectro.setCleaningDuration(cleaning_duration_seconds);
        delay(500);

        // Set the cleaning wait (the delay between the air blast and the measurement)
        Serial.println("Set the cleaning interval to ");
        Serial.print(cleaning_wait_seconds);
        Serial.println(" second[s]");
        spectro.setCleaningWait(cleaning_wait_seconds);
        delay(500);
    }
    else if (isSpec)
    {
        Serial.print("Current measurement interval is: ");
        Serial.print(spectro.getMeasInterval());
        Serial.println(" seconds");
    }

    // Print out the device setup
    spectro.printSetup(Serial);

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
        if (isSpec)
        {
            startFile(fpFile1, "fp", fpFileName1, compensFP);
            // startFile(fpFile2, "fp", fpFileName2, derivFP);
            // startFile(fpFile3, "fp", fpFileName3, diff2oldorgFP);
            // startFile(fpFile4, "fp", fpFileName4, transmission);
            // startFile(fpFile5, "fp", fpFileName5, derivcompFP);
            // startFile(fpFile6, "fp", fpFileName6, transmission10);
            // startFile(fpFile7, "fp", fpFileName7, other);
        }
    }

    if (isSpec && startLogger)
    {
        // Wait for an even interval of the logging interval to start the logging
        uint32_t now = spectro.getSystemTime();
        uint32_t secToWait = logging_interval_seconds - (now % logging_interval_seconds);
        Serial.print("Current time is ");
        Serial.println(anapro::timeToStringDot(now));
        Serial.print("Waiting ");
        Serial.print(secToWait);
        Serial.println(" seconds to begin logging at an even interval");
        delay((secToWait*1000) - 500);  // send the command half a second early

        Serial.println("Turning on Logging");
        spectro.setLoggingMode(0);
        Serial.println("Waiting for spectro::lyser to be ready after measurement.");
        delay(55000);
    }
    else while (spectro.getParameterTime() == 0){};
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

        spectro.wakeSpec();
        writeToFile(parFile, "par", parFileName);
        writeToFile(fpFile0, "fp", fpFileName0, fingerprint);
        if (isSpec)  // These fields don't exist in ana::pro
        {
            writeToFile(fpFile1, "fp", fpFileName1, compensFP);
            // writeToFile(fpFile2, "fp", fpFileName2, derivFP);
            // writeToFile(fpFile3, "fp", fpFileName3, diff2oldorgFP);
            // writeToFile(fpFile4, "fp", fpFileName4, transmission);
            // writeToFile(fpFile5, "fp", fpFileName5, derivcompFP);
            // writeToFile(fpFile6, "fp", fpFileName6, transmission10);
            // writeToFile(fpFile7, "fp", fpFileName7, other);
        }
    }

    // track how long the loop took
    Serial.print(F("Writing to the SD card took "));
    Serial.print(millis() - startLoop);
    Serial.println("ms");


    // Wait
    delay(delay_ms - (millis() - startLoop));

}
