#ifndef _ATT7022_EU
#define _ATT7022_EU

#include <Arduino.h>
#include <SPI.h>

class ATT7022EU
{
#define r_Pflag 0x3D
#define R_Sflag 0x2C

#define r_Pa 0x01
#define r_Pb 0x02
#define r_Pc 0x03
#define r_Pt 0x04

#define r_Qa 0x05
#define r_Qb 0x06
#define r_Qc 0x07
#define r_Qt 0x08

#define r_Sa 0x09
#define r_Sb 0x0A
#define r_Sc 0x0B
#define r_St 0x0C

#define r_UaRms 0x0D
#define r_UbRms 0x0E
#define r_UcRms 0x0F
#define r_UtRms 0x2B
#define r_IaRms 0x10
#define r_IbRms 0x11
#define r_IcRms 0x12
#define r_ItRms 0x13

#define r_UaRmsFundamental 0x48
#define r_UbRmsFundamental 0x49
#define r_UcRmsFundamental 0x4A
#define r_IaRmsFundamental 0x4B
#define r_IbRmsFundamental 0x4C
#define r_IcRmsFundamental 0x4D

#define r_Pfa 0x14
#define r_Pfb 0x15
#define r_Pfc 0x16
#define r_Pft 0x17

#define r_Freq 0x1C

#define r_Epa 0x1E
#define r_Epb 0x1F
#define r_Epc 0x20
#define r_Ept 0x21

#define r_Eqa 0x22
#define r_Eqb 0x23
#define r_Eqc 0x24
#define r_Eqt 0x25

#define r_Epa2 0x31
#define r_Epb2 0x32
#define r_Epc2 0x33
#define r_Ept2 0x34

#define r_Eqa2 0x35
#define r_Eqb2 0x36
#define r_Eqc2 0x37
#define r_Eqt2 0x38
#define Meter_G                1.1338582677165354330708661417323

#define Vu  0.1
#define Vi  0.04
#define Un  127
#define In  1.15
#define Meter_Ec 3200
#define Meter_HFConst        ((2.592*pow(10,10)*Meter_G*Meter_G*Vu*Vi)/(In*Un*Meter_Ec))
#define Meter_K                        (2.592*pow(10,10)/(Meter_HFConst*Meter_Ec*pow(2,23)))

#define CS_PIN 15
#define CLK_PIN 14
#define DATA_PIN 13
#define DIN_PIN 12

#define Set_CS digitalWrite(CS_PIN,HIGH)
#define Clr_CS digitalWrite(CS_PIN,LOW)

#define Set_SCL digitalWrite(CLK_PIN,HIGH)
#define Clr_SCL digitalWrite(CLK_PIN,LOW)

#define Set_DATA digitalWrite(DATA_PIN,HIGH)
#define Clr_DATA digitalWrite(DATA_PIN,LOW)

#define Rd_MISO digitalRead(DIN_PIN)

public:
	bool init() {
		_spiSettings = new SPISettings(100000, MSBFIRST, SPI_MODE1);
		pinMode(CS_PIN, OUTPUT);
		Set_CS;

		return calibrate();
	};

	inline float readFrequency() {
		return (float)SPI_ATT_Read(r_Freq) / 8192.0;
	};

	inline float readRMSVoltage(uint8_t phase = 0) {
		return (float)SPI_ATT_Read(r_UaRms + phase) / 8192.0;
	};

	inline float readRMSVoltageFundamental(uint8_t phase = 0) {
		return (float)SPI_ATT_Read(r_UaRmsFundamental + phase) / 8192.0;
	};

	inline float readRMSCurrent(uint8_t phase = 0) {
		return (float)SPI_ATT_Read(r_IaRms + phase) / 8192.0;
	};

	inline float readRMSNeutral() {
		return (float)SPI_ATT_Read(0x29) / 8192.0;
	};

	inline float readRMSCurrentFundamental(uint8_t phase = 0) {
		return (float)SPI_ATT_Read(r_IaRmsFundamental + phase) / 8192.0;
	};

	inline float readPowerVA(uint8_t phase = 0) {
		uint32_t P = SPI_ATT_Read(r_Sa + phase);

		float Pr = 0;

		if (P > 0x800000) {
			Pr = (0x1000000 - P);
			Pr = (-Pr / 8.0);
		}
		else {
			Pr = (float)P / 8.0;
		}

		return Pr;
	};

	inline float readPowerReal(uint8_t phase = 0) {
		uint32_t P = SPI_ATT_Read(r_Pa + phase);

		float Pr = 0;

		if (P > 0x800000) {
			Pr = (0x1000000 - P);
			Pr = (-Pr / 8.0);
		}
		else {
			Pr = (float)P / 8.0;
		}

		return Pr;
	};

	inline float readPowerVAR(uint8_t phase = 0) {
		uint32_t P = SPI_ATT_Read(r_Qa + phase);

		float Pr = 0;

		if (P > 0x800000) {
			Pr = (0x1000000 - P);
			Pr = (-Pr / 8.0);
		}
		else {
			Pr = (float)P / 8.0;
		}

		return Pr;
	};

	inline float readPowerFactor(uint8_t phase = 0) {
		uint32_t P = SPI_ATT_Read(r_Pfa + phase);

		float Pr = 0;

		if (P > 0x800000) {
			Pr = (0x1000000 - P);
			Pr = (-Pr / 8388608.0);
		}
		else {
			Pr = (float)P / 8388608.0;
		}

		return Pr;
	};

	inline double readTHDVoltage(uint8_t phase = 0) {
		double Uall = readRMSVoltage(phase);
		double Ub = readRMSVoltageFundamental(phase);
		double Uh = sqrt(abs((Uall*Uall) - (Ub*Ub)));

		return (Uh / Ub) * 100.0;
	}

	inline double readTHDCurrent(uint8_t phase = 0) {
		double Iall = readRMSCurrent(phase);
		double Ib = readRMSCurrentFundamental(phase);
		double Ih = sqrt(abs((Iall*Iall) - (Ib*Ib)));

		return (Ih / Ib) * 100.0;
	};

	void update() {
		// For testing only
		printf("\r\n********************\r\n");

		double Ub = ((double)SPI_ATT_Read(r_UcRms) / 8192.0);
		printf("\r\n       %f !\r\n", Ub);

		double Uall = ((double)SPI_ATT_Read(0x4A) / 8192.0);

		double Uh = sqrt(abs((Uall*Uall) - (Ub*Ub)));
		double THD = (Uh / Ub) * 100.0;
		printf("\r\n THD:  %f !\r\n", THD);
		printf("\r\n       %f !\r\n", (float)SPI_ATT_Read(r_IcRms) / 8192.0);

		printf("\r\n       %f !\r\n", (float)SPI_ATT_Read(r_Freq) / 8192.0);
		printf("\r\n********************\r\n");
	};

	void setSpi(SPIClass *spi) {
		_spi = spi;
	};

private:
	SPIClass *_spi;
	SPISettings *_spiSettings;

	void SPI_ATT_Write(uint8_t com_add, uint32_t data2) {
		com_add |= 0x80; // Set write bit
		data2 |= (uint32_t)com_add << 24;

		_spi->beginTransaction(*_spiSettings);
		Clr_CS;
		_spi->transfer32(data2);
		Set_CS;
		_spi->endTransaction();
	};

	uint32_t SPI_ATT_Read(uint8_t data) {
		_spi->beginTransaction(*_spiSettings);
		Clr_CS;

		uint32_t ret = _spi->transfer32(((uint32_t)data << 24));
		Set_CS;
		_spi->endTransaction();

		return ret;
	};

	bool calibrate(void)
	{
		// TODO: place parameters
		SPI_ATT_Write(0xC3, 0x000000);
		SPI_ATT_Write(0xC9, 0x00005A);

		SPI_ATT_Write(0xC3, 0x000000);
		SPI_ATT_Write(0xC9, 0x00005A);

		// Config
		SPI_ATT_Write(0x01, 0xB97E);
		SPI_ATT_Write(0x03, 0xFC04); // Enable full wave measurement 14.4Khz
		SPI_ATT_Write(0x31, 0x3427);

		// PGA gain 2
		SPI_ATT_Write(0x02, 0b0100000000);

		// Temperature compensation
		SPI_ATT_Write(0x6D, 0xFF00);
		SPI_ATT_Write(0x6E, 0x0DB8);
		SPI_ATT_Write(0x6F, 0xD1DA);

		//Offset Correction
		SPI_ATT_Write(0x24, 0x1d8); // Voltage A
		SPI_ATT_Write(0x25, 0x1d8); // Voltage B
		SPI_ATT_Write(0x26, 0x1d8); // Voltage C
		SPI_ATT_Write(0x27, 0x200); // Current A
		SPI_ATT_Write(0x28, 0x200); // Current B
		SPI_ATT_Write(0x29, 0x200); // Current C

		// Voltage gain adjust
		SPI_ATT_Write(0x17, 0xF0AA);
		SPI_ATT_Write(0x18, 0xF0AA);
		SPI_ATT_Write(0x19, 0xF0AA);

		// Constants
		SPI_ATT_Write(0x1E, 0x0005E7);

		SPI_ATT_Write(0xC9, 0x000001);

		//Verify chip code
		if (SPI_ATT_Read(0x00) == 0x7122A0) return true;
		else return false;
	};
};

#endif