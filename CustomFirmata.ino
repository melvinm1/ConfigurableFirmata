#include <ConfigurableFirmata.h>

// Use this to enable WIFI instead of serial communication. Tested on ESP32, but should also
// work with Wifi-enabled Arduinos
//#define ENABLE_WIFI

// const char* ssid     = "your-ssid";
// const char* password = "your-password";
// const int NETWORK_PORT = 27016;

// Note that the SERVO module currently is not supported on ESP32. So either disable this or patch the library
#ifndef ESP32
#define ENABLE_SERVO 
#endif

// Currently supported for AVR and ESP32
#if defined (ESP32) || defined (ARDUINO_ARCH_AVR)
#define ENABLE_SLEEP
#endif

#include <DigitalInputFirmata.h>
DigitalInputFirmata digitalInput;
#include <DigitalOutputFirmata.h>
DigitalOutputFirmata digitalOutput;

#ifdef ENABLE_SLEEP
#include "ArduinoSleep.h"
ArduinoSleep sleeper(39, 0);
#endif

#include <AnalogInputFirmata.h>
AnalogInputFirmata analogInput;
#include <AnalogOutputFirmata.h>
AnalogOutputFirmata analogOutput;

#ifdef ENABLE_WIFI
#include <WiFi.h>
#include "utility/WiFiClientStream.h"
#include "utility/WiFiServerStream.h"
WiFiServerStream serverStream(NETWORK_PORT);
#endif

#include <FirmataExt.h>
FirmataExt firmataExt;

#ifdef ENABLE_SERVO
#include <Servo.h>
#include <ServoFirmata.h>
ServoFirmata servo;
#endif

#include <FirmataReporting.h>
FirmataReporting reporting;

#include <Encoder.h>
#include <FirmataEncoder.h>
FirmataEncoder encoder;

void systemResetCallback()
{
// Does more harm than good on ESP32 (because may touch pins reserved
// for memory IO and other reserved functions)
#ifndef ESP32 
	for (byte i = 0; i < TOTAL_PINS; i++) 
	{
		if (IS_PIN_ANALOG(i)) 
		{
			Firmata.setPinMode(i, PIN_MODE_ANALOG);
		} 
		else if (IS_PIN_DIGITAL(i)) 
		{
			Firmata.setPinMode(i, PIN_MODE_OUTPUT);
		}
	}
#endif
	firmataExt.reset();
}

void initTransport()
{
	// Uncomment to save a couple of seconds by disabling the startup blink sequence.
	// Firmata.disableBlinkVersion();
  
#ifdef ESP8266
	// need to ignore pins 1 and 3 when using an ESP8266 board. These are used for the serial communication.
	Firmata.setPinMode(1, PIN_MODE_IGNORE);
	Firmata.setPinMode(3, PIN_MODE_IGNORE);
#endif
#ifdef ENABLE_WIFI
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	pinMode(VERSION_BLINK_PIN, OUTPUT);
	bool pinIsOn = false;
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(100);
		pinIsOn = !pinIsOn;
		digitalWrite(VERSION_BLINK_PIN, pinIsOn);
	}
	Firmata.begin(serverStream);
	Firmata.blinkVersion(); // Because the above doesn't do it.
#else 
	Firmata.begin(57600);
#endif
}

void initFirmata()
{
	firmataExt.addFeature(digitalInput);
	firmataExt.addFeature(digitalOutput);
	firmataExt.addFeature(analogInput);
	firmataExt.addFeature(analogOutput);
#ifdef ENABLE_SERVO
	firmataExt.addFeature(servo);
#endif
  firmataExt.addFeature(reporting);
#ifdef ENABLE_SLEEP
	firmataExt.addFeature(sleeper);
#endif
	firmataExt.addFeature(encoder);

	Firmata.attach(SYSTEM_RESET, systemResetCallback);
}

void setup()
{
	// Set firmware name and version.
	// Do this before initTransport(), because some client libraries expect that a reset sends this automatically.
	Firmata.setFirmwareNameAndVersion("CustomFirmata", FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
	initTransport();
	initFirmata();

	Firmata.parse(SYSTEM_RESET);
}

void loop()
{
	while(Firmata.available()) 
	{
		Firmata.processInput();
		if (!Firmata.isParsingMessage()) 
		{
			break;
		}
	}
	firmataExt.report(reporting.elapsed());
#ifdef ENABLE_WIFI
	serverStream.maintain();
#endif
}
