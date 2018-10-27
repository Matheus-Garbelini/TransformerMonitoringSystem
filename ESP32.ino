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

// Definitions
#define LED_PIN 2
#define LED_ON digitalWrite(LED_PIN, LOW);
#define LED_OFF digitalWrite(LED_PIN, HIGH);

#define CS_PIN 15
#define CLK_PIN 14
#define DATA_PIN 13
#define DIN_PIN 12

#define SetInput(x) pinMode(x,INPUT)
#define SetOutput(x) pinMode(x,OUTPUT)

#define Set_CS digitalWrite(CS_PIN,HIGH)
#define Clr_CS digitalWrite(CS_PIN,LOW)

#define Set_SCL digitalWrite(CLK_PIN,HIGH)
#define Clr_SCL digitalWrite(CLK_PIN,LOW)

#define Set_DATA digitalWrite(DATA_PIN,HIGH)
#define Clr_DATA digitalWrite(DATA_PIN,LOW)

#define Rd_MISO digitalRead(DIN_PIN)

#define delay_us delayMicroseconds

// --------- Global Classes --------
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

SPIClass *Spi = NULL;

uint32_t SPI_ATT_Read(uint8_t data)
{
	uint8_t i;
	uint32_t temp = 0;
	Set_CS;
	Clr_SCL;
	Clr_CS;
	for (i = 0; i < 8; i++)
	{
		Set_SCL;
		delay_us(50);
		if (data & 0x80)
			Set_DATA;
		else
			Clr_DATA;
		delay_us(3);
		Clr_SCL;
		delay_us(50);
		data <<= 1;
	}
	delay_us(3);
	for (i = 0; i < 24; i++)
	{
		temp <<= 1;
		Set_SCL;
		delay_us(50);
		if (Rd_MISO)
			temp |= 0x01;
		Clr_SCL;
		delay_us(50);
	}
	Set_CS;
	return (temp);
}

void SPI_ATT_Write(uint8_t com_add, uint32_t data2)
{
	uint8_t i, data1;
	data1 = 0x80 | com_add;
	Set_CS;
	Clr_SCL;
	Clr_CS;
	for (i = 0; i < 8; i++)
	{
		Set_SCL;
		delay_us(50);
		if (data1 & 0x80)
			Set_DATA;
		else
			Clr_DATA;
		delay_us(3);
		Clr_SCL;
		delay_us(50);
		data1 <<= 1;
	}
	for (i = 0; i < 24; i++)
	{
		Set_SCL;
		delay_us(50);
		if (data2 & 0x00800000)
			Set_DATA;
		else
			Clr_DATA;
		delay_us(3);
		Clr_SCL;
		delay_us(50);

		data2 <<= 1;
	}
	Set_CS;
}

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
			xTaskCreate([&](void *p) {
				T rtosObject = _taskObject;
				TickType_t time = xTaskGetTickCount();
				while (true)
				{
					rtosObject->update();
					vTaskDelayUntil(&time, pdMS_TO_TICKS(rtosObject->RTOS_updateTime));
				}
			}, name, 10000, NULL, value->RTOS_priority, NULL);
		}
	}
};

void setup()
{
	/*SerialManager.init();
	Config.init();
	GPS.init();
	HMI.init();
	Lora.init();
	Measurements.init();*/

	/*SetOutput(23);
	digitalWrite(23, HIGH);
	SetInput(DIN_PIN);
	SetOutput(CS_PIN);
	SetOutput(CLK_PIN);
	SetOutput(DATA_PIN);
	Clr_SCL;
	Clr_CS;
	delay_us(30);
	Set_CS;
	delay_us(600);*/

	visit_struct::for_each(MainTasks, TaskInitializer{});

	visit_struct::for_each(MainTasks, CreateTaskUpdater{});

	/*
		xTaskCreate([](void *p) {
			TickType_t time = xTaskGetTickCount();
			while (true)
			{
				GPS.update();
				vTaskDelayUntil(&time, pdMS_TO_TICKS(10));
			}
		}, "GPSManager", 10000, NULL, 2, NULL);

		xTaskCreate([](void *p) {
			TickType_t time = xTaskGetTickCount();
			while (true)
			{
				SerialManager.update();
				vTaskDelayUntil(&time, pdMS_TO_TICKS(5));
			}
		}, "SerialManager", 10000, NULL, 3, NULL);

		xTaskCreate([](void *p) {
			TickType_t time = xTaskGetTickCount();
			while (true)
			{
				HMI.update();
				vTaskDelayUntil(&time, pdMS_TO_TICKS(10));
			}
		}, "HMIManager", 10000, NULL, 2, NULL);

		xTaskCreate([](void *p) {
			TickType_t time = xTaskGetTickCount();
			while (true)
			{
				Lora.update();
				vTaskDelayUntil(&time, pdMS_TO_TICKS(20));
			}
		}, "LoRaManager", 10000, NULL, 2, NULL);

		xTaskCreate([](void *p) {
			TickType_t time = xTaskGetTickCount();
			while (true)
			{
				Measurements.update();
				vTaskDelayUntil(&time, pdMS_TO_TICKS(5));
			}
		}, "Measurements", 10000, NULL, 4, NULL);*/
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

	//GPS.update();
	//SerialManager.update();
	/*HMI.update();
	Lora.update();
	Measurements.update();*/

	vTaskDelete(NULL);
}