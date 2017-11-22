/*****************************************************************************
DisplayParameters.ino

This saves fingerprint data to an SD card and also prints the parameter data to
a small OLED screen.
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <Wire.h>  // For the I2C for the OLED display
#include <AMAdafruit_GFX.h>  // For the OLED display
#include <SDL_Arduino_SSD1306.h>  // For the OLED display
#include <SdFat.h> // To communicate with the SD card
#include <scanModbus.h>
#include <scanAnapro.h>

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define how often you want to log
bool startLogger = true;  // if you want to use this program to start logging
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

// Set up the OLED display
SDL_Arduino_SSD1306 display(-1);  // using I2C and not bothering with a reset pin

// A global variable for the filename
String fileName;
static time_t fileStartTime;

// Setting up the SD card
const int SDCardPin = 12;
SdFat sd;
File parFile;
static char parFileName[25];
File fpFile0;
static char fpFileName0[25];

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
}

// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{
    if (DEREPin > 0) pinMode(DEREPin, OUTPUT);
    if (buttonPin > 0) pinMode(buttonPin, INPUT_PULLUP);

    Serial1.begin(38400, SERIAL_8O1);
    // The default baud rate for the spectro::lyzer is 38400, 8 data bits, odd parity, 1 stop bit

    // Start up the sensor
    spectro.begin(specModbusAddress, Serial1, DEREPin);

    // Start the OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);

    // Turn on debugging
    // spectro.setDebugStream(&Serial);

    // Start up note
    display.println("S::CAN Spect::lyzer\nData Viewing\n");
    display.display();
    delay(2000);

    // Allow the RS485 adapter to warm up
    if (buttonPin > 0)
    {
        display.print("Push the button\non pin ");
        display.println(buttonPin);
        display.display();
        while(true) if (digitalRead(buttonPin) == HIGH)
            {break; display.clearDisplay();}
        delay(500);
    }

    if (spectro.getModelType() == 0x0603) isSpec = false;
    else isSpec = true;

    if (isSpec && startLogger)
    {
        // Turn off logging just in case it had been on.
        // The set-up cannot be changed while in logging mode.
        spectro.setLoggingMode(1);

        // Re-set the logging interval
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Set the measurement interval to ");
        display.print(logging_interval_minutes);
        display.println(" minute[s]");
        display.display();
        spectro.setMeasInterval(logging_interval_seconds);
        delay(2000);

        // Set the cleaning interval
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Set the cleaning interval to ");
        display.print(cleaning_interval_minutes);
        display.println(" minute[s]");
        display.display();
        spectro.setCleaningInterval(cleaning_interval_seconds);
        delay(2000);

        // Set the cleaning duration (the amount of time the valve is open)
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Set the cleaning duration to ");
        display.print(cleaning_duration_seconds);
        display.println(" second[s]");
        display.display();
        spectro.setCleaningDuration(cleaning_duration_seconds);
        delay(2000);

        // Set the cleaning wait (the delay between the air blast and the measurement)
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Set the cleaning interval to ");
        display.print(cleaning_wait_seconds);
        display.println(" second[s]");
        display.display();
        spectro.setCleaningWait(cleaning_wait_seconds);
        delay(2000);
    }
    else if (isSpec)
    {
        display.clearDisplay();
        display.setCursor(0,0);
        display.print("Current measurement interval:\n");
        display.print(spectro.getMeasInterval());
        display.println(" seconds");
        display.display();
    }

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
    }

    if (isSpec && startLogger)
    {
        // Wait for an even interval of the logging interval to start the logging
        uint32_t now = spectro.getSystemTime();
        uint32_t secToWait = logging_interval_seconds - (now % logging_interval_seconds);
        display.clearDisplay();
        display.setCursor(0,0);
        display.print("Current time is\n");
        display.println(anapro::timeToStringDot(now));
        display.print("\nWaiting ");
        display.print(secToWait);
        display.println(" seconds to begin logging at an even interval");
        display.display();
        delay((secToWait*1000) - 500);  // send the command half a second early

        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Turning on Logging");
        spectro.setLoggingMode(0);
        display.display();
        delay(2000);

        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Waiting for spectro::lyser to be ready after measurement.");
        display.display();
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
    if (sd.begin(SDCardPin, SPI_FULL_SPEED))
    {
        sensor.wakeSpec();
        writeToFile(parFile, "par", parFileName);
        writeToFile(fpFile0, "fp", fpFileName0, fingerprint);
    }

    display.clearDisplay();
    display.setCursor(0,0);
    display.print(anapro::timeToStringDot(spectro.getParameterTime()));
    display.println();
    display.display();

    // Print values one at a time
    for (int i = 1; i < spectro.getParameterCount()+1; i++)
    {
        display.print(spectro.getParameterName(i));
        display.print(" - ");
        display.print(spectro.getParameterValue(i));
        display.print(" ");
        display.print(spectro.getParameterUnits(i));
        display.println();
        display.display();
    }

    // Wait
    delay(delay_ms - (millis() - startLoop));

}
