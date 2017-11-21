/*****************************************************************************
DisplayParameters.ino

This saves fingerprint data to an SD card and also prints the parameter data to
a small OLED screen.
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <scanModbus.h>
#include <Wire.h>  // For the I2C for the OLED display
#include <AMAdafruit_GFX.h>  // For the OLED display
#include <SDL_Arduino_SSD1306.h>  // For the OLED display
#include <scanAnapro.h>
#include <SdFat.h> // To communicate with the SD card

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define how often you want to log
bool startLogger = true;  // if you want to use this program to start logging
uint32_t logging_interval_minutes = 2L;
uint32_t delay_ms = 1000L*60L*logging_interval_minutes;

// Define the button you will press to begin the program
const uint8_t buttonPin = 21;

// Define the sensor's modbus address
byte modbusAddress = 0x04;  // The sensor's modbus address, or SlaveID
// The default address seems to be 0x04, at 38400 baud, 8 data bits, odd parity, 1 stop bit.

// Define pin number variables
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)

// Construct the S::CAN modbus instance
scan sensor;
// Construct the "ana::pro" instance for printing formatted strings
anapro sensorPr(&sensor);
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
        sensorPr.printFingerprintHeader(file, "\t", source);
        sensorPr.printFingerprintHeader(Serial, "\t", source);
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
    // Check if the file already exists, else create a new one
    if (!sd.exists(filenameBuffer)) startFile(file, extension, filenameBuffer);

    // Check that the file is less than 2MB, else create a new one
    else if (file.size() > 2000000L) startFile(file, extension, filenameBuffer);

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
        sensorPr.printFingerprintDataRow(file, "\t", source);
        sensorPr.printFingerprintDataRow(Serial, "\t", source);
    }
    else
    {
        sensorPr.printParameterDataRow(file);
        sensorPr.printParameterDataRow(Serial);
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
    // sensor.begin(modbusAddress, &modbusSerial, DEREPin);
    sensor.begin(modbusAddress, Serial1, DEREPin);

    // Start the OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);

    // Turn on debugging
    // sensor.setDebugStream(&Serial);

    // Start up note
    display.println("S::CAN Spect::lyzer\nData Viewing\n");
    display.display();
    delay(2000);

    // Allow the sensor and converter to warm up
    if (buttonPin > 0)
    {
        display.print("Push the button\non pin ");
        display.println(buttonPin);
        display.display();
        while(true) if (digitalRead(buttonPin) == HIGH)
            {break; display.clearDisplay();}
        delay(500);
    }

    if (sensor.getModelType() == 0x0603) isSpec = false;
    else isSpec = true;

    if (isSpec && startLogger)
    {
        // Turn off logging just in case it had been on.
        sensor.setLoggingMode(1);
        // Re-set the logging interval
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Set the measurement interval to ");
        display.print(logging_interval_minutes);
        display.println(" minute[s]");
        display.display();
        sensor.setMeasInterval(logging_interval_minutes*60);
        delay(2000);
    }
    else if (isSpec)
    {
        display.clearDisplay();
        display.setCursor(0,0);
        display.print("Current measurement interval:\n");
        display.print(sensor.getMeasInterval());
        display.println(" seconds");
        display.display();
    }

    if (isSpec && startLogger)
    {
        // Wait for an even interval of the logging interval to start the logging
        uint32_t now = sensor.getSystemTime();
        uint32_t secToWait = now % (logging_interval_minutes*60);
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
        sensor.setLoggingMode(0);
        display.display();
        delay(2000);

        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Waiting for spectro::lyser to be ready after measurement.");
        display.display();
        // delay(21000L);  // The spec is just "busy" and cannot communicate for ~21 seconds
        // while (sensor.getParameterTime() == 0){};
        // display.println((millis() - now));
        delay(55000);
    }
    else while (sensor.getParameterTime() == 0){};

    // Initialise the SD card while waiting for registers to be ready
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
    display.print(anapro::timeToStringDot(sensor.getParameterTime()));
    display.println();
    display.display();

    // Print values one at a time
    for (int i = 1; i < sensor.getParameterCount()+1; i++)
    {
        display.print(sensor.getParameterName(i));
        display.print(" - ");
        display.print(sensor.getParameterValue(i));
        display.print(" ");
        display.print(sensor.getParameterUnits(i));
        display.println();
        display.display();
    }

    // Wait
    delay(delay_ms - (millis() - startLoop));

}
