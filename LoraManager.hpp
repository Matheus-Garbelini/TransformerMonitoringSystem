#ifndef LORA_MANAGER
#define LORA_MANAGER

#define LORA_FREQ 433e6

#include <ArduinoJson.h>
#include <LoRa.h>
#include "QueryManager.hpp"
#include "RS485Device.hpp"

static bool receivedRawPacket;

class LoraManager
{
private:
	bool _protocolEnabled = false;
	bool _redirLora = true;
	uint8_t radioId[4] = { 0, 0, 0, 1 };
	uint8_t connected = 0;
	String NodeName = "pucpr/bloco8"; // CCN node name

	uint32_t refreshSensors;
	uint16_t RSSI;
	float SNR;

	inline void populateJsonData(JsonObject &data, char sensorChar)
	{
		switch (sensorChar)
		{
		case NULL: // Add all sensor if NULL is received
		case 'T':
			// data["T"] = Sensors.getTemperature();
			if (sensorChar)
				break;
		case 'H':
			// data["H"] = Sensors.getRelativeHumidity();
			if (sensorChar)
				break;
		case 'P':
			// data["P"] = (int)(Sensors.getPressurePa() / 1000);
			if (sensorChar)
				break;
		case 'L':
			// data["L"] = (int)Sensors.getLightMeasurement();
			if (sensorChar)
				break;
		case 'R':
			data["R"] = RSSI;
			if (sensorChar)
				break;
		}
	}

	static void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
	{
		LoraManager &loraManager = (LoraManager &)packet_info.custom_pointer;
		for (uint8_t i = 0; i < length; i++)
		{
			Serial.print(payload[i], HEX);
			Serial.write(' ');
		}
		Serial.println();
		if (loraManager.connected)
		{
			StaticJsonBuffer<1024> jsonBuffer;
			JsonObject &root = jsonBuffer.parseBinaryObject((char *)payload);
			char message[128];
			uint16_t queryID = 0;
			const char *interestName;
			loraManager.RSSI = abs((int16_t)LoRa.packetRssi());
			loraManager.SNR = Radio.strategy.packetSnr();

			if (root.success())
			{
				uint8_t sendData = 1;
				uint8_t excludeData = 0;
				uint8_t includeData = 0;
				uint8_t order = 0;
				uint8_t registerQuery = 0;

				if (!root.containsKey("Type"))
					return; // Check if Type is existent in message
				if (!root.containsKey("Name"))
					return; // Check if Name is existent in message
				if (!root.containsKey("Query"))
					return; // Check if Name is existent in message

				if ((int)root["Type"] == 0)
				{ // Check if message type is a interest (Type = 0)
					interestName = root["Name"];
					if (loraManager.NodeName.equals(interestName))
					{ // Check if message name is addressed to this node
						Serial.println("Received Interest:");

						// 1) Read QueryID number and add it to LRU cache
						if (root.containsKey("Query"))
						{ // Check Query ID of message
							queryID = (int)root["Query"];
							root["Q"] = queryID;
							if (root.containsKey("D"))
							{
								JsonObject &QueryPeriod = root["D"];
								if (QueryPeriod.containsKey("S"))
								{
									uint16_t sampleTime = QueryPeriod["S"];

									if (sampleTime > 0)
									{
										registerQuery = 1;
										if (QueryPeriod.containsKey("L"))
										{
											uint16_t lifeTime = QueryPeriod["L"];
											QueryManager.addQuery(queryID, sampleTime, lifeTime);
										}
										else
										{
											QueryManager.addQuery(queryID, sampleTime);
										}
									}
									else
									{
										QueryManager.removeQuery(queryID);
										sendData = 0;
										Serial.print("Canceled Query ");
										Serial.println(queryID);
									}
								}

								root.remove("D");
							}

							root.remove("Query");
						}
						else // Insert blank QueryID if message do not contain QueryID (Single Query request)
						{
							root["Q"] = 0;
						}

						// 2) Read Filter object
						if (root.containsKey("Filter")) // Check if message contain data filtering
						{
							JsonObject &filter = root["Filter"];
							if (filter.containsKey("Data"))
								sendData = filter["Data"];
							if (filter.containsKey("Order"))
								order = filter["Order"];

							if (filter.containsKey("Include"))
								includeData = 1; // Include has priotity over exclude
							else if (filter.containsKey("Exclude"))
								excludeData = 1;
						}

						// 4) Read Option object
						// TODO
						// 5) Mount Data structure with/without filter

						if (sendData == 1)
						{
							JsonObject &data = root.createNestedObject("D");

							if (includeData == 1)
							{
								JsonObject &filter = root["Filter"];
								JsonArray &include = filter["Include"];

								for (auto rule : include)
								{
									const char keyChar = rule.as<const char *>()[0];
									Serial.println(keyChar);
									loraManager.populateJsonData(data, keyChar); // Get first character of rule
									if (registerQuery)
										QueryManager.addQueryRule(queryID, keyChar);
								}
							}
							else if (excludeData == 1)
							{
								JsonObject &filter = root["Filter"];
								JsonArray &exclude = filter["Exclude"];

								loraManager.populateJsonData(data, NULL); // Add all sensor values first

								for (auto rule : exclude) // Loop through all rules, removing sensor values
								{
									const char *keyChar = rule; // Get sensor rule name

									if (data.containsKey(keyChar))
										data.remove(keyChar);
								}
								if (registerQuery)
								{
									for (auto rule : data) // Loop through all rules, removing sensor values
									{
										const char *keyChar = rule.key; // Get sensor rule name

										QueryManager.addQueryRule(queryID, keyChar[0]);
									}
								}
							}
							else
							{
								loraManager.populateJsonData(data, NULL); // Add all sensor values

								if (registerQuery)
								{
									for (auto rule : data) // Loop through all rules, removing sensor values
									{
										const char *keyChar = rule.key; // Get sensor rule name

										QueryManager.addQueryRule(queryID, keyChar[0]);
									}
								}
							}

							// 5) Close Packet and send message with data structure
							if (!registerQuery)
							{
								if (root.containsKey("Filter"))
									root.remove("Filter");
								if (root.containsKey("Opt"))
									root.remove("Opt");

								root.remove("Type");
								root["T"] = 1; // Change message type to response (Type = 1)
								root.remove("Name");

								int size = root.binaryPrintTo(message, 128); // Serialize data to MessagePack

								delay(Radio.device_id() * 200);
								Radio.send(PJON_MASTER_ID, message, size); // Send packet in broadcast mode
								//unsigned int response = Radio.send_packet(PJON_MASTER_ID, message, size);
								Serial.println("Bytes sent: " + String(size));

								// Blink to show status
								// digitalWrite(LED_BUILTIN, LOW);
								// delay(30);
								// digitalWrite(LED_BUILTIN, HIGH);
							}
						}
					}
				}
			}
		}
	}

	void rawReceive(int packetSize)
	{
		StaticJsonBuffer<512> jsonBuffer;
		JsonObject &root = jsonBuffer.createObject();
		String msg = "";

		// Read packet into msg string
		for (int i = 0; i < packetSize; i++)
		{
			msg.concat((char)LoRa.read());
		}

		// Read lora message and insert it in json object
		root["msg"] = msg;

		root["bytes"] = packetSize;
		// Read rssi from received packet and insert it in json object
		root["rssi"] = LoRa.packetRssi();

		root["snr"] = LoRa.packetSnr();

		root["freqErr"] = LoRa.packetFrequencyError();

		// Print message with rssi in json
		root.prettyPrintTo(Serial);
		Serial.write('\n');

		if (RS485.getModbusMode() == false && _redirLora == true)
		{
			digitalWrite(22, HIGH);
			root.prettyPrintTo(RS485);
			RS485.write('\n');
			RS485.flush();
			delay(1);
			digitalWrite(22, LOW);
		}
	}

	static void LoraInterrupt()
	{
		receivedRawPacket = true;
	}

public:
	//RTOS Options - Optional
	bool RTOS_ENABLE = true;
	uint16_t RTOS_updateTime = 20;
	uint16_t RTOS_priority = 2;
	// Radio parameters with it's default values
	int freq = LORA_FREQ;
	float tx = 20.0;
	uint16_t sf = 7;
	uint16_t cr = 5;
	int bw = 250e3;
	uint16_t pl = 8;
	int sw = 0x12;
	bool crc = false;

	void resetValues()
	{
		tx = 20.0;
		sf = 7;
		cr = 5;
		bw = 250e3;
		pl = 8;
		sw = 0x12;
		crc = false;
	}

	void configureRadio()
	{
		// Default values
		LoRa.setTxPower(tx);         // TX power in dB
		LoRa.setSignalBandwidth(bw); // Signal bandwidth in Hz
		// Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, and 250E3.
		LoRa.setSpreadingFactor(7); // Change the spreading factor of the radio.
		/*
	  Supported values are between 6 and 12.
	  If a spreading factor of 6 is set,
	  implicit header mode must be used to transmit and receive packets.
	*/
		LoRa.setCodingRate4(cr); // Change the coding rate of the radio.
		/*
	  Supported values are between 5 and 8, these correspond to coding rates of 4/5 and 4/8.
	  The coding rate numerator is fixed at 4.
	*/
		LoRa.setSyncWord(sw);       // Change the sync word of the radio.
		LoRa.setPreambleLength(pl); // Change the preamble length of the radio.
		// Supported values are between 6 and 65535.
		LoRa.disableCrc(); // Enable or disable CRC usage
	}

	bool getProtocolEnabled()
	{
		return _protocolEnabled;
	}

	void setProtocolEnabled(bool value)
	{
		_protocolEnabled = value;
	}

	bool init()
	{
		bool ret;

		ret = Radio.begin();
		if (!ret)
			return false;
		Radio.set_custom_pointer(this);
		Radio.set_receiver(receiver_function);
		Radio.set_synchronous_acknowledge(true); //
		QueryManager.begin(Radio);

		pinMode(34, INPUT_PULLDOWN);
		attachInterrupt(34, LoraInterrupt, RISING);

		Serial.println("LoRa Initialized");

		//LoRa.onReceive(LoraInterrupt);
		LoRa.receive();

		return true;
	}

	void update()
	{
		static uint8_t acquired = PJON_NOT_ASSIGNED;
		static uint32_t disconnectedTimeout = 0;

		if (_protocolEnabled) // Pjon protocol handler
		{
			Radio.update();
			Radio.receive();

			QueryManager.update();

			if (!connected)
			{
				if (millis() - disconnectedTimeout > 5000)
				{
					Serial.println("Retrying connection...");
					delay(PJON_RANDOM(1000));
					connected = Radio.acquire_id_master_slave();
					Serial.println("Finished");
					disconnectedTimeout = millis();
				}
			}

			if ((Radio.device_id() && Radio.device_id() != 255))
			{
				acquired = Radio.device_id();
				connected = 1;
				Serial.print("Acquired device id: ");
				Serial.println(Radio.device_id());
			}
		}
		else if (receivedRawPacket == true) // Raw receive
		{
			receivedRawPacket = false;
			int packetSize = LoRa.parsePacket();

			if (packetSize > 0)
				rawReceive(packetSize);

			LoRa.receive();
		}
	}
};

#endif
extern LoraManager Lora;