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
// Yosemitech ships sensors with a default ID of 0x01.

// Define pin number variables
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text

// Construct the Yosemitech modbus instance
modbusMaster sensor;

// Define config for Serial.begin(baud, config);
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
    sensor.begin(modbusAddress, &Serial1, DEREPin);
    sensor.setDebugStream(&Serial);

    uint16_t baudrates[] = {1200, 9600, 19200, 38400, 57600};
    uint8_t configs[] = {SERIAL_8N1, SERIAL_8N2, SERIAL_8O1, SERIAL_8E1};

    bool success = false;

    for (byte k = 0x00; k < 0x0F; k++)
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
                success = sensor.uint16ToRegister(4, 1, bigEndian);
                Serial1.end();
                Serial.println("=======================");
                if (success) break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{}
