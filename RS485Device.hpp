#ifndef RS485_DEVICE
#define RS485_DEVICE

#include <Arduino.h>

#include <ModbusSerial.h>
#include <visit_struct.hpp>
#include "MeasurementsManager.hpp"
#include "ConfigFileManager.hpp"
#include <type_traits>
#include <stdlib.h>

#define BAUDRATE_DEFAULT 115200
#define SLAVE_ID_DEFAULT 2
#define RS485_DIR_PIN 22

#define UART_RS485_NUMBER 1
#define UART_RX_PIN 26
#define UART_TX_PIN 27
#define MODBUS_DIVISION_FACTOR 100.0
#define MODBUS_HOLDING_DIVISION_FACTOR_10000 10;
#define MODBUS_UPDATE_TIME 2000 //1 second

class RS485Manager : public HardwareSerial
{
private:
	// ---- Modbus ---------
	uint8_t _slaveId = SLAVE_ID_DEFAULT;
	//Modbus _slave;
	ModbusSerial _slave;

	// ---- Parameters -----
	int _baudrate = BAUDRATE_DEFAULT;
	bool _modbusMode = true;

	template< typename T>
	static inline word isValidNumber(T value, typename std::enable_if<std::is_same<T, String>::value >::type* = 0)
	{
		return 0;
	}

	template< typename T>
	static inline word isValidNumber(T value, typename std::enable_if<!std::is_same<T, String>::value >::type* = 0)
	{
		// 		return static_cast<word>(value);
		word v = (word)value;
		if (v >= 10000) {
			v /= MODBUS_HOLDING_DIVISION_FACTOR_10000;
		}
		return v;
	}

	template< typename T>
	static inline void  commitNumber(word reg, T &value, typename std::enable_if<std::is_same<T, String>::value >::type* = 0)
	{
		// Do nothing for String
	}

	template< typename T>
	static inline void commitNumber(word reg, T &value, typename std::enable_if<!std::is_same<T, String>::value >::type* = 0)
	{
		// 		return static_cast<word>(value);

		if (reg >= 1000) {
			value = (T)reg * MODBUS_HOLDING_DIVISION_FACTOR_10000;
		}
		else
		{
			value = reg;
		}
	}

	void addInputRegisters(MeasurementsManager &measurements)
	{
		int address = 0;
		visit_struct::for_each(measurements.GridMeasurements,
			[&](const char *name, auto &value) {
			_slave.addIreg(address, (word)(value*MODBUS_DIVISION_FACTOR));
			address++;
		});
	}

	void addHoldingRegisters(ConfigFileManager &config)
	{
		int address = 0;
		visit_struct::for_each(config.Params,
			[&](const char *name, auto &value) {
			_slave.addHreg(address, (isValidNumber(value)));
			address++;
		});
	}

	void commitHoldingRegisters(ConfigFileManager &config)
	{
		int address = 0;
		visit_struct::for_each(config.Params,
			[&](const char *name, auto &value) {
			commitNumber(_slave.Hreg(address), value);
			address++;
		});
	}

	inline void updateHoldingRegisters(ConfigFileManager &config)
	{
		int address = 0;
		visit_struct::for_each(config.Params,
			[&](const char *name, auto &value) {
			_slave.Hreg(address, (isValidNumber(value)));
			address++;
		});
	}

	inline void updateInputRegisters(MeasurementsManager &measurements)
	{
		int address = 0;
		visit_struct::for_each(measurements.GridMeasurements,
			[&](const char *name, auto &value) {
			_slave.Ireg(address, (word)(value*MODBUS_DIVISION_FACTOR));
			address++;
		});
	}

public:
	RS485Manager() : HardwareSerial(UART_RS485_NUMBER)//, _slave(this, _slaveId, RS485_DIR_PIN, this)
	{
	}

	void setModbus(bool value)
	{
		_modbusMode = value;
	}

	bool getModbusMode()
	{
		return _modbusMode;
	}

	void init()
	{
#if UART_RS485_NUMBER==1
		this->begin(_baudrate, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
#else
		this->begin(_baudrate);
#endif

		_slave.config(this, _baudrate, SERIAL_8N1, RS485_DIR_PIN);

		_slave.setSlaveId(_slaveId);

		addInputRegisters(Measurements);
		addHoldingRegisters(Config);
	}

	void update()
	{
		static uint32_t time;
		if (_modbusMode)
		{
			_slave.task();

			if (millis() - time > MODBUS_UPDATE_TIME) {
				time = millis();
				updateInputRegisters(Measurements);
				if (_slave.requested())
				{
					commitHoldingRegisters(Config);
					Config.save();
				}
				else
				{
					updateHoldingRegisters(Config);
				}
			}
		}
	}
};

#endif

extern RS485Manager RS485;