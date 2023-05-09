/*
 * DMA.h
 *
 *  Created on: 26 oct. 2018
 *      Author: Ramiro Chiocci
 */

#ifndef eDMA_H_
#define eDMA_H_

#include "stdint.h"
#include"hardware.h"

#ifndef LOGIC
#define LOGIC
typedef enum {
	FALSE = (uint8_t)0,
	TRUE = (uint8_t)1,
} LOGIC_t;
#endif

typedef struct
{
	uint32_t SADDR;
	uint16_t SOFF;
	uint16_t ATTR;
	union
	{
		uint32_t NBYTES_MLNO;
		uint32_t NBYTES_MLOFFNO;
		uint32_t NBYTES_MLOFFYES;
	};
	uint32_t SLAST;
	uint32_t DADDR;
	uint16_t DOFF;
	union
	{
		uint16_t CITER_ELINKNO;
		uint16_t CITER_ELINKYES;
	};
	uint32_t DLASTSGA;
	uint16_t CSR;
	union
	{
		uint16_t BITER_ELINKNO;
		uint16_t BITER_ELINKYES;
	};
}TCD_t;

typedef enum {
	SIZE_8 = (uint8_t) 0,
	SIZE_16,
	SIZE_32,
	//SIZE_16_BURST = (uint8_t) 4,
	//SIZE_32_BURST = (uint8_t) 5,
}DMA_Data_Size_t;

typedef enum {
	MLNO,
	MLOFFNO,
	MLOFFYES,
}NBYTE_t;

enum {
	HALF_INT,
	DONE_INT,
};

typedef TCD_t * (TCD_ptr);

typedef void (*callback_pt) ();
void eDMA_Init();
void eDMA_TCDConfiguration (uint8_t ch, TCD_ptr cfg, DMA_Data_Size_t sourceSize, DMA_Data_Size_t destSize, NBYTE_t type, LOGIC_t elinkState);
void eDMA_enableInterruptMode(uint8_t ch, callback_pt halfFunc, callback_pt doneFunc);
void eDMA_ChannelStartReq(uint8_t chDMA);
void eDMA_ChannelStopReq(uint8_t chDMA);
void eDMA_selectMuxReq(uint8_t ch, dma_request_source_t source, LOGIC_t triggerState);
void eDMA_StartChannel(uint8_t ch);
void eDMA_setMinorOff(uint8_t ch, int16_t offSrc, int16_t offDest);
#endif /* DMA_H_ */
