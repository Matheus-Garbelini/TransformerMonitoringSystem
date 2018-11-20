#ifndef GPS_MANAGER
#define GPS_MANAGER

#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <soc/rtc.h>
#include <TimeLib.h>
#include <sys/time.h>
#include "HMIManager.hpp"

#define SYNC_TIMEOUT 60000

#define CALIBRATE_ONE(cali_clk) calibrate_one(cali_clk, #cali_clk)

static uint32_t calibrate_one(rtc_cal_sel_t cal_clk, const char *name)
{
	const uint32_t cal_count = 1000;
	//const float factor = (1 << 19) * 1000.0f;
	uint32_t cali_val;
	// printf("%s:\n", name);

	for (int i = 0; i < 5; ++i)
	{
		// printf("calibrate (%d): ", i);
		cali_val = rtc_clk_cal(cal_clk, cal_count);
		//printf("%.3f kHz\n", factor / (float)cali_val);
	}
	return cali_val;
}

// Sync control variable
static volatile uint32_t _syncTime = 0;
//static uint32_t utcOffset = (-3 * 3600);
static uint32_t utcOffset = 0; // Use UTC

class GPSManager
{
private:
	int _baudrate = 9600;
	bool _debugGPS = false;
	bool _nmeaMode = false;
	const uint8_t _rx_pin = 19;
	const uint8_t _tx_pin = 18;
	const uint8_t _pps_pin = 35;
	uint32_t lastSyncTime = 0;

	SoftwareSerial _serialGPS;
	TinyGPSPlus _GPS;

	static void ppsInterrupt()
	{
		if (_syncTime)
		{
			setTime(_syncTime + 1); // Sync time to the PPS pulse after the NMEA received at _syncTime
			adjustTime(utcOffset);
			_syncTime = 0;// sync Time library
		}
	}

public:
	//RTOS Options - Optional
	bool RTOS_ENABLE = true;
	uint16_t RTOS_updateTime = 5;
	uint16_t RTOS_priority = 2;

	// --------------------------
	bool valid = false;
	double latitude = 0.0;
	double longitude = 0.0;

	GPSManager() : _serialGPS(_rx_pin, _tx_pin, false, 256)
	{
	}

	void init()
	{
		// ------------- Start RTC Clock and Calibration --------------
		rtc_clk_32k_bootstrap(512);
		rtc_clk_32k_bootstrap(512);
		rtc_clk_32k_enable(true);

		uint32_t cal_32k = CALIBRATE_ONE(RTC_CAL_32K_XTAL);
		rtc_clk_slow_freq_set(RTC_SLOW_FREQ_32K_XTAL);

		if (cal_32k == 0)
		{
			Serial.println("32K XTAL OSC has not started up");
		}
		else
		{
			Serial.println("32K XTAL OSC Calibrated");
		}

		struct timeval now = { 0 };
		settimeofday(&now, NULL);

		// ------------- Start GPS Serial -------------
		_serialGPS.begin(_baudrate);
		pinMode(_pps_pin, INPUT_PULLDOWN);
		attachInterrupt(_pps_pin, ppsInterrupt, RISING);
	}

	void setDebug(bool value)
	{
		_debugGPS = value;
	}

	void setNMEAMode(bool value)
	{
		_nmeaMode = value;
	}

	void syncGPS()
	{
		if (millis() - lastSyncTime > SYNC_TIMEOUT || (valid == false))
		{
			lastSyncTime = millis();

			if (_syncTime == 0)
			{
				if (_GPS.date.isValid() && _GPS.time.isValid())
				{
					TimeElements tm;
					tm.Year = _GPS.date.year() - 1970;
					tm.Month = _GPS.date.month();
					tm.Day = _GPS.date.day();
					tm.Hour = _GPS.time.hour();
					tm.Minute = _GPS.time.minute();
					tm.Second = _GPS.time.second();

					if (_GPS.date.year() > 2000)
					{
						_syncTime = makeTime(tm);

						valid = true;
					}
				}
				if (_GPS.location.isValid())
				{
					latitude = _GPS.location.lat();
					longitude = _GPS.location.lng();
				}
			}
		}
	}

	void update()
	{
		static uint32_t time = millis();

		if (!HMI._enabled)
		{
			while (_serialGPS.available())
			{
				if (_nmeaMode)
				{
					Serial.write(_serialGPS.read());
				}
				else if (_GPS.encode(_serialGPS.read())) {
					syncGPS();
					if (_debugGPS)
					{
						if (millis() - time > 1000)
						{
							time = millis();
							printGps();
						}
					}
				}
			}
		}
	}

	void printGps()
	{
		Serial.print(F("Location: "));
		if (_GPS.location.isValid())
		{
			Serial.print(_GPS.location.lat(), 6);
			Serial.print(F(","));
			Serial.print(_GPS.location.lng(), 6);
		}
		else
		{
			Serial.print(F("INVALID"));
		}

		Serial.print(F("  Date/Time: "));
		if (_GPS.date.isValid())
		{
			Serial.print(_GPS.date.month());
			Serial.print(F("/"));
			Serial.print(_GPS.date.day());
			Serial.print(F("/"));
			Serial.print(_GPS.date.year());
		}
		else
		{
			Serial.print(F("INVALID"));
		}

		Serial.print(F(" "));
		if (_GPS.time.isValid())
		{
			if (_GPS.time.hour() < 10)
				Serial.print(F("0"));
			Serial.print(_GPS.time.hour());
			Serial.print(F(":"));
			if (_GPS.time.minute() < 10)
				Serial.print(F("0"));
			Serial.print(_GPS.time.minute());
			Serial.print(F(":"));
			if (_GPS.time.second() < 10)
				Serial.print(F("0"));
			Serial.print(_GPS.time.second());
			Serial.print(F("."));
			if (_GPS.time.centisecond() < 10)
				Serial.print(F("0"));
			Serial.print(_GPS.time.centisecond());
		}
		else
		{
			Serial.print(F("INVALID"));
		}

		Serial.println();
	}
};

#endif

extern GPSManager GPS;