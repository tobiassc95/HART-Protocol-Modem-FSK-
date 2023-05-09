#include "DAC.h"
#include "systick.h"
#include "timer.h"
#include "UART.h"
#include "PIT_driver.h"
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define DAC_DATL_DATA0_WIDTH 8
#define PIT_MIN_FREQ	(20)	//En nano seg

//static char errorMsj[] = "WARNING: la frecuencia solicitada es menor a la minima permitible";
static bool inited = false;
static signal_t signal_form;						// forma de la señal que queremos enviar por el DAC 0
static uint8_t step = 1;							// pasos (niveles) que se dan el DAC

//static uint16_t period_ms;							// periodo se la señal en mili seg

//void DAC_PISR (void);

void DAC_Init (void)
{
	if(inited == false)
	{
		SIM->SCGC2 |= SIM_SCGC2_DAC0_MASK;				// Se hace el clock gating del DAC0
		SIM->SCGC2 |= SIM_SCGC2_DAC1_MASK;

		DAC0->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK | DAC_C0_DACTRGSEL_MASK;		// DACEN-> DAC enable | DACRFS-> La tension de fondo de escala es la alimenacion del micro | DACTRGSEL-> Se cambia el valor por software
		DAC1->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK | DAC_C0_DACTRGSEL_MASK;

		inited = true;
		PIT_init();
	}
}

void DAC_SetData (DAC_t dac, DACData_t data)
{
	dac->DAT[0].DATL = DAC_DATL_DATA0(data);
	dac->DAT[0].DATH = DAC_DATH_DATA1(data >> DAC_DATL_DATA0_WIDTH);
}

void DAC_SetSignal (DAC_t dac, signal_t signal, uint32_t period_ns)
{
	uint32_t IRQperiod = (period_ns / 0x0FFF) * step;

	if(IRQperiod < PIT_MIN_FREQ)
	{
//		uartWriteMsg(UART_0 ,errorMsj, sizeof(errorMsj));
		IRQperiod = PIT_MIN_FREQ;
	}
	signal_form = signal;

	PIT_setPeriod(PIT0, IRQperiod);														// El DAC va a usar el PIT0
	//PIT_setRutine(PIT0, DAC_PISR);
	//loadISRtimer(0, DAC_PISR, IRQperiod, PERIODICISR);
	//timerStart(0);
}

void DAC_EnableDMA(DAC_t dac)
{
	dac->C1 |= DAC_C1_DMAEN(TRUE);
}
/*
void DAC_PISR (void)
{
	static uint16_t k = 0;

	switch (signal_form)
	{
	case RAMP:
		DAC_SetData(DAC0, k);
		k += step;
		k %= 0x0FFF;
		break;
	case SIN:
		DAC_SetData(DAC0, sinValues[k]);
		k += step;
		k %= 0x0FFF;
		break;
	case SQUARE_50:
		if(k < 0x0FFF/2)
			DAC_SetData(DAC0, 0);
		else
			DAC_SetData(DAC0, 0x0FFF);
		k += step;
		k %= 0x0FFF;
	}
}
*/

void DAC_SetRes (res_t res)
{
	switch(res)
	{
	case RES_4096:
		step = 1;
		break;
	case RES_2048:
			step = 2;
			break;
	case RES_1024:
			step = 4;
			break;
	case RES_512:
			step = 8;
			break;
	case RES_256:
			step = 16;
			break;
	case RES_128:
			step = 32;
			break;
	case RES_64:
			step = 64;
			break;
	case RES_32:
			step = 128;
			break;
	}
}

uint8_t DAC_getStep()
{
	return(step);
}
