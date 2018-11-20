#ifndef SERIAL_MANAGER
#define SERIAL_MANAGER

#include <HardwareSerial.h>
#include <Nextion.h>
#include "JSONParser.hpp"
#include "RS485Device.hpp"

#define SERIAL_RX_BUFFER 256
#define SERIAL_RX_TIMEOUT_MS 50
#define SERIAL1_DEFAULT_BAUD 115200

#define NEX_RET_EVENT_TOUCH_HEAD (0x65)

class SerialHandler
{
private:
	// USART1 (USB/HMI)
	int _serial1Baud = SERIAL1_DEFAULT_BAUD;
	String _serial1Buffer = "";
	uint32_t _serial1Timeout = 0;
	char _hmiBuffer[8];

	// LINUX (OrangePiZero)

	String _linuxSerialBuffer = "";
	uint32_t _LinuxTimeout = 0;
	bool _enabledLinuxSerial = true;

	// RS485 Serial
	String _RS485Buffer = "";
	uint32_t _RS485Timeout = 0;
	bool _enabledRS485 = true;

public:
	//RTOS Options - Optional
	bool RTOS_ENABLE = true;
	uint16_t RTOS_updateTime = 5;
	uint16_t RTOS_priority = 3;

	// ----------- Getters / Setters ------------

	void setRS485Enabled(bool value)
	{
		_enabledRS485 = value;
	}

	bool getRS485Enabled()
	{
		return _enabledRS485;
	}

	void setLinuxSerialEnabled(bool value)
	{
		_enabledLinuxSerial = value;
	}

	bool getLinuxSerialEnabled()
	{
		return _enabledLinuxSerial;
	}

	// ------------ Functions -----------------

	bool SerialParser(HardwareSerial &SerialSelected, String &serialBuffer, uint32_t &serialTimeout, bool isHMI = false, int rsPin = 0)
	{
		bool parsed = false;
		char c = 0;

		uint8_t available = SerialSelected.available();
		if (!isHMI)
		{
			if (serialBuffer.length() > 0)
			{
				if (millis() - serialTimeout > SERIAL_RX_TIMEOUT_MS)
				{
					parsed = JSONManager.decode(serialBuffer, SerialSelected, rsPin);
					serialBuffer = ""; // Clear rx buffer
				}
			}

			while (available)
			{
				serialTimeout = millis(); // refresh serial non-blocking time
				c = SerialSelected.read();

				if (c != '\n')
				{
					if (serialBuffer.length() < SERIAL_RX_BUFFER)
						serialBuffer.concat(c);
					else
						serialBuffer = "";
				}
				else
				{                                                                   // Got some line
					parsed = JSONManager.decode(serialBuffer, SerialSelected, rsPin); // Decode JSON string and do some action
					serialBuffer = "";
				}
				available = SerialSelected.available();
			}
		}
		else if (available > 6) // If more than 6 bytes are received from IHM
		{
			c = SerialSelected.read();
			if (c == NEX_RET_EVENT_TOUCH_HEAD)
			{
				_hmiBuffer[0] = c;

				uint8_t i = 0;
				for (i = 1; i < 7; i++)
				{
					_hmiBuffer[i] = SerialSelected.read();
				}

				_hmiBuffer[i] = 0x00;

				if (0xFF == _hmiBuffer[4] && 0xFF == _hmiBuffer[5] && 0xFF == _hmiBuffer[6])
				{
					NexTouch::iterate(HMI._hmiEvents, _hmiBuffer[1], _hmiBuffer[2], (int32_t)_hmiBuffer[3]);
				}
			}
		}

		return parsed;
	}

	void init()
	{
#if UART_RS485_NUMBER==1
		Serial.begin(_serial1Baud);
#endif
		RS485.init();
		Linux.init();
	}

	void update()
	{
		SerialParser(Serial, _serial1Buffer, _serial1Timeout, HMI._enabled);

		if (_enabledLinuxSerial)
		{
			if (Linux.available())
			{
				SerialParser(Linux, _linuxSerialBuffer, _LinuxTimeout);
			}
		}

		if (_enabledRS485)
		{
			if (RS485.getModbusMode() == false)
			{
				SerialParser(RS485, _RS485Buffer, _RS485Timeout, false, RS485_DIR_PIN);
			}

			RS485.update();
		}
	}
};

#endif

extern SerialHandler SerialManager;