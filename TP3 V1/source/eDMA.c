/*
 * DMA.c
 *
 *  Created on: 26 oct. 2018
 *      Author: Ramiro Chiocci
 */

#include"hardware.h"
#include "eDMA.h"

//#define TRUE 1
//#define FALSE 0
#define nullPtr 0
#define NUM_DMA_CH 16
__ISR__ DMA0_IRQHandler(void);
#define eDMA_SETCLOCKSTATE(state) \
/* Enable the clock for the eDMA and the DMAMUX. */ \
	SIM->SCGC7 = (SIM->SCGC7 & ~SIM_SCGC7_DMA_MASK) | SIM_SCGC7_DMA(state);\
	SIM->SCGC6 = (SIM->SCGC6 & ~SIM_SCGC6_DMAMUX_MASK) | SIM_SCGC6_DMAMUX(state);
callback_pt callback_table[NUM_DMA_CH][2];
void eDMA_Init()
{
	eDMA_SETCLOCKSTATE(TRUE);
	(DMA0->CR) = 0x0000;
}

//void DMA_DataTransfer (uint8_t ch, uint32_t sourceAddress, uint32_t destinationAddress, uint16_t dataNumber, dataTransferSize bt, uint8_t sizeBytes, uint32_t sourceoff, uint32_t destoff, int slast, int dlast, uint16_t numMajorCycles)
void eDMA_TCDConfiguration (uint8_t ch, TCD_ptr cfg, DMA_Data_Size_t sourceSize, DMA_Data_Size_t destSize, NBYTE_t type, LOGIC_t elinkState)
{

	(DMA0->TCD)[ch].CSR = ((DMA0->TCD)[ch].CSR & ~DMA_CSR_DONE_MASK) | DMA_CSR_DONE(FALSE);  	// Clear Channel Done flag

	//DMA0->SEEI = DMA_SEEI_SEEI(ch); // Enable error interrupt in channel

	if (cfg->ATTR == 0x00) {
		(DMA0->TCD)[ch].ATTR = DMA_ATTR_SMOD(0)  |              						// Source address modulo feature is disabled
							DMA_ATTR_SSIZE(sourceSize) |              					// Source data transfer size: 1: 16-bit, 2=32-bit
							DMA_ATTR_DMOD(0)  |              								// Destination address modulo feature: 0=disabled, x= x power of 2 buffer[DMOD=4->buffer of 16bytes]
							DMA_ATTR_DSIZE(destSize);               						// Destination data transfer size: 1: 16-bit, 2=32-bit
	}
	else
		(DMA0->TCD)[ch].ATTR = cfg->ATTR;
	switch (type) {																		// Minor Byte Transfer Count
	case MLNO:
		DMA0->CR = (DMA0->CR & ~DMA_CR_EMLM_MASK) | DMA_CR_EMLM(FALSE);
		(DMA0->TCD)[ch].NBYTES_MLOFFNO = DMA_NBYTES_MLNO_NBYTES(cfg->NBYTES_MLNO);
		break;
	case MLOFFNO:
		DMA0->CR = (DMA0->CR & ~DMA_CR_EMLM_MASK) | DMA_CR_EMLM(TRUE);
		(DMA0->TCD)[ch].NBYTES_MLOFFNO = DMA_NBYTES_MLOFFNO_SMLOE(FALSE) | DMA_NBYTES_MLOFFNO_DMLOE(FALSE) | DMA_NBYTES_MLOFFNO_NBYTES(cfg->NBYTES_MLNO);
		break;
	case MLOFFYES:
		DMA0->CR = (DMA0->CR & ~DMA_CR_EMLM_MASK) | DMA_CR_EMLM(TRUE);
		(DMA0->TCD)[ch].NBYTES_MLOFFYES = DMA_NBYTES_MLOFFYES_NBYTES(cfg->NBYTES_MLNO);
		break;
	}
	
	(DMA0->TCD)[ch].SLAST = DMA_SLAST_SLAST(cfg->SLAST);								// Last Source Address Adjustment
	(DMA0->TCD)[ch].DADDR = DMA_DADDR_DADDR(cfg->DADDR);     						// Destination Address
	(DMA0->TCD)[ch].DOFF = DMA_DOFF_DOFF(cfg->DOFF);                					// Destination Address Signed Offset
	(DMA0->TCD)[ch].SADDR = DMA_SADDR_SADDR(cfg->SADDR);  							// Source Address
	(DMA0->TCD)[ch].SOFF = DMA_SOFF_SOFF(cfg->SOFF);	                				// Source Offset
	
	if (elinkState == TRUE) {
		(DMA0->TCD)[ch].CITER_ELINKYES = DMA_CITER_ELINKYES_CITER(cfg->CITER_ELINKYES)
										| DMA_CITER_ELINKYES_ELINK(TRUE);
		(DMA0->TCD)[ch].BITER_ELINKYES = DMA_BITER_ELINKYES_BITER(cfg->BITER_ELINKYES)
										| DMA_BITER_ELINKYES_ELINK(TRUE);
	}
	else {
		(DMA0->TCD)[ch].CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(cfg->CITER_ELINKNO)  		// Current Major Iteration Count is 1
			                          | DMA_CITER_ELINKNO_ELINK(FALSE);  	// The channel-to-channel linking is disabled
		(DMA0->TCD)[ch].BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(cfg->BITER_ELINKNO)  		// Current Major Iteration Count is 1
				                      | DMA_BITER_ELINKNO_ELINK(FALSE);  	// The channel-to-channel linking is disabled
	}

	(DMA0->TCD)[ch].DLAST_SGA = DMA_DLAST_SGA_DLASTSGA(cfg->DLASTSGA);				// Destination last address adjustment is -16
	if (cfg->CSR != 0x0000)
		(DMA0->TCD)[ch].CSR = cfg->CSR;
	else {
		(DMA0->TCD)[ch].CSR = DMA_CSR_BWC(0)      |	         				// BWC=0: No eDMA engine stalls - full bandwidth
						  DMA_CSR_MAJORELINK(0)  |        					// The channel-to-channel linking is disabled
						  DMA_CSR_MAJORLINKCH(0) |       					// channel 1 will be called from ch0
						  DMA_CSR_ESG(0)         |         					// The current channel�fs TCD is normal format - No scatter/gather
						  DMA_CSR_DREQ(0)        |         					// The channel's ERQ bit is not affected
						  DMA_CSR_INTHALF(FALSE) |         					// The half-point interrupt is disabled
						  DMA_CSR_INTMAJOR(FALSE) |         					// The end-of-major loop interrupt is enabled
						  DMA_CSR_START(FALSE);                				// The channel is not explicitly started
	}

	DMA0->CINT = DMA_CINT_CINT(ch);

	// Setup control and status register
	/* Enable the interrupts for the channel 0. */
	/* Clear all the pending events. */
	NVIC_ClearPendingIRQ((IRQn_Type)(DMA0_IRQn+(int)ch));
	/* Enable the DMA interrupts. */
	NVIC_EnableIRQ((IRQn_Type)(DMA0_IRQn+(int)ch));
}
//Send a software start request for direct copy without another module
void eDMA_StartChannel(uint8_t ch)
{
	if (ch < 16)
	{
		(DMA0->SSRT) = ((DMA0->SSRT) & ~DMA_SSRT_SSRT_MASK) | DMA_SSRT_SSRT(ch);
	}
}

//Enabled the channer request for dma operation
void eDMA_ChannelStartReq(uint8_t chDMA)
{
	DMA0->SERQ = (DMA0->SERQ & ~DMA_SERQ_SERQ_MASK) | DMA_SERQ_SERQ(chDMA) ;
}

//Disabled the channel request for dma operation
void eDMA_ChannelStopReq(uint8_t chDMA)
{
	DMA0->CERQ = (DMA0->CERQ & ~DMA_CERQ_CERQ_MASK) | DMA_CERQ_CERQ(chDMA) ;
}

void eDMA_selectMuxReq(uint8_t ch, dma_request_source_t source, LOGIC_t triggerState)
{
	// Enable Channel 0 and set ADC0 as DMA request source
	DMAMUX->CHCFG[ch] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_TRIG(triggerState) | DMAMUX_CHCFG_SOURCE(source);
}

void eDMA_setMinorOff(uint8_t ch, int16_t offSrc, int16_t offDest)
{
	eDMA_ChannelStopReq(ch);
	(DMA0->TCD)[ch].CSR = ((DMA0->TCD)[ch].CSR & ~DMA_CSR_DONE_MASK) | DMA_CSR_DONE(FALSE);  	// Clear Channel Done flag
	if (offDest != 0)
		(DMA0->TCD)[ch].DOFF = (DMA0->TCD)[ch].NBYTES_MLNO * offDest;
	if (offSrc != 0)
		(DMA0->TCD)[ch].SOFF = (DMA0->TCD)[ch].NBYTES_MLNO * offSrc;
	eDMA_ChannelStartReq(ch);
}

void eDMA_enableInterruptMode(uint8_t ch, callback_pt halfFunc, callback_pt doneFunc)
{
	NVIC_DisableIRQ((IRQn_Type)(DMA0_IRQn+(int)ch));
	if (halfFunc != nullPtr) {
		(DMA0->TCD)[ch].CSR |= DMA_CSR_INTHALF(TRUE);
		callback_table[ch][HALF_INT] = halfFunc;
	}
	if (doneFunc != nullPtr) {
		(DMA0->TCD)[ch].CSR |= DMA_CSR_INTMAJOR(TRUE);
		callback_table[ch][DONE_INT] = doneFunc;
	}
	DMA0->CINT = DMA_CINT_CINT(ch);


	// Setup control and status register
	/* Enable the interrupts for the channel 0. */
	/* Clear all the pending events. */
	NVIC_ClearPendingIRQ((IRQn_Type)(DMA0_IRQn+(int)ch));
	/* Enable the DMA interrupts. */
	NVIC_EnableIRQ((IRQn_Type)(DMA0_IRQn+(int)ch));
}

//Interrupts handlers
__ISR__ DMA0_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x00;
	if ((((DMA0->TCD)[0].CSR & DMA_CSR_DONE_MASK) != 0) && ((DMA0->TCD)[0].CSR & DMA_CSR_INTMAJOR_MASK))
		(*(callback_table[0][DONE_INT]))();
	if ((((DMA0->TCD)[0].CSR & DMA_CSR_INTHALF_MASK) != 0) && ((DMA0->TCD)[0].CSR & DMA_CSR_INTHALF_MASK))
		(*(callback_table[0][HALF_INT]))();
	(DMA0->TCD)[0].CSR = ((DMA0->TCD)[0].CSR & ~DMA_CSR_DONE_MASK) | DMA_CSR_DONE(FALSE);  			// Clear Channel Done flag
	(DMA0->TCD)[0].CSR = ((DMA0->TCD)[0].CSR & ~DMA_CSR_INTHALF_MASK) | DMA_CSR_INTHALF(FALSE);  	// Clear Channel Half flag
}
__ISR__ DMA1_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x01;
	if (((DMA0->TCD)[1].CSR & DMA_CSR_DONE_MASK) != 0)
		(*(callback_table[1][DONE_INT]))();
	if (((DMA0->TCD)[1].CSR & DMA_CSR_INTHALF_MASK) != 0)
		(*(callback_table[0][HALF_INT]))();
	(DMA0->TCD)[1].CSR = ((DMA0->TCD)[1].CSR & ~DMA_CSR_DONE_MASK) | DMA_CSR_DONE(FALSE);  			// Clear Channel Done flag
	(DMA0->TCD)[1].CSR = ((DMA0->TCD)[1].CSR & ~DMA_CSR_INTHALF_MASK) | DMA_CSR_INTHALF(FALSE);  	// Clear Channel Half flag
}
__ISR__ DMA2_IRQHandler(void)
{
	/* Clear the interrupt flag. */
		DMA0->CINT |= 0x02;
		if (((DMA0->TCD)[2].CSR & DMA_CSR_DONE_MASK) != 0)
			(*(callback_table[2][DONE_INT]))();
		if (((DMA0->TCD)[2].CSR & DMA_CSR_INTHALF_MASK) != 0)
			(*(callback_table[2][HALF_INT]))();
		(DMA0->TCD)[2].CSR = ((DMA0->TCD)[2].CSR & ~DMA_CSR_DONE_MASK) | DMA_CSR_DONE(FALSE);  			// Clear Channel Done flag
		(DMA0->TCD)[2].CSR = ((DMA0->TCD)[2].CSR & ~DMA_CSR_INTHALF_MASK) | DMA_CSR_INTHALF(FALSE);  	// Clear Channel Half flag
}
__ISR__ DMA3_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x03;
	if (((DMA0->TCD)[3].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[3])();
}
__ISR__ DMA4_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x04;
	if (((DMA0->TCD)[4].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[4])();
}
__ISR__ DMA5_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x05;
	if (((DMA0->TCD)[5].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[5])();
}
__ISR__ DMA6_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x06;
	if (((DMA0->TCD)[6].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[6])();
}
__ISR__ DMA7_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x07;
	if (((DMA0->TCD)[7].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[7])();
}
__ISR__ DMA8_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x08;
	if (((DMA0->TCD)[8].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[8])();
}
__ISR__ DMA9_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x09;
	if (((DMA0->TCD)[9].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[9])();
}
__ISR__ DMA10_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x0A;
	if (((DMA0->TCD)[10].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[10])();
}
__ISR__ DMA11_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x0B;
	if (((DMA0->TCD)[11].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[11])();
}
__ISR__ DMA12_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x0C;
	if (((DMA0->TCD)[12].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[12])();
}
__ISR__ DMA13_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x0D;
	if (((DMA0->TCD)[13].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[13])();
}
__ISR__ DMA14_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x0E;
	if (((DMA0->TCD)[14].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[14])();

}__ISR__ DMA15_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	DMA0->CINT |= 0x0F;
	if (((DMA0->TCD)[15].CSR & DMA_CSR_DONE_MASK) != 0)
		(*callback_table[15])();
}

__ISR__ DMA_Error_IRQHandler(void)
{

}
