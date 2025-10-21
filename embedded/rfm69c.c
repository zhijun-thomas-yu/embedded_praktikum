/*
 * rfm69c.c
 *
 *  Created on: 24.01.2018
 *      Author: p7
 */


#include "rfm69c.h"


/*

 * Pin config:

 * NSS - PB4 (slave select)

 */



#define MAX_TEXT_LENGTH		16

#define PAYLOAD_SIZE		MAX_TEXT_LENGTH + 2

#define BROADCAST_ADDR		0xFF


#define FREQ_SET_315		{ 0x4E, 0xC0, 0x00 }

#define FREQ_SET_434		{ 0x6C, 0x80, 0x00 }

//#define FREQ_SET_868		{ 0xD9, 0x73, 0x33 }
//#define FREQ_SET_868		{ 0xD9, 0x77, 0x0A }
#define FREQ_SET_868		{ 0xD9, 0x79, 0x99 }
// #define FREQ_SET_868		{ 0xD9, 0x74, 0x7A }
//#define FREQ_SET_868		{ 0xD9, 0x75, 0xC3 }  // Unsere Freuqenz von 869,84 MHz


#define FREQ_SET_915		{ 0xE4, 0xC0, 0x00 }



/// SPI handlers

void writeByte(const uint8_t address, uint8_t data);

void write16(const uint8_t address, const uint16_t data);

void writeBurst(const uint8_t address, uint8_t *data, const uint8_t length);


uint8_t readByte(const uint8_t address);

void readBurst(const uint8_t address, uint8_t *readBuffer, const uint8_t length);


bool testSPI(void);



uint8_t id;

uint8_t currentMode;



bool RFM_Initialize(const int8_t p_out, const uint8_t nodeAddress) {

	printf("rfm initialize start");
	SPI_Initialize();
    printf("SPI Test returned: %d\n", testSPI());


	sleep_ms(50);


/*
	if (!testSPI()) {

		// clear FIFO

		RFM_RxMode();

		RFM_StandbyMode();

		if (!testSPI()) return false;

	}
	*/

	// carrier frequency: 868 MHz

	uint8_t freqVal[3] = FREQ_SET_868;

	writeBurst(0x07, freqVal, 3);



	// PA0 output power, PA1 & PA2 off

	uint8_t pVal;

	if (p_out <= -18) pVal = 0;

	else if (p_out >= 13) pVal = 13+18;

	else pVal = p_out + 18;

	writeByte(0x11, (pVal & 0x1F) | 0x80);



	// FSK modulation, packet mode

	writeByte(0x02, 0x00);



	// disable OCP at first

	writeByte(0x13, 0x0F);



	// LNA Z_in = 50 ohms

	writeByte(0x18, 0x00); // def: 0x80



	// RxBw: 83 KHz

	writeByte(0x19, 0x52);



	// default settings for RegAfcBw

	writeByte(0x1A, 0x8B);



	// Automatic frequency control settings

	writeByte(0x1E, 0x0E);



	// don't use an output pin for the clock signal (default)

	writeByte(0x26, 0x07);



	// default settings for RegRssiThresh

	writeByte(0x29, 0xE4);



	// preamble size: 500 bytes

	write16(0x2C, 500);
	//writeByte(0x2C, 0x00);
	//writeByte(0x2D, 0x0C);


	// sync word size: 2 bytes

	writeByte(0x2E, 0x88);



	// set sync word

	write16(0x2F, 0x9AD2);



	// Manchester encoding, CRC check on, address filtering on

	writeByte(0x37, 0x34);



	// set payload length

	writeByte(0x38, PAYLOAD_SIZE);



	// set node address

	writeByte(0x39, nodeAddress);

	id = nodeAddress;



	// set broadcast address

	writeByte(0x3A, BROADCAST_ADDR);



	// FifoThreshold: 15, TxStartCondition: FifoLevel

	writeByte(0x3C, (15 & 0x7F));



	// use only PA0

	writeByte(0x5A, 0x55);

	writeByte(0x5C, 0x70);



	// enable improved DAGC

	writeByte(0x6F, 0x30);



	// enable OCP

	writeByte(0x13, 0x10);



	// TEST: continuous 868 MHz sine

	//writeByte(0x02, 0x48);



	// clear FIFO

	RFM_RxMode();

	RFM_StandbyMode();


	printf("rfm initialize end");
	return true;

}


void RFM_Send(const uint8_t recipient, const char *message) {

	// construct the payload

	uint8_t payload[PAYLOAD_SIZE];

	payload[0] = recipient;

	payload[1] = id;



	for (uint8_t i = 0; i < MAX_TEXT_LENGTH; i++) {

		char temp = message[i];

		payload[i + 2] = (uint8_t) temp;

		if (temp == '\0') break;

	}



	// write into FIFO

	writeBurst(0x00, payload, PAYLOAD_SIZE);

}


Packet RFM_GetRxPacket(void) {

	static Packet ret;



	// the received packet is stored in the RFM69's FIFO (address 0x00)

	ret.isBroadcast = (readByte(0x00) == BROADCAST_ADDR);

	ret.sender = readByte(0x00);



	char msg[MAX_TEXT_LENGTH + 1];

	readBurst(0x00, msg, MAX_TEXT_LENGTH);

	msg[MAX_TEXT_LENGTH] = '\0';


	uint8_t i;
	for ( i= 0; i < MAX_TEXT_LENGTH + 1; i++)

		ret.messageText[i] = msg[i];



	return ret;

}


void RFM_RxMode(void) {

	if (!SPI_isEnabled()) return;



	// go into Rx mode

	writeByte(0x01, 0x10);



	// pin mapping: PayloadReady -> DIO0

	writeByte(0x25, 0x44);



	// wait until ready

	uint8_t status;
	do { status = readByte(0x27); }
	while (((status & 0xC0) != 0xC0) && (status != 0xFF));
	while (! (readByte(0x1E) & 0x10));



	currentMode = RX;



	// start RSSI measurement

	writeByte(0x23, 1);

}


void RFM_TxMode(void) {

	if (!SPI_isEnabled()) return;



	// go into Tx mode

	writeByte(0x01, 0x0C);



	// pin mapping: PacketSent -> DIO0

	writeByte(0x25, 0x04);



	// wait until ready

	uint8_t status;

	do { status = readByte(0x27); }

	while (((status & 0xA0) != 0xA0) && (status != 0xFF));



	currentMode = TX;

}


void RFM_StandbyMode(void) {

	if (!SPI_isEnabled()) return;



	writeByte(0x01, 0x04);

	while ((readByte(0x27) & 0x80) != 0x80);



	currentMode = STANDBY;

}


uint8_t RFM_GetMode(void) {

	return currentMode;

}


int8_t RFM_RSSIValue(void) {

	if (!SPI_isEnabled()) return -128;



	// start RSSI measurement

	writeByte(0x23, 1);



	sleep_ms(5);



	// read RSSI value

	uint8_t rawRssi = readByte(0x24) / 2;

	return -rawRssi;

}


uint8_t RFM_GetNodeAddress(void) {

	return id;

}


uint8_t RFM_ReadByte(const uint8_t address) {

	return readByte(address);

}



/// helper functions (SPI)


void writeByte(const uint8_t address, uint8_t data) {

	SPI_write(address, &data, 1);

}


void write16(const uint8_t address, const uint16_t data) {

	uint8_t temp[2] = { data >> 8, data & 0xFF };

	SPI_write(address, temp, 2);

}


void writeBurst(const uint8_t address, uint8_t *data, const uint8_t length) {

	SPI_write(address, data, length);

}


uint8_t readByte(const uint8_t address) {
	
	uint8_t ret;

	SPI_read(address, &ret, 1);

	return ret;

}


void readBurst(uint8_t address, uint8_t *readBuffer, uint8_t length) {

	SPI_read(address, readBuffer, length);

}


bool testSPI(void) {
	


	uint8_t testData[16]       = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

	const uint8_t compData[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };



	// write test data into FIFO

	writeBurst(0x00, testData, 16);



	// read data from FIFO

	readBurst(0x00, testData, 16);


	
	for (uint8_t i = 0; i < 16; i++) {
		//printf("soll:ist \t%d:%d\n", compData[i],testData[i]);
		if (compData[i] != testData[i]){
			//printf("Error testing SPI: mismatch: %d : %d\n",compData[i],testData[i]);
			return false;
		}
	}

	//printf("Successfully Initialized SPI Communication\n");

	return true;

}
