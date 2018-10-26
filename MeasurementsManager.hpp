#ifndef MEASUREMENTS_MANAGER
#define MEASUREMENTS_MANAGER

#include <Arduino.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <visit_struct.hpp>
#include "LinuxDevice.hpp"

#define WAIT_FOR_GPS 1

// TODO create att7022eu class

struct MEASUREMENTS
{
	bool valid = true;
	uint32_t time = 0;

	float rmsVoltage1 = 127.3;
	float rmsVoltage2 = 127.3;
	float rmsVoltage3 = 127.3;

	float rmsCurrent1 = 1.3;
	float rmsCurrent2 = 1.3;
	float rmsCurrent3 = 1.3;

	float realPower1 = 100;
	float realPower2 = 100;
	float realPower3 = 100;
};

VISITABLE_STRUCT(MEASUREMENTS,
	valid,
	rmsVoltage1,
	rmsVoltage2,
	rmsVoltage3,
	rmsCurrent1,
	rmsCurrent2,
	rmsCurrent3,
	realPower1,
	realPower2,
	realPower3);

class MeasurementsManager
{
private:
	StaticJsonBuffer<512> jsonBuffer;

	void updateMeasurements()
	{
		// TODO here, read att7022 measurements
		GridMeasurements.time = now();
		float voltageRandom = GridMeasurements.rmsVoltage1;
		GridMeasurements.rmsVoltage1 = voltageRandom + (float)(random(100) - 5) / 10.0;
		GridMeasurements.rmsVoltage2 = voltageRandom + (float)(random(100) - 5) / 10.0;
		GridMeasurements.rmsVoltage3 = voltageRandom + (float)(random(100) - 5) / 10.0;

		float currentRandom = GridMeasurements.rmsCurrent1;
		GridMeasurements.rmsCurrent1 = currentRandom + (float)(random(100) - 5) / 10.0;
		GridMeasurements.rmsCurrent2 = currentRandom + (float)(random(100) - 5) / 10.0;
		GridMeasurements.rmsCurrent3 = currentRandom + (float)(random(100) - 5) / 10.0;

		float realPowerRandom = GridMeasurements.realPower1;
		GridMeasurements.realPower1 = realPowerRandom + (float)(random(100) - 5) / 10.0;
		GridMeasurements.realPower2 = realPowerRandom + (float)(random(100) - 5) / 10.0;
		GridMeasurements.realPower3 = realPowerRandom + (float)(random(100) - 5) / 10.0;
	}

	void encodeJSON(JsonObject &root)
	{
		visit_struct::for_each(GridMeasurements,
			[&root](const char *name, auto &value) {
			root[name] = value;
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

			root.printTo(Linux);
			Linux.write('\n');
			jsonBuffer.clear();
		}
	}

public:
	MEASUREMENTS GridMeasurements;

	bool init()
	{
		// TODO: Initialize att7022eu
		return true;
	}

	void update()
	{
		static uint32_t time = millis();

		if (millis() - time > 1000)
		{
			uint32_t times = now();

			time = millis();

			updateMeasurements();
			sendToLinux();
		}
	}
};

#endif

extern MeasurementsManager Measurements;