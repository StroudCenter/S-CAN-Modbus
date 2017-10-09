/*****************************************************************************
getParameterValues.ino

This prints all of the setup information for a spectro::lyzer (or ana::gate)
and then puts the spec into logging mode and prints out the data.
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
uint32_t logging_interval_minutes = 1L;
uint32_t delay_ms = 1000L*60L*logging_interval_minutes;

// Define the button you will press to begin the program
const uint8_t buttonPin = 21;

// Define the sensor's modbus address
byte modbusAddress = 0x04;  // The sensor's modbus address, or SlaveID
// The default address seems to be 0x04, at 38400 baud, 8 data bits, odd parity, 1 stop bit.

// Define pin number variables
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text

// Construct the S::CAN modbus instance
scan sensor;
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
    // sensor.begin(modbusAddress, &modbusSerial, DEREPin);
    sensor.begin(modbusAddress, Serial1, DEREPin);

    // Turn on debugging
    // sensor.setDebugStream(&Serial);

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

    if (sensor.getModelType() == 0x0603) isSpec = false;
    else isSpec = true;

    if (isSpec)
    {
        // Turn off logging just in case it had been on.
        sensor.setLoggingMode(1);
        // Re-set the logging interval
        Serial.println("Set the measurement interval to ");
        Serial.print(logging_interval_minutes);
        Serial.println(" minute[s]");
        sensor.setMeasInterval(logging_interval_minutes*60);
        Serial.print("Current measurement interval is: ");
        Serial.print(sensor.getMeasInterval());
        Serial.println(" seconds");
    }

    // Print out the device setup
    sensor.printSetup(Serial);
    Serial.println("=======================");
    Serial.println("=======================");

    if (isSpec)
    {
        // Wait for an even interval of the logging interval to start the logging
        uint32_t now = sensor.getSystemTime();
        uint32_t secToWait = now % (logging_interval_minutes*60);
        Serial.print("Current time is ");
        Serial.println(now);
        Serial.print("Waiting ");
        Serial.print(secToWait);
        Serial.println(" seconds to begin logging at an even interval");
        delay((secToWait*1000) - 500);  // send the command half a second early

        Serial.println("Turning on Logging");
        sensor.setLoggingMode(0);
        Serial.println("Waiting for spectro::lyser to be ready after measurement.");
        delay(21000L);  // The spec is just "busy" and cannot communicate for ~21 seconds
    }

    // Wait to allow spec to take data and put it into registers
    Serial.println("Waiting for the first measurement results to be ready");
    Serial.println("This takes nearly a minute!!");
    delay(55000);
    // while (sensor.getParameterTime() == 0){};
    // Serial.println((millis() - now));
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
    status = sensor.getDeviceStatus();
    Serial.print("Current device status is: ");
    Serial.println(status, BIN);
    sensor.printDeviceStatus(status, Serial);

    Serial.print("Last sample was taken at ");
    Serial.print((unsigned long)(sensor.getParameterTime()));
    Serial.println(" seconds past Jan 1, 1970");

    // Get values one at a time
    for (int i = 1; i < sensor.getParameterCount()+1; i++)
    {
        Serial.println("----");
        Serial.print("Parameter number ");
        Serial.print(i);
        Serial.print(" is: ");
        Serial.print(sensor.getParameterName(i));
        Serial.print(" which currently has a value of ");
        Serial.print(sensor.getParameterValue(i));
        Serial.print(" ");
        Serial.print(sensor.getParameterUnits(i));
        Serial.print(" and status code ");
        uint16_t parm_status = sensor.getParameterStatus(i);
        Serial.println(parm_status, BIN);
        sensor.printParameterStatus(parm_status, Serial);
    }

    // wait until next reading
    delay(delay_ms);
}
