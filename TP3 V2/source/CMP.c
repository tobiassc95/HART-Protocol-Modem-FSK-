#include "CMP.h"
#include "systick.h"
#include "portpin.h"
#include "timer.h"
#include "UART.h"

#define DISABLED	0

enum {
	IN0,
	IN1,
	IN2,
	IN3,
	IN4,
	IN5,
	IN6,
	IN7
};

void CMP_Init (void)
{
	SIM->SCGC4 |= SIM_SCGC4_CMP_MASK;				// Se hace el clock gating del comparador


	PINconfigure(PORTNUM2PIN(PORT_C, 2), PIN_MUX0, PIN_IRQ_DISABLE);		// Se configura el mux para que se conecte al CMP1
	PINconfigure(PORTNUM2PIN(PORT_C, 3), PIN_MUX0, PIN_IRQ_DISABLE);
	PINconfigure(PORTNUM2PIN(PORT_C, 4), PIN_MUX6, PIN_IRQ_DISABLE);




	//		SAMPLING MODE

//	CMP1->CR1 |= CMP_CR1_SE_MASK;
//	CMP1->CR0 |= CMP_CR0_FILTER_CNT(7);
//	CMP1->FPR = CMP_FPR_FILT_PER_MASK;


	//		HISTERESIS

	//CMP1->CR0 |= CMP_CR0_HYSTCTR(3);


	//DAC0->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK | DAC_C0_DACTRGSEL_MASK;		// DACEN-> DAC enable | DACRFS-> La tension de fondo de escala es la alimenacion del micro | DACTRGSEL-> Se cambia el valor por software
	//DAC1->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK | DAC_C0_DACTRGSEL_MASK;
}


void CMP_config (CMP_config_t *CMP_config)
{
	CMP1->MUXCR |= CMP_MUXCR_PSEL(IN0);				//La entrada no inversora sera una externa (La 2 del puerto C)

	CMP1->CR1 = CMP_CR1_EN_MASK | CMP_CR1_COS_MASK | CMP_CR1_OPE_MASK;


	if(CMP_config->SE_enable)
		CMP1->CR1 |= CMP_CR1_SE_MASK;
	else if(CMP_config->WE_enable)				// La hago un else if porque no esta bueno tener los 2 al mismo tiempo
		CMP1->CR1 |= CMP_CR1_WE_MASK;

	CMP1->CR0 |= CMP_CR0_FILTER_CNT(CMP_config->FilterCount);
	CMP1->FPR = CMP_FPR_FILT_PER(CMP_config->FilterPeriod);
	CMP1->CR0 |= CMP_CR0_HYSTCTR(CMP_config->hysteresis);

	if(CMP_config->DAC_enable == true)
	{
		CMP1->DACCR = CMP_DACCR_DACEN_MASK;
		CMP1->DACCR |= CMP_DACCR_VRSEL_MASK;
		CMP1->DACCR |= CMP_DACCR_VOSEL((CMP_config->VDD_pecentage)*63/100);
		CMP1->MUXCR |= CMP_MUXCR_MSEL(IN7);			//La entrada inversora del comparador sea el DAC
	}
	else
	{
		CMP1->DACCR = DISABLED;
		//PINmode(PORTNUM2PIN(PORT_C, 3), PIN_INPUT);
		CMP1->MUXCR |= CMP_MUXCR_MSEL(IN1);			//La entrada inversora del comparador sera la señal externa
	}

	//PINmode(PORTNUM2PIN(PORT_C, 2), PIN_INPUT);
	//PINmode(PORTNUM2PIN(PORT_C, 4), PIN_OUTPUT);

	//CMP1->MUXCR |= CMP_MUXCR_PSEL(IN0);				//La entrada no inversora sera una externa (La 2 del puerto C)
}
