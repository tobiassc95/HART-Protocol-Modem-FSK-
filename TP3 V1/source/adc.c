#include "adc.h"
#include "hardware.h"
#include "DMA.h"
#include "portpin.h"

#define voidPtr 0
#define PTC8 PORTNUM2PIN(PORT_C, 8)

static ADC_Type* adc[2] = {ADC0, ADC1};
static uint16_t adc0Data = 0; //el dato a guardar en memoria a traves de la dma.
static uint8_t dataRdy = FALSE; //flag para mandar el  valor.

void ADC_pisrADC();

void ADCconfigure (uint8_t adc_, uint8_t mode, uint8_t clkdiv, uint8_t sampt) {
	//CLOCK GATING
	if(adc_ == ADC_0)
		SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;
	else if(adc_ == ADC_1)
		SIM->SCGC3 |= SIM_SCGC3_ADC1_MASK;

	//CONFIG
	adc[adc_]->CFG1 = ADC_CFG1_ADIV(clkdiv) | ADC_CFG1_MODE(mode) | ADC_CFG1_ADICLK(1); //ADICLK can be 0(BUSCLOCK) or 1(BUSCLOCK/2).
	adc[adc_]->SC2 = ADC_SC2_ADTRG(0);
	if(sampt != ADC_4CYCLES) {
		adc[adc_]->CFG1 |= ADC_CFG1_ADLSMP(1);
		adc[adc_]->CFG2 = ADC_CFG2_ADLSTS(sampt);
	}
}

void ADCstart (uint8_t adc_, uint8_t ch_) {
	//adc[adc_]->CFG2 = (adc[adc_]->CFG2 & ~ADC_CFG2_MUXSEL_MASK) | ADC_CFG2_MUXSEL(0); //???
	adc[adc_]->SC1[0] = ADC_SC1_DIFF(0) | ADC_SC1_ADCH(ch_) | ADC_SC1_AIEN_MASK;
}

void ADCstop (uint8_t adc_) {
	adc[adc_]->SC1[0] |= ADC_SC1_ADCH(0x1F);
}

void enableADCinterrupt(uint8_t adc_) {
	if(adc_ == ADC_0)
		NVIC_EnableIRQ(ADC0_IRQn);
	else if(adc_ == ADC_1)
		NVIC_EnableIRQ(ADC1_IRQn);
}

void disableADCinterrupt(uint8_t adc_) {
	if(adc_ == ADC_0)
		NVIC_DisableIRQ(ADC0_IRQn);
	else if(adc_ == ADC_1)
		NVIC_DisableIRQ(ADC1_IRQn);
}

uint16_t getADCdata(uint8_t adc_) {
	if(adc[adc_]->SC2 & ADC_SC2_DMAEN_MASK) //si se usa el dma, se devuelve el dato guardado en memoria.
		return (adc0Data);
	return adc[adc_]->R[0];
}

uint8_t ADCdata2read(uint8_t adc_) {
	if(adc[adc_]->SC2 & ADC_SC2_DMAEN_MASK) {
		if (dataRdy == TRUE) {
			dataRdy = FALSE;
			return(TRUE);
		}
	}
	else {
		if(adc[adc_]->SC1[0] & ADC_SC1_COCO_MASK)
			return (adc[adc_]->SC1[0] & ADC_SC1_COCO_MASK)>>ADC_SC1_COCO_SHIFT;
	}
	return(FALSE);
}

void ADC_enableDMA(uint8_t adc_, uint8_t dmaCh) {
	adc[adc_]->SC2 |= ADC_SC2_DMAEN_MASK;    // DMA Enable
	// DMA configuration for ADC, data is an unsigned 16-bit variable for data receive.
	DMA_dataTransfer(2, (uint32_t)(ADC0->R), (uint32_t)(&adc0Data), SIZE_8, 2, kDmaRequestMux0ADC0, FALSE);
	DMA_dataTransferInterrupt(2,ADC_pisrADC,voidPtr);
}

void ADC_pisrADC() {
#ifdef DEBUG_
	PINwrite(PORTNUM2PIN(PORT_C,8),0);
#endif

	dataRdy = TRUE;

#ifdef DEBUG_
	PINwrite(PORTNUM2PIN(PORT_C,8),1);
#endif
}

//__ISR__ ADC0_IRQHandler(void) {
//	NVIC_DisableIRQ(ADC0_IRQn);
//
//	if(adc[0]->SC1[0] & ADC_SC1_COCO_MASK)
//
//	NVIC_EnableIRQ(ADC0_IRQn);
//}
