/*
 * FSK_MOD.c
 *
 *  Created on: 23 oct. 2019
 *      Author: guido
 */

#include "FSK_MOD.h"
#include "timer.h"
#include "DAC.h"
#include "PIT_driver.h"
#include "DMA.h"
#include <math.h>

#define BIT_TIME (833)
#define SPACE_PERIOD	(454)					// periodo de oscilacion del 0
#define MICRO2NANO(x)	((x)*1000)
#define SCALE_FACTOR (0.38)
//#define NUM_OF_POINTS (0x0FFF)					// numero de puntos del DAC
#define NUM_OF_POINTS (0x1000)
#define CANT_VALUES	(0x1000)
#define BIT_FRAME_LENGTH	(1 + 8 + 1 + 1)		// BIT  de start, 8 BITS  de palabra, BIT  de paridad(impar), BIT de stop
#define MASK	(0x01)
#define BUFFER_LEN	(3500)

static int16_t sinValues[CANT_VALUES];						// Tenemos un arreglo con todos los valores del seno(para no tener calcularlos en las interrupciones)
static bool inited = false;
static uint8_t mod_index = 0;				// indice para recorrer el bitstream
static char char2mod;						// char que el usuario quiere modular a FSK analogica
static bool bitstream[11];				// bitstream a modular
static char buffer[BUFFER_LEN];
static uint16_t write_index = 0;
static uint16_t read_index = 0;
static parity_t par = EVEN;


void char2bitstream(char char2mod, parity_t p);
void FSK_MOD_PISR (void);


void FSK_mod_init(void)
{
	DAC_Init();
	DAC_EnableDMA(DAC0);
	int i;
	for(i = 0; i < CANT_VALUES; i++)
		sinValues[i] = (0x0FFF/2)*sin(((double)i)/0x0FFF * 2*3.14) + (0x0FFF/2);
	DAC_SetRes(RES_128);
	DMA_PeriodicDataTransfer(0, (uint32_t) (sinValues),(uint32_t) ((uint16_t *)(&(DAC0->DAT[0]))), SIZE_16, 1, CANT_VALUES/DAC_getStep(), kDmaRequestMux0AlwaysOn58, TRUE,DAC_getStep(),0,(-1)*CANT_VALUES,0);
	PIT_start(PIT0, 1000000);					//Se inicializan los PITs no el max periodo de interrupcion
	PIT_start(PIT1, 1000000);					//Luego se los setea como corresponde
	read_index = 0;
	write_index = 0;

	DAC_SetSignal(DAC0, SIN, MICRO2NANO(BIT_TIME));								// Estamos en estado IDLE

}


void FSK_modulate_char(char a, parity_t p)
{
	char2mod = a;

	if(inited == false)
	{
		inited = true;
		char2bitstream(a, p);
		PIT_setRutine(PIT1, FSK_MOD_PISR);
		PIT_start(PIT1, MICRO2NANO(833));
//		loadISRtimer(1, FSK_MOD_PISR, BIT_TIME*SCALE_FACTOR, PERIODICISR);		// Vamos a queres cada un tiempo de bit modificar (o no)
//		timerStart(1);												// la frecuencia de la señal analogica para la modulacion
	}
}


void FSK_MOD_PISR (void)
{
	if(mod_index == BIT_FRAME_LENGTH)									// Si ya se leyo toda la palabra de se deja de generar la interrupcion
	{
		inited = false;
		mod_index = 0;
		PIT_stop(PIT1);
		DAC_SetRes(RES_128);
		DAC_SetSignal(DAC0, SIN, MICRO2NANO(BIT_TIME));								// Estamos en estado IDLE
	}
	else
	{
		switch(bitstream[mod_index])
		{
		case 1:
			DAC_SetRes(RES_128);
			DAC_SetSignal(DAC0, SIN, MICRO2NANO(BIT_TIME));								// Genero una señal sin de 833 useg de periodo
			mod_index++;
			break;
		case 0:
			DAC_SetRes(RES_128);										// Le bajo la resolucion para que no se interrumpa demasiado rapido
			DAC_SetSignal(DAC0, SIN, MICRO2NANO(SPACE_PERIOD));								// Genero una señal sin de 833 useg de periodo
			mod_index++;
			break;
		}
	}
}


void char2bitstream(char char2mod, parity_t p)
{
	bitstream[0] = 0;						// BIT de start <--> 0
	uint8_t sum = 0;

	int i;
	for (i = 0; i < 8; i++)
	{
		bitstream[8-i] = (char2mod>>i) & MASK;		// BITS del char
		sum += (char2mod>>i) & MASK;
	}

	switch(p)
	{
	case EVEN:
		bitstream[9] = (sum%2);				// BIT de paridad
		break;
	case ODD:
		bitstream[9] = !(sum%2);
		break;
	}

	bitstream[10] = 1;						// BIT de stop <--> 1
}


void FSK_modulate_char_array(char* str, uint8_t lentgh)
{
	int i;
	for(i = 0; i < lentgh ; i++)
		buffer[(read_index + i)%BUFFER_LEN] = str[i];		//Queremos que de vueltan en el buffer(circular)
	read_index = (read_index + lentgh)%BUFFER_LEN;
}

void FSK_update(void)
{
	if((write_index < read_index) && inited == false)			//Si el indice de write es menor al de read significa que todavia hay chars para convertir
	{															//Inited = false significa que estan en idle, no modulando ningun char
		write_index %= BUFFER_LEN;
		FSK_modulate_char(buffer[write_index], par);
		write_index++;
	}
}

void FSK_config_parity(parity_t p)
{
	par = p;
}
