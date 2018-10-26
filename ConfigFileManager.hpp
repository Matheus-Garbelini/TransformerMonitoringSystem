#ifndef CONFIG_FILE
#define CONFIG_FILE

#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <ArduinoTrace.h>
#include <visit_struct.hpp>

#define FORMAT_SPIFFS_IF_FAILED true
#define CONFIG_FILE_PATH "/config.json"
#define JSON_BUFFER_SIZE 1024

// Macros
#define typeof(...) std::remove_reference_t < decltype(__VA_ARGS__) >

// Global Default Parameters struct
struct PARAMS
{
	// Serial
	bool Linux_EnableSerial = true;
	int Linux_SerialBaudrate = 115200;

	bool RS485_EnableSerial = true;
	int RS485_SerialBaudrate = 115200;
	bool RS485_EnableModbus = true;
	uint8_t RS485_SlaveID = 2;
	uint8_t RS485_RedirLora = true;

	// GPS
	bool GPS_Debug = false;
	bool GPS_NMEAMode = false;
	int GPS_utcOffeset = -3 * 3600; // Brazil UTC offset

	// Lora
	bool Lora_EnableProtocol = false;
	bool Lora_EnableLoraWAN = false;
	String Lora_NodeName = "/pucpr/bloco8";
	uint32_t Lora_Frequency = 433; // * e6
	float Lora_PowerTX = 20.0;
	uint16_t Lora_SpreadingFactor = 7;
	uint16_t Lora_CodingRate = 5;
	int Lora_Bandwidth = 250; // * e3
	uint16_t Lora_PreambleLength = 8;
	int Lora_SyncWord = 0x12;
	bool Lora_EnableCRC = false;
};

// Reflection parameters to be load or saved into configuration file
VISITABLE_STRUCT(PARAMS,
	Linux_EnableSerial,
	Linux_SerialBaudrate,
	RS485_EnableSerial,
	RS485_SerialBaudrate,
	RS485_EnableModbus,
	RS485_SlaveID,

	GPS_Debug,
	GPS_NMEAMode,
	GPS_utcOffeset,

	Lora_EnableProtocol,
	Lora_EnableLoraWAN,
	Lora_NodeName,
	Lora_Frequency,
	Lora_PowerTX,
	Lora_SpreadingFactor,
	Lora_CodingRate,
	Lora_Bandwidth,
	Lora_PreambleLength,
	Lora_SyncWord,
	Lora_EnableCRC);

class ConfigFileManager
{
private:
	File _configFile;

	void decodeJSON(JsonObject& root)
	{
		visit_struct::for_each(Params,
			[&root](const char* name, auto& value)
		{
			if (root.containsKey(name))
			{
				value = root[name].as<typeof(value)>();
			}
		});
	}

	void encodeJSON(JsonObject& root)
	{
		visit_struct::for_each(Params,
			[&root](const char* name, auto& value)
		{
			root[name] = value;
		});
	}

	void commitParams() {
	}

public:
	PARAMS Params;

	ConfigFileManager(/* args */)
	{
	};
	// Functions

	void load()
	{
		StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;

		_configFile = SPIFFS.open(CONFIG_FILE_PATH, FILE_READ);

		// If there's no default file
		if (!_configFile)
		{
			_configFile.close();
			save();
			return;
		}

		JsonObject& root = jsonBuffer.parse(_configFile);

		if (root.success())
		{
			decodeJSON(root);
			Serial.println("Config file loaded");
		}

		_configFile.close();
	}

	void save()
	{
		StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;

		_configFile = SPIFFS.open(CONFIG_FILE_PATH, FILE_WRITE);

		JsonObject& root = jsonBuffer.createObject();

		encodeJSON(root);

		root.printTo(_configFile);

		_configFile.close();
	}

	// Print config file

	void printConfigFile(HardwareSerial& SerialType)
	{
		StaticJsonBuffer<1024> jsonBuffer;

		JsonObject& root = jsonBuffer.createObject();
		encodeJSON(root);

		root.prettyPrintTo(SerialType);
	}

	bool init()
	{
		const bool ret = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);
		// If file system is mounted
		/*if (ret)
			load();*/

		return ret;
	}

	void update() {
	}
};

#endif

extern ConfigFileManager Config;