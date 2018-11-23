#ifndef JSON_PARSER
#define JSON_PARSER

#define FIRMWARE_VERSION "1.2"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LoRa.h>
#include "RS485Device.hpp"

#define SET_DIR \
    if (rsPin)  \
    digitalWrite(rsPin, HIGH)
#define CLEAR_DIR                 \
    if (rsPin)                    \
    {                             \
        delay(10);                \
        digitalWrite(rsPin, LOW); \
    }

class JSONHandler
{
private:
	/* data */
public:
	bool decode(String &serialBuffer, HardwareSerial &selectedSerial = Serial, int rsPin = 0)
	{
		StaticJsonBuffer<512> jsonBuffer;
		JsonObject &root = jsonBuffer.parseObject(serialBuffer);

		bool success = root.success();

		if (success)
		{
			if (root.containsKey("linux"))
			{
				bool value = root["linux"];
				Linux.setInitialized(value);
			}
			if (root.containsKey("hmi"))
			{
				bool value = root["hmi"];
				HMI.setEnable(value);
			}
			if (root.containsKey("gps"))
			{
				bool value = root["gps"];
				GPS.setDebug(value);
			}
			if (root.containsKey("nmea"))
			{
				bool value = root["nmea"];
				GPS.setNMEAMode(value);
			}
			if (root.containsKey("modbus"))
			{
				bool value = root["modbus"];
				RS485.setModbus(value);
			}
			if (root.containsKey("loraprotocol"))
			{
				Serial.println("LoRa protocol");
				bool value = root["loraprotocol"];
				Lora.setProtocolEnabled(value);
				LoRa.receive();
			}
			if (root.containsKey("dump"))
			{
				bool value = root["dump"];
				Measurements.dump_serial = value;
			}
			if (root.containsKey("tx"))
			{
				Lora.tx = (float)root["tx"];
				LoRa.setTxPower(Lora.tx);
				SET_DIR;
				selectedSerial.print("TX Power (dB): ");
				selectedSerial.println(Lora.tx);
				CLEAR_DIR;
			}
			if (root.containsKey("sf"))
			{
				Lora.sf = root["sf"];
				LoRa.setSpreadingFactor(Lora.sf);
				SET_DIR;
				selectedSerial.print("Spreading Factor: ");
				selectedSerial.println(Lora.sf);
				CLEAR_DIR;
			}
			if (root.containsKey("cr"))
			{
				Lora.cr = root["cr"];
				LoRa.setCodingRate4(Lora.cr);
				SET_DIR;
				selectedSerial.print("Coding Rate: ");
				selectedSerial.print("4/");
				selectedSerial.println(Lora.cr);
				CLEAR_DIR;
			}
			if (root.containsKey("bw"))
			{
				Lora.bw = root["bw"];
				LoRa.setSignalBandwidth(Lora.bw);
				SET_DIR;
				selectedSerial.print("Signal Bandwidth: ");
				selectedSerial.println(Lora.bw);
				CLEAR_DIR;
			}
			if (root.containsKey("pl"))
			{
				Lora.pl = root["pl"];
				LoRa.setPreambleLength(Lora.pl);
				SET_DIR;
				selectedSerial.print("Preamble Length: ");
				selectedSerial.println(Lora.pl);
				CLEAR_DIR;
			}
			if (root.containsKey("sw"))
			{
				Lora.sw = root["sw"];
				LoRa.setSyncWord(Lora.sw);
				SET_DIR;
				selectedSerial.print("Syncword: ");
				selectedSerial.println(Lora.sw, HEX);
				CLEAR_DIR;
			}
			if (root.containsKey("crc"))
			{
				Lora.crc = root["crc"];
				if (Lora.crc)
				{
					LoRa.enableCrc();
					SET_DIR;
					selectedSerial.println("CRC Enabled");
					CLEAR_DIR;
				}
				else
				{
					LoRa.disableCrc();
					SET_DIR;
					selectedSerial.println("CRC Disabled");
					CLEAR_DIR;
				}
			}

			if (root.containsKey("msg"))
			{
				String msg = root["msg"];
				LoRa.beginPacket();
				LoRa.print(msg);
				LoRa.endPacket();
				LoRa.receive(); // put the radio into receive mode
			}
		}
		// In case the input is not in JSON format
		else if (serialBuffer.equals("version"))
		{
			SET_DIR;
			selectedSerial.println("-------------------------------");
			selectedSerial.print("Firmware version: ");
			selectedSerial.println(FIRMWARE_VERSION);
			selectedSerial.println("-------------------------------");
			CLEAR_DIR;
		}

		else if (serialBuffer.equals("help"))
		{
			SET_DIR;
			selectedSerial.println("-------------------------------");
			selectedSerial.println("Commands List:");
			selectedSerial.println("{sf:7-12} - Set Radio Spreading Factor ranging from 7 to 12");
			selectedSerial.println("{cr:5-8} - Set Radio Coding rate denominator ranging from 5 to 8.");
			selectedSerial.println("{bw:125e3-250e3} - Set Radio signal Bandwidth in Hz. Maximum is 250Khz.");
			selectedSerial.println("{tx:2-20} - Set Radio TX power in dB ranging from 2 to 20.");
			selectedSerial.println("{pl:2-20} - Set Radio preamble length ranging from 6 to 65535.");
			selectedSerial.println("{sw:0x12} - Set Radio sync word.");
			selectedSerial.println("{crc:true-false} - Enable or disable CRC verification.");
			selectedSerial.println("config - Show radio parameters");
			selectedSerial.println("random - Print random value from radio");
			selectedSerial.println("version - Print radio version");
			selectedSerial.println("reset - Reset radio parameters to default values");
			selectedSerial.println("hreset - Reset radio software");

			selectedSerial.println("Received packet fields:");
			selectedSerial.println("\"msg\" - Packet string");
			selectedSerial.println("\"rssi\" - Received Signal Strength Indication of the packet in dB");
			selectedSerial.println("\"snr\" - Packet Signal-to-Noise Ratio in dB");
			selectedSerial.println("\"freqErr\" - Frequency error of the received packet in Hz");
			selectedSerial.println("-------------------------------");
			CLEAR_DIR;
		}
		else if (serialBuffer.equals("config"))
		{
			JsonObject &root = jsonBuffer.createObject();
			root["freq"] = Lora.freq;
			root["sf"] = Lora.sf;
			root["cr"] = Lora.cr;
			root["bw"] = Lora.bw;
			root["pl"] = Lora.pl;
			root["sw"] = String(Lora.sw, HEX);
			root["crc"] = Lora.crc;

			SET_DIR;
			root.prettyPrintTo(selectedSerial);
			selectedSerial.write('\n');

			CLEAR_DIR;
		}
		else if (serialBuffer.equals("reset"))
		{
			Lora.resetValues();
			Lora.configureRadio();
			SET_DIR;
			selectedSerial.println("Default parameters loaded");
			CLEAR_DIR;
		}
		else if (serialBuffer.equals("hreset"))
		{
			//nvic_sys_reset();
			ESP.restart();
		}
		else if (serialBuffer.equals("memory"))
		{
			SET_DIR;
			selectedSerial.println(ESP.getFreeHeap() / 1000);
			CLEAR_DIR;
		}
		else if (serialBuffer.equals("random"))
		{
			SET_DIR;
			Serial.println(LoRa.random());
			CLEAR_DIR;
		}
		//else if (serialBuffer.length() > 0)
		//{
		//    LoRa.beginPacket();
		//    LoRa.print(serialBuffer);
		//    LoRa.endPacket();
		//    // put the radio into receive mode
		//    LoRa.receive();
		//}

		return success;
	}
};

#endif

extern JSONHandler JSONManager;