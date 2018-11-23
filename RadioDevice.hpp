#ifndef RADIO
#define RADIO

#define PJON_INCLUDE_TL
#include <PJON.h>

template <typename Strategy>
class BoardRadioNode : public PJON<Strategy>
{
private:
	bool _radioEnabled = false;

public:
	BoardRadioNode(uint8_t PjonId = PJON_NOT_ASSIGNED) : PJON<Strategy>(PjonId)
	{
	}

	inline void setSPI(SPIClass *spi) {
		PJON<Strategy>::strategy.setSPI(spi);
	}

	inline bool begin()
	{
		PJON<Strategy>::strategy.setPins(23, 25, 34);
		bool init = PJON<Strategy>::strategy.setFrequency(433E6);

		if (init)
		{
			PJON<Strategy>::strategy.setSignalBandwidth(250E3);
			PJON<Strategy>::strategy.setSpreadingFactor(7);
			//PJONSlave<Strategy>::begin();
			Serial.println(PJON<Strategy>::device_id());
			//PJONSlave<Strategy>::set_error(error_handler);
		}

		return init;
	}
};

#endif

extern BoardRadioNode<ThroughLora> Radio;