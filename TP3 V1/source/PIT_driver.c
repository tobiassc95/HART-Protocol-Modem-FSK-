/*
 * PIT_driver.c
 *
 *  Created on: 24 oct. 2019
 *      Author: guido
 */

#define CLOCK_PERIOD	(20)		//20 nano seg es el periodo del clock del PIT (50Hz)
#define N_PITS	(4)

#include "PIT_driver.h"

typedef struct {
	uint32_t period;
	void (*funcptr) (void);			//callback del pit
	bool enable; 					//Si el pit esta en acivo
}PIT_t;


static PIT_t PIT_array[N_PITS];
static bool inited = false;
void do_nothing(void);				//Si se llama a PIT start y no tiene un callback se va a romper


void PIT_init(void)
{
	if(!inited)
	{
	//	PIT->MCR = 0;
		SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;				//Hacemos clock gating al PIT
		PIT->MCR &= ~PIT_MCR_MDIS_MASK;					//Habilitamos el PIT

		int i;
		for(i = 0; i < N_PITS; i++)
		{
			PIT_array[i].enable = false;
			PIT_array[i].funcptr = do_nothing;
		}
		inited = true;
	}
}

void PIT_start(PIT_num num, uint32_t period_ns)
{
	PIT_array[num].enable = true;

	PIT->MCR &= ~PIT_MCR_MDIS_MASK;					//Rehabilitamos el PIT

	uint32_t LDVAL_trigger =  (period_ns / CLOCK_PERIOD) - 1;
	PIT->CHANNEL[num].LDVAL = PIT_LDVAL_TSV(LDVAL_trigger);
	PIT->CHANNEL[num].TFLG  = PIT_TFLG_TIF_MASK;
	PIT->CHANNEL[num].TCTRL =  PIT_TCTRL_TIE_MASK		// Se habilitan las interrupciones
								|  PIT_TCTRL_TEN_MASK;		// Timer enable

	NVIC_ClearPendingIRQ(PIT0_IRQn + num);
	NVIC_EnableIRQ(PIT0_IRQn + num); 					// Habilita las inteerupciones del pit deseado
}

void PIT_stop(PIT_num num)
{
	PIT_array[num].enable = false;
	NVIC_DisableIRQ(PIT0_IRQn + num);
}

void PIT_setPeriod(PIT_num num, uint32_t period_ns)
{
	PIT->MCR |= PIT_MCR_MDIS_MASK;					//Desabilitamos el PIT para cambiar el periodo

	uint32_t LDVAL_trigger =  (period_ns / CLOCK_PERIOD) - 1;
	PIT->CHANNEL[num].LDVAL = PIT_LDVAL_TSV(LDVAL_trigger);
	PIT->CHANNEL[num].TFLG  = PIT_TFLG_TIF_MASK;
	PIT->CHANNEL[num].TCTRL =  PIT_TCTRL_TIE_MASK		// Se habilitan las interrupciones
								|  PIT_TCTRL_TEN_MASK;		// Timer enable

	PIT->MCR &= ~PIT_MCR_MDIS_MASK;					//Rehabilitamos el PIT
}

void PIT_setRutine(PIT_num num, void (*func)(void))
{
	PIT_array[num].funcptr = func;
}



__ISR__ PIT0_IRQHandler(void)
{
	PIT->CHANNEL[PIT0].TFLG = PIT_TFLG_TIF_MASK;
	if(PIT_array[PIT0].enable)
		PIT_array[PIT0].funcptr();
}

__ISR__ PIT1_IRQHandler(void)
{
	PIT->CHANNEL[PIT1].TFLG = PIT_TFLG_TIF_MASK;
	if(PIT_array[PIT1].enable)
		PIT_array[PIT1].funcptr();
}

__ISR__ PIT2_IRQHandler(void)
{
	PIT->CHANNEL[PIT2].TFLG |= PIT_TFLG_TIF_MASK;
	if(PIT_array[PIT2].enable)
		PIT_array[PIT2].funcptr();
}

__ISR__ PIT3_IRQHandler(void)
{
	PIT->CHANNEL[PIT3].TFLG |= PIT_TFLG_TIF_MASK;
	if(PIT_array[PIT3].enable)
		PIT_array[PIT3].funcptr();
}


void do_nothing(void)
{

}
