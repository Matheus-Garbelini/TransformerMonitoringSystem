#ifndef QUERY_MANAGER
#define QUERY_MANAGER
//#include <unordered_map>

#include <map>

#include <stdlib.h>
#include <vector>
#include "RadioDevice.hpp"
#include <ArduinoJson.h>

#define LIFETIME_MAX 60 * 60 // 1 Hour

class QueryManagerClass
{
  private:
	struct QueryInfo
	{
		std::vector<char> DataRules;
		bool UTCEnabled;
		uint16_t samplePeriod;  // Query message sample time
		uint32_t initialPeriod; // Query scheduled initial time
		uint16_t finalPeriod;   // Query scheduled expiration time
		uint32_t currentPeriod; // Current Query time
	};

	std::map<uint16_t, QueryInfo> QueryList;
	BoardRadioNode<ThroughLora> *Radio;

	uint16_t nextQueryID = 0;   // Stores the next Query event to serve (update timing value, send message and sleep)
	uint16_t lastSleepTime = 0; // Stores last sleep time until next query event
	bool gpsEnabled = false;	// Enable GPS PPS time sincronization

	inline void populateJsonData(JsonObject &data, char sensorChar)
	{
		//switch (sensorChar)
		//{
		//case NULL: // Add all sensor if NULL is received
		//case 'T': data["T"] = Sensors.getTemperature();
		//	if (sensorChar)
		//		break;
		//case 'H': data["H"] = Sensors.getRelativeHumidity();
		//	if (sensorChar)
		//		break;
		//case 'P': data["P"] = (int)(Sensors.getPressurePa() / 1000);
		//	if (sensorChar)
		//		break;
		//case 'L': data["L"] = (int)Sensors.getLightMeasurement();
		//	if (sensorChar)
		//		break;
		//case 'R': data["R"] = (int)abs(Radio->strategy.packetRssi(false));
		//	break;
		//}
	}

  public:
	void begin(BoardRadioNode<ThroughLora> &Radio)
	{
		this->Radio = &Radio;
	}

	uint8_t updateQueryID(int QueryID)
	{
		return QueryList.find(QueryID) == QueryList.end();
	}

	void update()
	{
		StaticJsonBuffer<1024> jsonBuffer;
		char text[128];

		if (QueryList.size() != 0)
		{
			for (auto query : QueryList)
			{
				//Serial.println(query.second.samplePeriod);
				uint32_t time = millis();

				if (time - query.second.currentPeriod >= (uint32_t)(query.second.samplePeriod * 1000))
				{
					QueryList[query.first].currentPeriod = time;

					JsonObject &root = jsonBuffer.createObject();
					JsonObject &data = root.createNestedObject("D");

					for (auto rule : query.second.DataRules)
					{
						populateJsonData(data, rule);
					}

					root["Q"] = query.first;
					root["T"] = 1;
					delay(Radio->device_id() * 200);
					Serial.println("Query Sent");
					uint8_t size = root.binaryPrintTo(text);
					Radio->send(PJON_BROADCAST, text, size);

					digitalWrite(2, LOW);
					delay(30);
					digitalWrite(2, HIGH);
				}

				if (time - query.second.initialPeriod > query.second.finalPeriod * 1000)
				{
					QueryList.erase(query.first); // Delete Query
				}
			}
		}
	}

	void addQuery(uint16_t id, uint16_t samplePeriod)
	{
		QueryList[id] = QueryInfo{{}, false, samplePeriod, millis(), 60, millis()};
		Serial.print("Query ID: ");
		Serial.print(id);
		Serial.print(" SamplePeriod: ");
		Serial.println(samplePeriod);
	}

	void addQuery(uint16_t id, uint16_t samplePeriod, uint16_t finalPeriod)
	{
		finalPeriod = constrain(finalPeriod, 0, LIFETIME_MAX);
		QueryList[id] = QueryInfo{{}, false, samplePeriod, millis(), finalPeriod, millis()};
		Serial.print("Query ID: ");
		Serial.print(id);
		Serial.print(" SamplePeriod: ");
		Serial.println(samplePeriod);
		Serial.print(" FinalPeriod: ");
		Serial.println(finalPeriod);
	}

	void addQueryRule(uint16_t id, char Rule)
	{
		QueryList[id].DataRules.push_back(Rule);
	}

	void removeQuery(uint16_t id)
	{
		QueryList.erase(id);
	}
};
#endif

extern QueryManagerClass QueryManager;
