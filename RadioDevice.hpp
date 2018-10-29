#ifndef RADIO
#define RADIO

#define PJON_INCLUDE_TL
#include <PJON.h>
#include <PJONSlave.h>

template <typename Strategy>
class BoardRadioNode : public PJONSlave<Strategy>
{
private:
	bool _radioEnabled = false;

public:
	BoardRadioNode(uint8_t PjonId = PJON_NOT_ASSIGNED) : PJONSlave<Strategy>(PjonId)
	{
	}

	BoardRadioNode(uint8_t radioId[], uint8_t PjonId = PJON_NOT_ASSIGNED) : PJONSlave<Strategy>(radioId, PjonId)
	{
	}

	inline bool begin()
	{
		PJONSlave<Strategy>::strategy.setPins(23, 25, 34);
		bool init = PJONSlave<Strategy>::strategy.setFrequency(433E6);

		if (init)
		{
			PJONSlave<Strategy>::strategy.setSignalBandwidth(250E3);
			PJONSlave<Strategy>::strategy.setSpreadingFactor(7);
			PJONSlave<Strategy>::begin();
			Serial.println(PJONSlave<Strategy>::device_id());
			//PJONSlave<Strategy>::set_error(error_handler);
		}

		return init;
	}
};

#endif

extern BoardRadioNode<ThroughLora> Radio;