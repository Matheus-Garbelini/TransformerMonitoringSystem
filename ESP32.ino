#pragma once
// Core libraries
#include <inttypes.h>
#include <Arduino.h>
#include <SPI.h>
#include <HardwareSerial.h>
#include <esp32-hal.h>
#include <stdio.h>
#include <TimeLib.h>
#include <visit_struct.hpp>
#include <string>

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

// --------- Global Classes --------
uint8_t radioId = 2;
BoardRadioNode<ThroughLora> Radio(radioId);
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

// -------- Instaces ----------
SPIClass *SpiHardware;

struct MAIN_TASKS {
	SerialHandler *SerialTask = &SerialManager;
	ConfigFileManager *ConfigTask = &Config;
	GPSManager *GPSTask = &GPS;
	HMIManager *HMITask = &HMI;
	LoraManager *LoraTask = &Lora;
	MeasurementsManager *MeasureTask = &Measurements;
} MainTasks;

VISITABLE_STRUCT(MAIN_TASKS,
	SerialTask,
	ConfigTask,
	GPSTask,
	HMITask,
	LoraTask,
	MeasureTask);

struct TaskInitializer {
	template <typename T>
	void operator()(const char * name, T &value) {
		value->init();
	}
};

struct CreateTaskUpdater {
	template <typename T>
	void operator()(const char * name, T &value) {
		static T _taskObject;
		_taskObject = value;
		if (value->RTOS_ENABLE) {
			//tskNO_AFFINITY
			xTaskCreatePinnedToCore([&](void *p) {
				T rtosObject = _taskObject;
				TickType_t time = xTaskGetTickCount();
				while (true)
				{
					rtosObject->update();
					vTaskDelayUntil(&time, pdMS_TO_TICKS(rtosObject->RTOS_updateTime));
				}
			}, name, 10000, NULL, value->RTOS_priority, NULL, tskNO_AFFINITY);
		}
	}
};

void setup()
{
	SpiHardware = new SPIClass(HSPI);
	SpiHardware->begin();

	// Configure SPI for Lora and ATT7022EU
	Lora.setSPI(SpiHardware);
	Measurements.setSPI(SpiHardware);

	// Initialize all classes
	visit_struct::for_each(MainTasks, TaskInitializer{});

	// Create freeRtos tasks for each class
	visit_struct::for_each(MainTasks, CreateTaskUpdater{});
}

void loop()
{
	vTaskDelete(NULL);
}