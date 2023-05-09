/*
 * FSK_MOD.c
 *
 *  Created on: 23 oct. 2019
 *      Author: guido
 */

#include "FSK_MOD.h"
#include "portpin.h"
#include "timer.h"
#include "ftm.h"
#include "PIT_driver.h"
#include <math.h>


#define BIT_TIME (833.0)
#define SPACE_PERIOD	(454.0)					// periodo de oscilacion del 0
#define MICRO2NANO(x)	((x)*1000)
#define SCALE_FACTOR (0.38)
#define NUM_OF_POINTS (0x0FFF)					// numero de puntos del DAC
#define BIT_FRAME_LENGTH	(1 + 8 + 1 + 1)		// BIT  de start, 8 BITS  de palabra, BIT  de paridad(impar), BIT de stop
#define MASK	(0x01)
#define BUFFER_LEN	(3500)
#define NSAMP (100)//cantidad de muestras por 1.2 KHz.
#define PI 3.141593


bool modulating = false;
uint8_t mod_index = 0;				// indice para recorrer el bitstream
static char char2mod;						// char que el usuario quiere modular a FSK analogica
static bool mod_bitstream[11];				// bitstream a modular
static char mod_buffer[BUFFER_LEN];
uint16_t mod_write_index = 0;
uint16_t mod_read_index = 0;
parity_t par = EVEN;

uint16_t compval1[NSAMP]; //contiene "tiempo" en ON de la señal de 1.2 KHz.
//static uint16_t compval0[NSAMP]; //contiene el "tiempo" en ON de la señal de 2.4 KHz.
uint16_t compval_index = 0;


void char2bitstream(char char2mod, parity_t p);
void MOD_PISR (void);
void modulate_IRQ(void);


void mod_Init(void)
{
	PIT_start(PIT1, 1000000);					//Luego se los setea como corresponde
	mod_read_index = 0;
	mod_write_index = 0;
	compval_index = 0;

	uint8_t i;

	FTMconfigure(FTM_0, FTM_UP, FTM_DIV1, 0, 416-1); //416 -> 8320 ns = 120.19KHz. 120.19KHz/100 = 1.202KHz.
	FTMpwm(FTM_0, FTM_CH0, FTM_PWM_EDGE_CLEAR, 416-1);
	//enableFTMinterrupt(FTM_0);

	for (i = 0; i < NSAMP; i++) {
		compval1[i] = (1 + sin(i*PI/50))*getFTMoverfval(FTM_0)/2; //2*PI*f*i/
//		compval0[i] = (1 + sin(i*PI/25))*getFTMoverfval(FTM_0)/2;
	}

	FTMstart(FTM_0);

	PIT_setRutine(PIT0, modulate_IRQ);							// Se empieza a modular en estado IDLE (solo tira 1's)
	PIT_start(PIT0, MICRO2NANO(BIT_TIME)/NSAMP);					// No sera necesario inicializar el PIT 0 porque es el que utiliza l DAC
}


void modulate_char(char a, parity_t p)
{
	char2mod = a;

	if(modulating == false)
	{
		modulating = true;
		char2bitstream(a, p);
		PIT_setRutine(PIT1, MOD_PISR);
		PIT_start(PIT1, MICRO2NANO(833));
		mod_index = 0;
	}
}


void MOD_PISR (void)
{
	if(mod_index == BIT_FRAME_LENGTH)									// Si ya se leyo toda la palabra de se deja de generar la interrupcion
	{
		modulating = false;
		PIT_stop(PIT1);
		PIT_setPeriod(PIT0, MICRO2NANO(BIT_TIME)/NSAMP);
//		DAC_SetRes(RES_128);
//		DAC_SetSignal(DAC0, SIN, MICRO2NANO(BIT_TIME));								// Estamos en estado IDLE
	}
	else
	{
		switch(mod_bitstream[mod_index])
		{
		case 1:
			PIT_setPeriod(PIT0, MICRO2NANO(BIT_TIME)/NSAMP);
			//DAC_SetRes(RES_128);
			//DAC_SetSignal(DAC0, SIN, MICRO2NANO(BIT_TIME));								// Genero una señal sin de 833 useg de periodo
			break;
		case 0:
			PIT_setPeriod(PIT0, MICRO2NANO(BIT_TIME)/NSAMP/2);			// Dado que el 0 es exactamente del doble de frecuencia que el 0, con generar interrupciones al doble de velocidad se lograra doblar la frecuencia
			//DAC_SetRes(RES_128);										// Le bajo la resolucion para que no se interrumpa demasiado rapido
			//DAC_SetSignal(DAC0, SIN, MICRO2NANO(SPACE_PERIOD));								// Genero una señal sin de 833 useg de periodo
			break;
		}
		mod_index++;
	}
}


void char2bitstream(char char2mod, parity_t p)
{
	mod_bitstream[0] = 0;						// BIT de start <--> 0
	uint8_t sum = 0;

	int i;
	for (i = 0; i < 8; i++)
	{
		mod_bitstream[8-i] = (char2mod>>i) & MASK;		// BITS del char
		sum += (char2mod>>i) & MASK;
	}

	switch(p)
	{
	case EVEN:
		mod_bitstream[9] = (sum%2);				// BIT de paridad
		break;
	case ODD:
		mod_bitstream[9] = !(sum%2);
		break;
	}

	mod_bitstream[10] = 1;						// BIT de stop <--> 1
}


void modulate_char_array(char* str, uint8_t lentgh)
{
	int i;
	for(i = 0; i < lentgh ; i++)
		mod_buffer[(mod_read_index + i)%BUFFER_LEN] = str[i];		//Queremos que de vueltan en el buffer(circular)
	mod_read_index = (mod_read_index + lentgh)%BUFFER_LEN;
}

void mod_Run(void)
{
	if((mod_write_index < mod_read_index) && (modulating == false))			//Si el indice de write es menor al de read significa que todavia hay chars para convertir
	{															//modulating = false significa que estan en idle, no modulando ningun char
		mod_write_index %= BUFFER_LEN;
		modulate_char(mod_buffer[mod_write_index], par);
		mod_write_index++;
	}
}

void mod_config_parity(parity_t p)
{
	par = p;
}

void modulate_IRQ(void)
{
	setFTMcompval(FTM_0, FTM_CH0, compval1[compval_index]);
	compval_index++;
	compval_index %= NSAMP;
}
