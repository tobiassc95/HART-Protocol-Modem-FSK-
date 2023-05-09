/*
 * DMA.h
 *
 *  Created on: 25 oct. 2019
 *      Author: root
 */

#ifndef DMA_H_
#define DMA_H_
#include "eDMA.h"
typedef void (*funPtr) (void);

void DMA_Init();
// Funtion DMA_dataTransfer
// Transfer a data unit specified in dataSize parameter through a dma request
// Input: ch a DMA channel, sourceAdd is the source pointer, destAdd is the address pointer, dataSize specified the unit memory length to be copied,
// dataNumber is the number of unit memory copy to be copied, req is the dma request source number, periodicMode specified PIT autotrigger.
void DMA_dataTransfer(uint8_t ch, uint32_t sourceAdd, uint32_t destAdd, DMA_Data_Size_t dataSize, uint16_t dataNumber, dma_request_source_t req, LOGIC_t periodicMode);

// Funtion DMA_PiodicDataTransfer
// Transfer a data unit specified in dataSize parameter through a dma request
// Input: ch a DMA channel, sourceAdd is the source pointer, destAdd is the address pointer, dataSize specified the unit memory length to be copied,
// dataNumber is the number of unit memory copy to be copied, req is the dma request source number, periodicMode specified PIT autotrigger, minorOffSrc and minorOffDest is the minor offset aplied to address for each minor loop, majorOffSrc and majorOffDest is the major offset aplied to address for each major loop
void DMA_PeriodicDataTransfer(uint8_t ch, uint32_t sourceAdd, uint32_t destAdd, DMA_Data_Size_t dataType, uint16_t dataSize,
		uint16_t dataNumber, dma_request_source_t req, LOGIC_t periodicMode, int16_t minorOffSrc, int16_t minorOffDest,
		signed long majorOffSrc, signed long majorOffDest);
void DMA_dataTransferInterrupt(uint8_t ch, funPtr doneIntFunc, funPtr halfIntFunc);
#endif /* DMA_H_ */
