#ifndef HMI_MANAGER
#define HMI_MANAGER

#include <stdlib.h>
#include <Nextion.h>
#include <TimeLib.h>
#include "LoraManager.hpp"
#include "LinuxDevice.hpp"
#include "MeasurementsManager.hpp"

class HMIManager
{
private:
	// Nextion HMI Variables
	NexTimer ConfirmLoaded = NexTimer(1, 10, "tm0");

	NexDSButton ToggleRadio = NexDSButton(1, 2, "bt0");

	NexCrop StatusGPS = NexCrop(1, 3, "q0");
	NexCrop StatusNetwork = NexCrop(1, 4, "q1");
	NexCrop LinuxStatus = NexCrop(1, 9, "q2");

	NexText TextDate = NexText(1, 6, "t1");
	NexText TextClock = NexText(1, 5, "t0");

	struct
	{
		NexText rmsVoltage1 = NexText(1, 11, "t2");
		NexText rmsVoltage2 = NexText(1, 14, "t5");
		NexText rmsVoltage3 = NexText(1, 17, "t8");

		NexText rmsCurrent1 = NexText(1, 12, "t3");
		NexText rmsCurrent2 = NexText(1, 15, "t6");
		NexText rmsCurrent3 = NexText(1, 18, "t9");

		NexText realPower1 = NexText(1, 13, "t4");
		NexText realPower2 = NexText(1, 16, "t7");
		NexText realPower3 = NexText(1, 19, "t10");
	} HMIMeasurements;

	bool _linuxStatus = false;
	bool _statusGPS = false;
	bool _statusNetwork = false;
	// --------------------------- Callbacks -----------------------------------------
	static void toggleRadioCallback(void *ptr)
	{
		uint32_t value;
		HMIManager *hmi = (HMIManager *)ptr;

		hmi->ToggleRadio.getValue(&value);

		if (value < 2)
		{
			Lora.setProtocolEnabled(value);
		}

		// hmi->StatusNetwork.setPic((value == 1 ? 14 : 13));
	}

	inline void updateMeasurements()
	{
		if (Measurements.GridMeasurements.valid)
		{
			char text[32];
			sprintf(text, "%.1f", Measurements.GridMeasurements.V_L1);
			HMIMeasurements.rmsVoltage1.setText(text);
			sprintf(text, "%.1f", Measurements.GridMeasurements.V_L2);
			HMIMeasurements.rmsVoltage2.setText(text);
			sprintf(text, "%.1f", Measurements.GridMeasurements.V_L3);
			HMIMeasurements.rmsVoltage3.setText(text);

			sprintf(text, "%.1f", Measurements.GridMeasurements.I_L1);
			HMIMeasurements.rmsCurrent1.setText(text);
			sprintf(text, "%.1f", Measurements.GridMeasurements.I_L2);
			HMIMeasurements.rmsCurrent2.setText(text);
			sprintf(text, "%.1f", Measurements.GridMeasurements.I_L3);
			HMIMeasurements.rmsCurrent3.setText(text);

			sprintf(text, "%.1f", Measurements.GridMeasurements.W_L1);
			HMIMeasurements.realPower1.setText(text);
			sprintf(text, "%.1f", Measurements.GridMeasurements.W_L2);
			HMIMeasurements.realPower2.setText(text);
			sprintf(text, "%.1f", Measurements.GridMeasurements.W_L3);
			HMIMeasurements.realPower3.setText(text);
		}
	}

	inline void updateClock()
	{
		if (timeStatus() != timeNotSet)
		{
			uint32_t time = now();
			char text[16];

			sprintf(text, "%.2d/%.2d/%d\0", day(time), month(time), year(time));
			TextDate.setText(text);

			sprintf(text, "%.2d:%.2d:%.2d\0", hour(time), minute(time), second(time));
			TextClock.setText(text);
		}
	}

public:
	bool _enabled = false;

	//RTOS Options - Optional
	bool RTOS_ENABLE = true;
	uint16_t RTOS_updateTime = 100;
	uint16_t RTOS_priority = 1;

	NexTouch *_hmiEvents[2] =
	{
		&ToggleRadio,
		NULL };

	void init()
	{
		ToggleRadio.attachPop(toggleRadioCallback, this);
	}

	bool setEnable(bool value)
	{
		// Initialize IHM Mode
		_enabled = value;
		if (value)
		{
			nexInit();
		}

		return _enabled;
	}

	void update()
	{
		static uint32_t time = millis();
		if (_enabled)
		{
			if (millis() - time > 1000)
			{
				time = millis();

				updateClock();
				updateMeasurements();
				// Linux Status
				if (_linuxStatus != Linux.getInitialized())
				{
					_linuxStatus = Linux.getInitialized();
					if (_linuxStatus)
					{
						ConfirmLoaded.enable();
					}
					else
					{
						ConfirmLoaded.disable();
						LinuxStatus.setPic(13); // Disable Linux Status gif
					}
				}

				// TODO: Network, GPS
			}
		}
	}
};

#endif

extern HMIManager HMI;