/*****************************************************************************
resetSettings.ino

This attempts to send a reset request to a spectro::lyzer by sending a "broadcast"
(address 0) send to it to set register 4 (reset settings) trying all of the
various likely baudrate and parity combinations for the spec.
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <SensorModbusMaster.h>

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define the sensor's modbus address
byte modbusAddress = 0x00;  // The sensor's modbus address, or SlaveID

// Define the button you will press to begin the program
const uint8_t buttonPin = 21;

// Define pin number variables
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text

// Construct the modbus instance
modbusMaster sensor;

// Define config for Serial.begin(baud, config);
String parseSerialMode(int mode);
#define SERIAL_5N1 0x00
#define SERIAL_6N1 0x02
#define SERIAL_7N1 0x04
#define SERIAL_8N1 0x06
#define SERIAL_5N2 0x08
#define SERIAL_6N2 0x0A
#define SERIAL_7N2 0x0C
#define SERIAL_8N2 0x0E
#define SERIAL_5E1 0x20
#define SERIAL_6E1 0x22
#define SERIAL_7E1 0x24
#define SERIAL_8E1 0x26
#define SERIAL_5E2 0x28
#define SERIAL_6E2 0x2A
#define SERIAL_7E2 0x2C
#define SERIAL_8E2 0x2E
#define SERIAL_5O1 0x30
#define SERIAL_6O1 0x32
#define SERIAL_7O1 0x34
#define SERIAL_8O1 0x36
#define SERIAL_5O2 0x38
#define SERIAL_6O2 0x3A
#define SERIAL_7O2 0x3C
#define SERIAL_8O2 0x3E

// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{
    if (DEREPin > 0) pinMode(DEREPin, OUTPUT);

    Serial.begin(57600);  // Main serial port for debugging via USB Serial Monitor

    // Turn on debugging
    // sensor.setDebugStream(&Serial);

    // Start up note
    Serial.println("S::CAN Spect::lyzer Search Utility");
    Serial.println("Please note that this may take several minutes to run!");

    // Allow the sensor and converter to warm up
    if (buttonPin > 0)
    {
        Serial.print("Search will begin after pushing the button connected to pin ");
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

    uint16_t baudrates[] = {38400, 9600, 19200, 57600, 1200, 2400, 4800};
    uint8_t configs[] = {SERIAL_8O1, SERIAL_8N1, SERIAL_8N2, SERIAL_8E1};

    bool foundMe = false;
    for (byte k = 0x01; k < 0x0F; k++)
    {
        sensor.begin(k, &Serial1, DEREPin);
        sensor.setDebugStream(&Serial);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                Serial.print("Current modbus address: 0x0");
                Serial.println(k, HEX);
                Serial.print("Current baud rate: ");
                Serial.println(baudrates[i]);
                Serial.print("Current configuration: 0x");
                Serial.println(configs[j], HEX);
                Serial1.begin(baudrates[i], configs[j]);  // port for communicating with sensor
                int address = sensor.uint16FromRegister(0x03, 0x00, bigEndian);
                if (address > 0) {
                    foundMe = true;
                    Serial.println("Sensor replied!");
                    Serial.println("=======================");
                    Serial.print("******Current modbus address: 0x0");
                    Serial.println(k, HEX);
                    Serial.print("******Current baud rate: ");
                    Serial.println(baudrates[i]);
                    Serial.print("******Current configuration: ");
                    Serial.println(parseSerialMode(configs[j]));
                    Serial.println("=======================");
                    break;
                }
                if (foundMe) break;
                Serial1.end();
                Serial.println("=======================");
                delay(10);
            }
            if (foundMe) break;
        }
        if (foundMe) break;
    }
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{}


String parseSerialMode(int mode)
{
    switch (mode)
    {
        case SERIAL_5N1: return "5N1 - 5 data bits, no parity, 1 stop bit"; break;
        case SERIAL_6N1: return "6N1 - 6 data bits, no parity, 1 stop bit"; break;
        case SERIAL_7N1: return "7N1 - 7 data bits, no parity, 1 stop bit"; break;
        case SERIAL_8N1: return "8N1 - 8 data bits, no parity, 1 stop bit"; break;
        case SERIAL_5N2: return "5N2 - 5 data bits, no parity, 2 stop bits"; break;
        case SERIAL_6N2: return "6N2 - 6 data bits, no parity, 2 stop bits"; break;
        case SERIAL_7N2: return "7N2 - 7 data bits, no parity, 2 stop bits"; break;
        case SERIAL_8N2: return "8N2 - 8 data bits, no parity, 2 stop bits"; break;
        case SERIAL_5E1: return "5E1 - 5 data bits, even parity, 1 stop bit"; break;
        case SERIAL_6E1: return "6E1 - 6 data bits, even parity, 1 stop bit"; break;
        case SERIAL_7E1: return "7E1 - 7 data bits, even parity, 1 stop bit"; break;
        case SERIAL_8E1: return "8E1 - 8 data bits, even parity, 1 stop bit"; break;
        case SERIAL_5E2: return "5E2 - 5 data bits, even parity, 2 stop bits"; break;
        case SERIAL_6E2: return "6E2 - 6 data bits, even parity, 2 stop bits"; break;
        case SERIAL_7E2: return "7E2 - 7 data bits, even parity, 2 stop bits"; break;
        case SERIAL_8E2: return "8E2 - 8 data bits, even parity, 2 stop bits"; break;
        case SERIAL_5O1: return "5O1 - 5 data bits, odd parity, 1 stop bit"; break;
        case SERIAL_6O1: return "6O1 - 6 data bits, odd parity, 1 stop bit"; break;
        case SERIAL_7O1: return "7O1 - 7 data bits, odd parity, 1 stop bit"; break;
        case SERIAL_8O1: return "8O1 - 8 data bits, odd parity, 1 stop bit"; break;
        case SERIAL_5O2: return "5O2 - 5 data bits, odd parity, 2 stop bits"; break;
        case SERIAL_6O2: return "6O2 - 6 data bits, odd parity, 2 stop bits"; break;
        case SERIAL_7O2: return "7O2 - 7 data bits, odd parity, 2 stop bits"; break;
        case SERIAL_8O2: return "8O2 - 8 data bits, odd parity, 2 stop bits"; break;
        default: return ""; break;
    }
}
