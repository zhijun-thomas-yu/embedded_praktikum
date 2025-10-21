/*
 * rfm69c.h
 *
 *  Created on: 24.01.2018
 *      Author: p7
 */

#ifndef RFM69C_H

#define RFM69C_H

//#include "spi.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "spi.h"



// Operating modes

enum { RX, TX, STANDBY };


// Data structure for received packets

typedef struct {

	uint8_t sender;

	bool isBroadcast;

	char messageText[17];

} Packet;

bool testSPI(void);

/**

 * Initialization. Must be invoked before any other function of this module.

 * After initialization the RFM69 is in standby state.

 *

 * Input

 * p_out: output power in [dBm] between -18 and 13

 * nodeAddress: address of the RFM module used for Rx and Tx

 *

 * Output

 * returns true if everything is set up correctly (especially SPI)

 */

bool RFM_Initialize(const int8_t p_out, const uint8_t nodeAddress);


/**

 * Sends a (max. 16 character) text message to 'recipient'.

 */

void RFM_Send(const uint8_t recipient, const char *payload);


/**

 * Returns the most recently received packet. The payload has a maximum length

 * of 16 characters and is always null-terminated.

 */

Packet RFM_GetRxPacket(void);


/**

 * The following functions switch the RFM69's operation mode and block until mode

 * is ready.

 */

void RFM_RxMode(void);

void RFM_TxMode(void);

void RFM_StandbyMode(void);


/**

 * Returns RX, TX or STANDBY

 */

uint8_t RFM_GetMode(void);


/**

 * Returns the node's address

 */

uint8_t RFM_GetNodeAddress(void);


/**

 * Yields the current input signal strength in [dBm]

 */

int8_t RFM_RSSIValue(void);


/**

 * Reads the byte from given address from the transceiver.

 * Can be used for debugging purposes, e.g. checking status...

 */

uint8_t RFM_ReadByte(const uint8_t address);



#endif /* RFM69C_H */
