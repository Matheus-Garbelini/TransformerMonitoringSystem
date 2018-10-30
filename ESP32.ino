#define PJON_INCLUDE_TL
#include <SPI.h>
#include <PJONSlave.h>

// Bus id definition
uint8_t bus_id[] = { 0, 0, 0, 1 };

// PJON object
PJONSlave<ThroughLora> bus(bus_id, PJON_NOT_ASSIGNED);

int packet;
char content[] = "01234567890123456789";
uint32_t times;

void setup() {
	Serial.begin(115200);
	bus.set_error(error_handler);
	bus.strategy.setPins(23, 25, 34);
	Serial.println(bus.strategy.setFrequency(433000000UL));
	bus.strategy.setSignalBandwidth(250E3);
	bus.begin();

	times = millis();
}

static void error_handler(uint8_t code, uint16_t data, void *p) {
	if (code == PJON_CONNECTION_LOST) {
		Serial.print("Connection lost with master ");
		Serial.println(bus.packets[data].content[0], DEC);
	}
	if (code == PJON_ID_ACQUISITION_FAIL) {
		Serial.print("Unable to acquire an id: ");
		Serial.println(data);
	}
}

void loop() {
	if (millis() - times > 3000) {
		Serial.println(bus.device_id());
		times = millis();
	}
	bus.update();
	bus.receive();
};