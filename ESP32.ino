// Core libraries
#include <inttypes.h>
#include <Arduino.h>
#include <SPI.h>

// Definitions
#define LED_PIN 2
#define LED_ON digitalWrite(LED_PIN, LOW);
#define LED_OFF digitalWrite(LED_PIN, HIGH);

#define CS_PIN 15
#define CLK_PIN 14
#define DATA_PIN 13
#define DIN_PIN 12

#define SetInput(x) pinMode(x,INPUT)
#define SetOutput(x) pinMode(x,OUTPUT)

#define Set_CS digitalWrite(CS_PIN,HIGH)
#define Clr_CS digitalWrite(CS_PIN,LOW)

#define Set_SCL digitalWrite(CLK_PIN,HIGH)
#define Clr_SCL digitalWrite(CLK_PIN,LOW)

#define Set_DATA digitalWrite(DATA_PIN,HIGH)
#define Clr_DATA digitalWrite(DATA_PIN,LOW)

#define Rd_MISO digitalRead(DIN_PIN)

#define delay_us delayMicroseconds

#define HFconst	0xA0
#define UADC	0xBF
#define UgainA	0x9B
#define UgainB	0x9C
#define UgainC	0x9D

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

#define Vu  0.190
#define Vi  0.003
#define Un  220
#define In  4.15
#define Meter_Ec 3200
#define Meter_HFConst        ((2.592*pow(10,10)*Meter_G*Meter_G*Vu*Vi)/(In*Un*Meter_Ec))
#define Meter_K                        (2.592*pow(10,10)/(Meter_HFConst*Meter_Ec*pow(2,23)))

struct Meter_Adj_Val_Stru_Def
{
	float I_Amp_Factor;
	float V_Amp_Factor;
	float P_Gain_compensation;
	float Ph_compensation;
};

struct DataTypeDef
{
	uint32_t P;
	float Rp;
	uint32_t Q;
	float Rq;
	uint32_t S;
	float Rs;
	uint32_t URms;
	float Rurms;
	uint32_t IRms;
	float Rirms;
	uint32_t Pf;
	float Rpf;
	uint32_t Freq;
	float Rfreq;
};

struct DataTypeDef ADataTypeDef, BDataTypeDef, CDataTypeDef, TDataTypeDef;
struct Meter_Adj_Val_Stru_Def PhaseA = { 1.0,1.0,0.0,0.0 }, PhaseB = { 1.0,1.0,0.0,0.0 }, PhaseC = { 1.0,1.0,0.0,0.0 };

SPIClass *Spi = NULL;

void ATT_Adjust(void)
{
	SPI_ATT_Write(0xC3, 0x000000);
	SPI_ATT_Write(0xC9, 0x00005A);

	SPI_ATT_Write(0xC3, 0x000000);
	SPI_ATT_Write(0xC9, 0x00005A);

	// Config
	SPI_ATT_Write(0x01, 0xB97E);
	SPI_ATT_Write(0x03, 0xF804);
	SPI_ATT_Write(0x31, 0x3427);

	// PGA gain 2
	SPI_ATT_Write(0x02, 0b0100000000);

	// Temperature compensation
	SPI_ATT_Write(0x6D, 0xFF00);
	SPI_ATT_Write(0x6E, 0x0DB8);
	SPI_ATT_Write(0x6F, 0xD1DA);

	// Voltage gain adjust
	SPI_ATT_Write(0x17, 0xF0AA);
	SPI_ATT_Write(0x18, 0xF0AA);
	SPI_ATT_Write(0x19, 0xB5AA);

	SPI_ATT_Write(0xC9, 0x000001);
}

void setup()
{
	Serial.begin(115200);

	// TODO: att7022 tests here

	Spi = new SPIClass(HSPI);
	Spi->begin(CLK_PIN, DIN_PIN, DATA_PIN, CS_PIN);
	Spi->setHwCs(false);
	Spi->setFrequency(100000);
	Spi->setBitOrder(MSBFIRST);
	Spi->setDataMode(SPI_MODE1);
	pinMode(CS_PIN, OUTPUT);
	pinMode(23, OUTPUT);
	pinMode(25, OUTPUT);
	Set_CS;
	digitalWrite(23, HIGH);
	digitalWrite(25, LOW);

	ATT_Adjust();
};

void output_att7022(struct DataTypeDef output) {
	printf("\r\n********************\r\n");
	/*printf("\r\nP = %x !\r\n", output.P);
	printf("\r\n    %f !\r\n", output.Rp);
	printf("\r\nQ = %x !\r\n", output.Q);
	printf("\r\n    %f !\r\n", output.Rq);
	printf("\r\nS = %x !\r\n", output.S);
	printf("\r\n    %f !\r\n", output.Rs);*/
	//printf("\r\nURms = %x !\r\n", output.URms);
	printf("\r\n       %f !\r\n", output.Rurms);
	//printf("\r\nIRms = %x !\r\n", output.IRms);
	printf("\r\n       %f !\r\n", output.Rirms);
	/*printf("\r\nPf = %x !\r\n", output.Pf);
	printf("\r\n     %f !\r\n", output.Rpf);
	printf("\r\nFreq = %x !\r\n", output.Freq);*/
	printf("\r\n       %f !\r\n", output.Rfreq);
	printf("\r\n********************\r\n");
}

void Read_ATT_AData(void)
{
	ADataTypeDef.P = SPI_ATT_Read(r_Pa);
	ADataTypeDef.Q = SPI_ATT_Read(r_Qa);
	ADataTypeDef.S = SPI_ATT_Read(r_Sa);
	ADataTypeDef.URms = SPI_ATT_Read(r_UaRms);
	ADataTypeDef.IRms = SPI_ATT_Read(r_IaRms);
	ADataTypeDef.Pf = SPI_ATT_Read(r_Pfa);
	ADataTypeDef.Freq = SPI_ATT_Read(r_Freq);
	if (ADataTypeDef.P > 0x800000) {
		ADataTypeDef.Rp = 0x1000000 - ADataTypeDef.P;
		ADataTypeDef.Rp = -((ADataTypeDef.Rp*Meter_K*fabs(PhaseA.I_Amp_Factor)*fabs(PhaseA.V_Amp_Factor))\
			- ((ADataTypeDef.Rp*Meter_K*fabs(PhaseA.I_Amp_Factor)*fabs(PhaseA.V_Amp_Factor))*PhaseA.P_Gain_compensation));    //   2^15/2^23
	}
	else
		ADataTypeDef.Rp = (ADataTypeDef.P*Meter_K*fabs(PhaseA.I_Amp_Factor)*fabs(PhaseA.V_Amp_Factor))\
		- ((ADataTypeDef.P*Meter_K*fabs(PhaseA.I_Amp_Factor)*fabs(PhaseA.V_Amp_Factor))*PhaseA.P_Gain_compensation);
	if (ADataTypeDef.Q > 0x800000) {
		ADataTypeDef.Rq = 0x1000000 - ADataTypeDef.Q;
		ADataTypeDef.Rq = -((ADataTypeDef.Rq*Meter_K*fabs(PhaseA.I_Amp_Factor)*fabs(PhaseA.V_Amp_Factor))\
			- ((ADataTypeDef.Rq*Meter_K*fabs(PhaseA.I_Amp_Factor)*fabs(PhaseA.V_Amp_Factor))*PhaseA.Ph_compensation));
	}
	else
		ADataTypeDef.Rq = (ADataTypeDef.Q*Meter_K*fabs(PhaseA.I_Amp_Factor)*fabs(PhaseA.V_Amp_Factor))\
		- ((ADataTypeDef.Q*Meter_K*fabs(PhaseA.I_Amp_Factor)*fabs(PhaseA.V_Amp_Factor))*PhaseA.Ph_compensation);
	ADataTypeDef.Rs = ADataTypeDef.S / 256.0;
	ADataTypeDef.Rurms = ADataTypeDef.URms / 8192.0*PhaseA.V_Amp_Factor;   //   2^10/2^23
	ADataTypeDef.Rirms = ADataTypeDef.IRms / 8192.0*PhaseA.I_Amp_Factor;
	if (ADataTypeDef.Pf > 0x800000) {
		ADataTypeDef.Rpf = 0x1000000 - ADataTypeDef.Pf;
		ADataTypeDef.Rpf = -(ADataTypeDef.Rpf / 8388608.0);
	}
	else
		ADataTypeDef.Rpf = ADataTypeDef.Pf / 8388608.0;
	ADataTypeDef.Rfreq = ADataTypeDef.Freq / 8192.0;

	output_att7022(ADataTypeDef);
}

void Read_ATT_BData(void)
{
	BDataTypeDef.P = SPI_ATT_Read(r_Pb);
	BDataTypeDef.Q = SPI_ATT_Read(r_Qb);
	BDataTypeDef.S = SPI_ATT_Read(r_Sb);
	BDataTypeDef.URms = SPI_ATT_Read(r_UbRms);
	BDataTypeDef.IRms = SPI_ATT_Read(r_IbRms);
	BDataTypeDef.Pf = SPI_ATT_Read(r_Pfb);
	BDataTypeDef.Freq = SPI_ATT_Read(r_Freq);
	if (BDataTypeDef.P > 0x800000) {
		BDataTypeDef.Rp = 0x1000000 - BDataTypeDef.P;
		BDataTypeDef.Rp = -((BDataTypeDef.Rp*Meter_K*fabs(PhaseB.I_Amp_Factor)*fabs(PhaseB.V_Amp_Factor))\
			- ((BDataTypeDef.Rp*Meter_K*fabs(PhaseB.I_Amp_Factor)*fabs(PhaseB.V_Amp_Factor))*PhaseB.P_Gain_compensation));    //   2^15/2^23
	}
	else
		BDataTypeDef.Rp = (BDataTypeDef.P*Meter_K*fabs(PhaseB.I_Amp_Factor)*fabs(PhaseB.V_Amp_Factor))\
		- ((BDataTypeDef.P*Meter_K*fabs(PhaseB.I_Amp_Factor)*fabs(PhaseB.V_Amp_Factor))*PhaseB.P_Gain_compensation);
	if (BDataTypeDef.Q > 0x800000) {
		BDataTypeDef.Rq = 0x1000000 - BDataTypeDef.Q;
		BDataTypeDef.Rq = -((BDataTypeDef.Rq*Meter_K*fabs(PhaseB.I_Amp_Factor)*fabs(PhaseB.V_Amp_Factor))\
			- ((BDataTypeDef.Rq*Meter_K*fabs(PhaseB.I_Amp_Factor)*fabs(PhaseB.V_Amp_Factor))*PhaseB.Ph_compensation));
	}
	else
		BDataTypeDef.Rq = (BDataTypeDef.Q*Meter_K*fabs(PhaseB.I_Amp_Factor)*fabs(PhaseB.V_Amp_Factor))\
		- ((BDataTypeDef.Q*Meter_K*fabs(PhaseB.I_Amp_Factor)*fabs(PhaseB.V_Amp_Factor))*PhaseB.Ph_compensation);
	BDataTypeDef.Rs = BDataTypeDef.S / 256.0;
	BDataTypeDef.Rurms = BDataTypeDef.URms / 8192.0*PhaseB.V_Amp_Factor;   //   2^10/2^23
	BDataTypeDef.Rirms = BDataTypeDef.IRms / 8192.0*PhaseB.I_Amp_Factor;
	if (BDataTypeDef.Pf > 0x800000) {
		BDataTypeDef.Rpf = 0x1000000 - BDataTypeDef.Pf;
		BDataTypeDef.Rpf = -(BDataTypeDef.Rpf / 8388608.0);
	}
	else
		BDataTypeDef.Rpf = BDataTypeDef.Pf / 8388608.0;
	BDataTypeDef.Rfreq = BDataTypeDef.Freq / 8192.0;

	output_att7022(BDataTypeDef);
}

void Read_ATT_CData(void)
{
	CDataTypeDef.P = SPI_ATT_Read(r_Pc);
	CDataTypeDef.Q = SPI_ATT_Read(r_Qc);
	CDataTypeDef.S = SPI_ATT_Read(r_Sc);
	CDataTypeDef.URms = SPI_ATT_Read(r_UcRms);
	CDataTypeDef.IRms = SPI_ATT_Read(r_IcRms);
	CDataTypeDef.Pf = SPI_ATT_Read(r_Pfc);
	CDataTypeDef.Freq = SPI_ATT_Read(r_Freq);
	if (CDataTypeDef.P > 0x800000) {
		CDataTypeDef.Rp = 0x1000000 - CDataTypeDef.P;
		CDataTypeDef.Rp = -((CDataTypeDef.Rp*Meter_K*fabs(PhaseC.I_Amp_Factor)*fabs(PhaseC.V_Amp_Factor))\
			- ((CDataTypeDef.Rp*Meter_K*fabs(PhaseC.I_Amp_Factor)*fabs(PhaseC.V_Amp_Factor))*PhaseC.P_Gain_compensation));    //   2^15/2^23
	}
	else
		CDataTypeDef.Rp = (CDataTypeDef.P*Meter_K*fabs(PhaseC.I_Amp_Factor)*fabs(PhaseC.V_Amp_Factor))\
		- ((CDataTypeDef.P*Meter_K*fabs(PhaseC.I_Amp_Factor)*fabs(PhaseC.V_Amp_Factor))*PhaseC.P_Gain_compensation);
	if (CDataTypeDef.Q > 0x800000) {
		CDataTypeDef.Rq = 0x1000000 - CDataTypeDef.Q;
		CDataTypeDef.Rq = -((CDataTypeDef.Rq*Meter_K*fabs(PhaseC.I_Amp_Factor)*fabs(PhaseC.V_Amp_Factor))\
			- ((CDataTypeDef.Rq*Meter_K*fabs(PhaseC.I_Amp_Factor)*fabs(PhaseC.V_Amp_Factor))*PhaseC.Ph_compensation));
	}
	else
		CDataTypeDef.Rq = (CDataTypeDef.Q*Meter_K*fabs(PhaseC.I_Amp_Factor)*fabs(PhaseC.V_Amp_Factor))\
		- ((CDataTypeDef.Q*Meter_K*fabs(PhaseC.I_Amp_Factor)*fabs(PhaseC.V_Amp_Factor))*PhaseC.Ph_compensation);
	CDataTypeDef.Rs = CDataTypeDef.S / 256.0;
	CDataTypeDef.Rurms = CDataTypeDef.URms / 8192.0*PhaseC.V_Amp_Factor;   //   2^10/2^23
	CDataTypeDef.Rirms = CDataTypeDef.IRms / 8192.0*PhaseC.I_Amp_Factor;
	if (CDataTypeDef.Pf > 0x800000) {
		CDataTypeDef.Rpf = 0x1000000 - CDataTypeDef.Pf;
		CDataTypeDef.Rpf = -(CDataTypeDef.Rpf / 8388608.0);
	}
	else
		CDataTypeDef.Rpf = CDataTypeDef.Pf / 8388608.0;
	CDataTypeDef.Rfreq = CDataTypeDef.Freq / 8192.0;

	output_att7022(CDataTypeDef);
}

void SPI_ATT_Write(uint8_t com_add, uint32_t data2) {
	Clr_CS;
	com_add |= 0x80; // Set write bit
	data2 |= (uint32_t)com_add << 24;
	Spi->write32(data2);
	Set_CS;
}

uint32_t SPI_ATT_Read(uint8_t data) {
	Clr_CS;
	Spi->write(data);
	uint32_t ret = 0;
	Spi->transferBits(0, &ret, 24);
	Set_CS;
	return ret;
}

void loop()
{
	delay(500);
	Read_ATT_CData();
}