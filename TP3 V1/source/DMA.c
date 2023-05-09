/*
 * DMA.c
 *
 *  Created on: 25 oct. 2019
 *      Author: root
 */

#include "DMA.h"
#include "stdint.h"

typedef void (* funPtr) (void);

void DMA_Init()
{
	eDMA_Init();
}

void DMA_dataTransfer(uint8_t ch, uint32_t sourceAdd, uint32_t destAdd, DMA_Data_Size_t dataSize, uint16_t dataNumber, dma_request_source_t req, LOGIC_t periodicMode)
{
	static TCD_t newTcd;
	newTcd.SADDR = sourceAdd;												// Source data address
	newTcd.DADDR = destAdd;													// Destination data address
	newTcd.NBYTES_MLNO = dataNumber*((uint16_t) 1 << dataSize);				// No minor loop mapping mode
	newTcd.SOFF = ((uint16_t) 1) << dataSize;								// Minor loop offset of source address
	newTcd.DOFF = ((uint16_t) 1) << dataSize;								// Minor loop offset of destination address
	newTcd.SLAST = -0x02;													// Final offset to original points
	newTcd.DLASTSGA = -0x02;
	newTcd.CITER_ELINKNO = 0x01;											// Disable channel to channel link and set major counter
	newTcd.BITER_ELINKNO = 0x01;

	eDMA_TCDConfiguration (ch, &newTcd, dataSize, dataSize, MLNO, FALSE);
	eDMA_selectMuxReq(ch, req, periodicMode);
	eDMA_ChannelStartReq(ch);
}


void DMA_PeriodicDataTransfer(uint8_t ch, uint32_t sourceAdd, uint32_t destAdd, DMA_Data_Size_t dataType, uint16_t dataSize,
		uint16_t dataNumber, dma_request_source_t req, LOGIC_t periodicMode, int16_t minorOffSrc, int16_t minorOffDest,
		signed long majorOffSrc, signed long majorOffDest)
{
	static TCD_t newTcd;
	newTcd.SADDR = sourceAdd;
	newTcd.DADDR = destAdd;
	newTcd.NBYTES_MLNO = ((uint16_t) 1 << dataType)*dataSize;
	newTcd.SOFF = (int16_t)((1 << dataType)*dataSize*minorOffSrc);
	newTcd.DOFF = (int16_t)((1 << dataType)*dataSize*minorOffDest);
	newTcd.SLAST = (signed long)((1 << dataType)*dataSize*majorOffSrc);
	newTcd.DLASTSGA = (signed long)((1 << dataType)*dataSize*majorOffDest);

	newTcd.CITER_ELINKNO = dataNumber;
	newTcd.BITER_ELINKNO = dataNumber;

	newTcd.CSR = 0x0000;

	eDMA_TCDConfiguration (ch, &newTcd, dataSize, dataSize, MLNO, FALSE);
	eDMA_selectMuxReq(ch, req, periodicMode);
	eDMA_ChannelStartReq(ch);
}

void DMA_setMinorStep(uint8_t ch, int16_t step, LOGIC_t srcEnable, LOGIC_t destEnable)
{
	eDMA_setMinorOff(ch, step*srcEnable, step*destEnable);
	eDMA_ChannelStartReq(ch);
}

void DMA_dataTransferInterrupt(uint8_t ch, funPtr doneIntFunc, funPtr halfIntFunc)
{
	eDMA_enableInterruptMode(ch, halfIntFunc, doneIntFunc);
}
