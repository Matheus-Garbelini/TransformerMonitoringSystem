#pragma once
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

#define HFconst	0xA0   //¸ßÆµÊä³ö²ÎÊý
#define UADC	0xBF   //µçÑ¹Í¨µÀÔöÒæ
#define UgainA	0x9B
#define UgainB	0x9C
#define UgainC	0x9D

SPIClass *Spi = NULL;
//
//uint32_t SPI_ATT_Read(uint8_t data)
//{
//	uint8_t i;
//	uint32_t temp = 0;
//	Set_CS;
//	Clr_SCL;
//	delayMicroseconds(50);
//	Clr_CS;
//	for (i = 0; i < 8; i++)
//	{
//		Set_SCL;
//		delay_us(50);
//		if (data & 0x80)
//			Set_DATA;
//		else
//			Clr_DATA;
//		delay_us(50);
//		Clr_SCL;
//		delay_us(50);
//		data <<= 1;
//	}
//	delay_us(50);
//	for (i = 0; i < 24; i++)
//	{
//		temp <<= 1;
//		Set_SCL;
//		delay_us(50);
//		if (Rd_MISO)
//			temp |= 0x01;
//		Clr_SCL;
//		delay_us(50);
//	}
//	Set_CS;
//	return (temp);
//}
//
//void SPI_ATT_Write(uint8_t com_add, uint32_t data2)
//{
//	uint8_t i, data1;
//	data1 = 0x80 | com_add;
//	Set_CS;
//	Clr_SCL;
//	Clr_CS;
//	for (i = 0; i < 8; i++)
//	{
//		Set_SCL;
//		delay_us(50);
//		if (data1 & 0x80)
//			Set_DATA;
//		else
//			Clr_DATA;
//		delay_us(3);
//		Clr_SCL;
//		delay_us(50);
//		data1 <<= 1;
//	}
//	for (i = 0; i < 24; i++)
//	{
//		Set_SCL;
//		delay_us(50);
//		if (data2 & 0x00800000)
//			Set_DATA;
//		else
//			Clr_DATA;
//		delay_us(3);
//		Clr_SCL;
//		delay_us(50);
//
//		data2 <<= 1;
//	}
//	Set_CS;
//}

void ATT_Adjust(void)
{
	uint32_t read1 = 0x55;

	SPI_ATT_Write(0xC3, 0x000000);
	delay(200);
	SPI_ATT_Write(0xC9, 0x00005A);

	SPI_ATT_Write(0x01, 0xB9FE);
	SPI_ATT_Write(0x03, 0xF804);
	SPI_ATT_Write(0x31, 0x3437);
	SPI_ATT_Write(0x02, 0x0000);

	SPI_ATT_Write(0x6D, 0xFF00);
	SPI_ATT_Write(0x6E, 0x0DB8);
	SPI_ATT_Write(0x6F, 0xD1DA);

	SPI_ATT_Write(UADC, 0x000001);
	SPI_ATT_Write(HFconst, 0x0005E7);
	///*----------------------------------------
	//
	//-----------------------------------------*/
	SPI_ATT_Write(UgainA, 0x000000);
	SPI_ATT_Write(UgainB, 0x000000);
	SPI_ATT_Write(UgainC, 0x8172F5);	//8483573

	SPI_ATT_Write(0xC9, 0x000000);

	SPI_ATT_Write(0xC6, 0x00005A);
	read1 = SPI_ATT_Read(0x00);
	printf("\r\nIn 0xC6 with 0x5A: 0x00 is %x !\r\n", read1);
	read1 = 0x55;

	read1 = SPI_ATT_Read(0x01);
	printf("\r\nModeCfg is %x !\r\n", read1);
	read1 = 0x55;

	read1 = SPI_ATT_Read(0x03);
	printf("\r\nEMUCfg is %x !\r\n", read1);
	read1 = 0x55;

	read1 = SPI_ATT_Read(0x31);
	printf("\r\nModuleCfg is %x !\r\n", read1);
	read1 = 0x55;

	SPI_ATT_Write(0xC6, 0x000000);		//¼ÆÁ¿Êý¾Ý¶Á³öÊ¹ÄÜ
	read1 = SPI_ATT_Read(0x00);
	printf("\r\nIn 0xC6 without 0x5A: 0x00 is %x !\r\n", read1);
	read1 = SPI_ATT_Read(0x38);
	printf("\r\n%x !\r\n", read1);
}

void setup()
{
	Serial.begin(115200);

	// TODO: att7022 tests here

	Spi = new SPIClass(HSPI);
	Spi->begin(CLK_PIN, DIN_PIN, DATA_PIN, CS_PIN);
	Spi->setHwCs(false);
	Spi->setFrequency(10000);
	Spi->setBitOrder(MSBFIRST);
	Spi->setDataMode(SPI_MODE0);
	pinMode(CS_PIN, OUTPUT);
	pinMode(23, OUTPUT);
	Set_CS;
	digitalWrite(23, HIGH);
};

void SPI_ATT_Write(uint8_t com_add, uint32_t data2) {
	Clr_CS;
	Spi->write(com_add);
	delayMicroseconds(50);
	Spi->transferBits(data2, NULL, 24);
	Set_CS;
}

uint32_t SPI_ATT_Read(uint8_t data) {
	Clr_CS;
	delayMicroseconds(1);
	Spi->write(data);
	delayMicroseconds(50);
	uint32_t ret = 0;
	Spi->transferBits(0, &ret, 24);
	Set_CS;
	return ret;
}

void loop()
{
	ATT_Adjust();
	delay(2000);
}