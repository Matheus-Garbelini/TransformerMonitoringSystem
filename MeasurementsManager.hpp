#ifndef MEASUREMENTS_MANAGER
#define MEASUREMENTS_MANAGER
#include <limits>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <unordered_map>
#include <visit_struct.hpp>
#include "LinuxDevice.hpp"
#include "ATT7022EU.hpp"

#define WAIT_FOR_GPS 0
#define MINUTE_SECONDS (60*1000) // 60 Seconds in milliseconds

// JSON Fields
#define JSON_CMD "CMD"
#define JSON_CMD_MEASURE "measure"
#define JSON_CMD_INTEGRAL "integral"
#define JSON_CMD_EVENT "event"

// TODO create att7022eu class

struct MEASUREMENTS
{
	bool valid = false;
	uint32_t time = 0;

	float Frequency = 60.0;

	float V_L1 = 0;
	float V_L2 = 0;
	float V_L3 = 0;

	float I_L1 = 0;
	float I_L2 = 0;
	float I_L3 = 0;

	float A_N;

	float THD_V_L1N = 0.0;
	float THD_V_L2N = 0.0;
	float THD_V_L3N = 0.0;
	float THD_I_L1N = 0.0;
	float THD_I_L2N = 0.0;
	float THD_I_L3N = 0.0;

	float VA_L1;
	float VA_L2;
	float VA_L3;

	// TODO: Read real,active and reactive power
	float W_L1 = 100;
	float W_L2 = 100;
	float W_L3 = 100;

	float VAR_L1;
	float VAR_L2;
	float VAR_L3;

	float PF_L1;
	float PF_L2;
	float PF_L3;
};

VISITABLE_STRUCT(MEASUREMENTS,
	Frequency,
	V_L1,
	V_L2,
	V_L3,
	I_L1,
	I_L2,
	I_L3,
	A_N,
	THD_V_L1N,
	THD_V_L2N,
	THD_V_L3N,
	THD_I_L1N,
	THD_I_L2N,
	THD_I_L3N,
	VA_L1,
	VA_L2,
	VA_L3,
	W_L1,
	W_L2,
	W_L3,
	VAR_L1,
	VAR_L2,
	VAR_L3,
	PF_L1,
	PF_L2,
	PF_L3
);

struct VARIABLE_STRUCTURE {
	uint32_t timeout;
	double average;
	float min;
	float max;
	uint32_t samples;
	uint32_t lastTime;
};

struct INTEGRAL_STRUCTURE {
	VARIABLE_STRUCTURE minutes_1 = { MINUTE_SECONDS };
	VARIABLE_STRUCTURE minutes_5 = { MINUTE_SECONDS * 5 };
	VARIABLE_STRUCTURE minutes_15 = { MINUTE_SECONDS * 15 };
};

VISITABLE_STRUCT(INTEGRAL_STRUCTURE,
	minutes_1,
	minutes_5,
	minutes_15);

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

		GridMeasurements.Frequency = EnergyIC.readFrequency();

		GridMeasurements.V_L1 = EnergyIC.readRMSVoltage(0);
		GridMeasurements.V_L2 = EnergyIC.readRMSVoltage(1);
		GridMeasurements.V_L3 = EnergyIC.readRMSVoltage(2);

		GridMeasurements.I_L1 = EnergyIC.readRMSCurrent(0);
		GridMeasurements.I_L2 = EnergyIC.readRMSCurrent(1);
		GridMeasurements.I_L3 = EnergyIC.readRMSCurrent(2);

		GridMeasurements.A_N = EnergyIC.readRMSNeutral();

		GridMeasurements.THD_V_L1N = EnergyIC.readTHDVoltage(0);
		GridMeasurements.THD_V_L2N = EnergyIC.readTHDVoltage(1);
		GridMeasurements.THD_V_L3N = EnergyIC.readTHDVoltage(2);

		GridMeasurements.THD_I_L1N = EnergyIC.readTHDCurrent(0);
		GridMeasurements.THD_I_L2N = EnergyIC.readTHDCurrent(1);
		GridMeasurements.THD_I_L3N = EnergyIC.readTHDCurrent(2);

		GridMeasurements.VA_L1 = EnergyIC.readPowerVA(0);
		GridMeasurements.VA_L2 = EnergyIC.readPowerVA(1);
		GridMeasurements.VA_L3 = EnergyIC.readPowerVA(2);

		GridMeasurements.W_L1 = EnergyIC.readPowerReal(0);
		GridMeasurements.W_L2 = EnergyIC.readPowerReal(1);
		GridMeasurements.W_L3 = EnergyIC.readPowerReal(2);

		GridMeasurements.VAR_L1 = EnergyIC.readPowerVAR(0);
		GridMeasurements.VAR_L2 = EnergyIC.readPowerVAR(1);
		GridMeasurements.VAR_L3 = EnergyIC.readPowerVAR(2);

		GridMeasurements.PF_L1 = EnergyIC.readPowerFactor(0);
		GridMeasurements.PF_L2 = EnergyIC.readPowerFactor(1);
		GridMeasurements.PF_L3 = EnergyIC.readPowerFactor(2);

		updateIntegralData(IntegralData);
	}

	void encodeJSON(JsonObject &root)
	{
		visit_struct::for_each(GridMeasurements,
			[&root](const char *name, auto &value) {
			root[name] = value;
		});
	}

	void createIntegralData(std::unordered_map<char *, INTEGRAL_STRUCTURE> &data) {
		uint32_t current_time = millis();

		visit_struct::for_each(GridMeasurements,
			[&](char *nameVar, auto &value) {
			data[nameVar] = {}; // Create empty VARIABLE_STRUCTURE
			auto &object = data[nameVar];

			visit_struct::for_each(object,
				[&](char *name, auto &value) {
				value.lastTime = current_time;
				value.max = std::numeric_limits<float>::lowest();
				value.min = std::numeric_limits<float>::max();
			});
		});
	}

	// Function for 1, 5 and 15 minutes integralization
	void updateIntegralData(std::unordered_map<char *, INTEGRAL_STRUCTURE> &data) {
		uint32_t current_time = millis();

		uint32_t current_sync_time = now();
		// Iterate measure variables

		visit_struct::for_each(GridMeasurements,
			[&](char *nameVar, auto &valueMeas) {
			auto &object = data[nameVar];

			// Iterate variable integral data for 1, 5 and 15 minutes
			visit_struct::for_each(object,
				[&](char *nameMinute, auto &valueObject) {
				valueObject.samples += 1;
				valueObject.average += valueMeas;

				if (valueMeas >= valueObject.max) valueObject.max = valueMeas;
				if (valueMeas <= valueObject.min) valueObject.min = valueMeas;

				if (current_time - valueObject.lastTime >= valueObject.timeout) {
					valueObject.lastTime = current_time;
					valueObject.average /= (double)valueObject.samples;
					//TODO: change now() to millis and decrement 1 second for correct start

					sendIntegralData(nameVar, valueObject, current_sync_time - 60);
					// Clear integralization data
					valueObject.samples = 0;
					valueObject.average = 0;
					valueObject.max = std::numeric_limits<float>::lowest();
					valueObject.min = std::numeric_limits<float>::max();
				}
			});
		});
	}

	static void sendIntegralData(const char * name, VARIABLE_STRUCTURE &variable, uint32_t updateTime) {
		DynamicJsonBuffer jsonBuffer;
		JsonObject &root = jsonBuffer.createObject();
		char text[128];
		root[JSON_CMD] = JSON_CMD_INTEGRAL;
		root["name"] = name;
		root["average"] = variable.average;
		root["max"] = variable.max;
		root["min"] = variable.min;
		root["period"] = variable.timeout;
		root["time"] = updateTime;

		// TODO: route to Linux
		if (Linux.getInitialized()) {
			root.printTo(Linux);
			Linux.print("\n");
		}

		uint32_t size = root.printTo(text);
		LoRa.beginPacket();
		LoRa.write((uint8_t *)text, size);
		LoRa.write('\n');
		LoRa.endPacket();
		LoRa.receive();

		jsonBuffer.clear();
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

			root[JSON_CMD] = JSON_CMD_MEASURE;

			encodeJSON(root);

			root["time"] = GridMeasurements.time;
			root.printTo(Linux);
			Linux.print("\n");

			if (dump_serial) {
				root.prettyPrintTo(Serial);
				Serial.print("\n");
			}

			jsonBuffer.clear();
		}
	}

public:
	MEASUREMENTS GridMeasurements;

	//RTOS Options - Optional
	bool RTOS_ENABLE = true;
	uint16_t RTOS_updateTime = 50;
	uint16_t RTOS_priority = 23; // Max priority
	bool dump_serial = false;

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
			sendToLinux();
		}

		updateMeasurements();
	}
};

#endif

extern MeasurementsManager Measurements;