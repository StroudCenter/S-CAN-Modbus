/*****************************************************************************
getParameterValues.ino

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
bool isSpec;  // as opposed to a controller with ana::gate

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

    // Print out the device setup
    spectro.printSetup(Serial);

    if (isSpec && startLogger)
    {
        // Wait for an even interval of the logging interval to start the logging
        uint32_t now = spectro.getSystemTime();
        uint32_t secToWait = logging_interval_seconds - (now % logging_interval_seconds);
        Serial.print("Current time is ");
        Serial.println(now);
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
    // Print out the device status
    Serial.println("=================");
    Serial.println("=================");
    uint16_t status;
    status = spectro.getDeviceStatus();
    Serial.print("Current device status is: ");
    Serial.println(status, BIN);
    spectro.printDeviceStatus(status, Serial);

    Serial.print("Last sample was taken at ");
    Serial.print((unsigned long)(spectro.getParameterTime()));
    Serial.println(" seconds past Jan 1, 1970");

    // Get values one at a time
    for (int i = 1; i < spectro.getParameterCount()+1; i++)
    {
        Serial.println("----");
        Serial.print("Parameter number ");
        Serial.print(i);
        Serial.print(" is: ");
        Serial.print(spectro.getParameterName(i));
        Serial.print(" which currently has a value of ");
        Serial.print(spectro.getParameterValue(i));
        Serial.print(" ");
        Serial.print(spectro.getParameterUnits(i));
        Serial.print(" and status code ");
        uint16_t parm_status = spectro.getParameterStatus(i);
        Serial.println(parm_status, BIN);
        spectro.printParameterStatus(parm_status, Serial);
    }

    // wait until next reading
    delay(delay_ms);
}
