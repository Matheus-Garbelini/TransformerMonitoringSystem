#pragma once
// Core libraries
#include <inttypes.h>
#include <Arduino.h>
#include <SPI.h>
#include <HardwareSerial.h>
#include <esp32-hal.h>
#include <stdio.h>
#include <TimeLib.h>

// User libraries
#include "RadioDevice.hpp"
#include "QueryManager.hpp"
#include "GPSManager.hpp"
#include "HMIManager.hpp"
#include "JSONParser.hpp"
#include "LinuxDevice.hpp"
#include "RS485Device.hpp"
#include "LoraManager.hpp"
#include "MeasurementsManager.hpp"
#include "ConfigFileManager.hpp"
#include "SerialManager.hpp"

// --------- Singletons --------
BoardRadioNode<ThroughLora> Radio;
QueryManagerClass QueryManager;
GPSManager GPS;
HMIManager HMI;

JSONHandler JSONManager;
LinuxManager Linux;
RS485Manager RS485;
SerialHandler SerialManager;
LoraManager Lora;
MeasurementsManager Measurements;
ConfigFileManager Config;
// ----------------------------

#define LED_PIN 2
#define LED_ON digitalWrite(LED_PIN, LOW);
#define LED_OFF digitalWrite(LED_PIN, HIGH);

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 5		   /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;

SPIClass *Spi = NULL;

void setup()
{
	//setTime(1539567224UL);
	//delay(1000);
	SerialManager.init();
	Config.init();
	GPS.begin();
	HMI.init();
	Lora.init();
	Measurements.init();

	// Print freeheap here
	Serial.print("Free Heap: ");
	Serial.println(esp_get_free_heap_size() / 1e6);

	// Spi = new SPIClass(HSPI);
	// Spi->begin();
	// pinMode(15, OUTPUT);
	// digitalWrite(15, LOW);
}

void loop()
{
	/*static uint32_t time = 0;

	if (millis() - time > 1000) {
		time = millis();

		uint32_t times = now();

		printf("%.2d/%.2d/%d\n", day(times), month(times), year(times));

		printf("%.2d:%.2d:%.2d\n", hour(times), minute(times), second(times));
	}*/

	GPS.update();
	SerialManager.update();
	HMI.update();
	Lora.update();
	Measurements.update();
}