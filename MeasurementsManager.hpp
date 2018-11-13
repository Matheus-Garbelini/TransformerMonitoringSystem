#ifndef MEASUREMENTS_MANAGER
#define MEASUREMENTS_MANAGER

#include <Arduino.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <unordered_map>
#include <visit_struct.hpp>
#include "LinuxDevice.hpp"
#include "ATT7022EU.hpp"

#define WAIT_FOR_GPS 0

// TODO create att7022eu class

struct MEASUREMENTS
{
	bool valid = false;
	uint32_t time = 0;

	float frequency = 60.0;

	float rmsVoltage1 = 0;
	float rmsVoltage2 = 0;
	float rmsVoltage3 = 0;

	float rmsCurrent1 = 0;
	float rmsCurrent2 = 0;
	float rmsCurrent3 = 0;

	float thdVoltage1 = 0.0;
	float thdVoltage2 = 0.0;
	float thdVoltage3 = 0.0;
	float thdCurrent1 = 0.0;
	float thdCurrent2 = 0.0;
	float thdCurrent3 = 0.0;

	// TODO: Read real,active and reactive power
	float realPower1 = 100;
	float realPower2 = 100;
	float realPower3 = 100;
};

VISITABLE_STRUCT(MEASUREMENTS,
	frequency,
	rmsVoltage1,
	rmsVoltage2,
	rmsVoltage3,
	rmsCurrent1,
	rmsCurrent2,
	rmsCurrent3,
	thdVoltage1,
	thdVoltage2,
	thdVoltage3,
	thdCurrent1,
	thdCurrent2,
	thdCurrent3);
//realPower1,
//realPower2,
//realPower3);

typedef struct {
	double average;
	float min;
	float max;
	uint32_t samples;
	uint32_t lastTime;
} INTEGRAL_STRUCTURE;

class MeasurementsManager
{
private:
	StaticJsonBuffer<1024> jsonBuffer;
	std::unordered_map<char *, INTEGRAL_STRUCTURE> IntegralData;
	ATT7022EU EnergyIC;

	void updateMeasurements()
	{
		// TODO here, read att7022 measurements
		GridMeasurements.time = now();

		GridMeasurements.frequency = EnergyIC.readFrequency();

		GridMeasurements.rmsVoltage1 = EnergyIC.readRMSVoltage(0);
		GridMeasurements.rmsVoltage2 = EnergyIC.readRMSVoltage(1);
		GridMeasurements.rmsVoltage3 = EnergyIC.readRMSVoltage(2);

		GridMeasurements.rmsCurrent1 = EnergyIC.readRMSCurrent(0);
		GridMeasurements.rmsCurrent2 = EnergyIC.readRMSCurrent(1);
		GridMeasurements.rmsCurrent3 = EnergyIC.readRMSCurrent(2);

		GridMeasurements.thdVoltage1 = EnergyIC.readTHDVoltage(0);
		GridMeasurements.thdVoltage2 = EnergyIC.readTHDVoltage(1);
		GridMeasurements.thdVoltage3 = EnergyIC.readTHDVoltage(2);
		//Serial.println("------------------");
		//EnergyIC.update();
		//updateIntegralData(IntegralData);
		//Serial.println("------------------");
	}

	void encodeJSON(JsonObject &root)
	{
		visit_struct::for_each(GridMeasurements,
			[&root](const char *name, auto &value) {
			root[name] = value;
		});
	}

	void createIntegralData(std::unordered_map<char *, INTEGRAL_STRUCTURE> &data) {
		visit_struct::for_each(GridMeasurements,
			[&](char *name, auto &value) {
			data[name] = { 0 };
			data[name].lastTime = now();
		});
	}

	// Function for 1, 5 and 15 minutes integralization
	void updateIntegralData(std::unordered_map<char *, INTEGRAL_STRUCTURE> &data) {
		visit_struct::for_each(GridMeasurements,
			[&](char *name, auto &value) {
			/*auto &entry = data[name];
			entry.samples += 1;*/
			data[name].average = value;
			//printf("%s: %f\n", name, value);
		});
	}

	void sendToLinux()
	{
#if WAIT_FOR_GPS == 1
		if (Linux.getInitialized() && timeStatus() != timeNotSet)
#else
		if (Linux.getInitialized())
#endif
		{
			// TODO: Move to Serial Manager (2nd thread)
			JsonObject &root = jsonBuffer.createObject();

			encodeJSON(root);

			root["time"] = GridMeasurements.time;
			root.printTo(Linux);
			Linux.write('\n');
			jsonBuffer.clear();
		}
	}

public:
	MEASUREMENTS GridMeasurements;

	//RTOS Options - Optional
	bool RTOS_ENABLE = true;
	uint16_t RTOS_updateTime = 50;
	uint16_t RTOS_priority = 23; // Max priority

	bool init()
	{
		EnergyIC.init();
		GridMeasurements.valid = true;
		createIntegralData(IntegralData);
		return GridMeasurements.valid;
	}

	void setSPI(SPIClass *spi) {
		EnergyIC.setSpi(spi);
	}

	void update()
	{
		static uint32_t _time = millis();

		uint32_t currentTime = millis();
		if (currentTime - _time >= 1000) // 1 second
		{
			_time = currentTime;

			updateMeasurements();
			sendToLinux();
		}
	}
};

#endif

extern MeasurementsManager Measurements;