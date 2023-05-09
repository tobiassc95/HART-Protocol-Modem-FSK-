#include "portpin.h"
#include "timer.h"
#include "ftm.h"
#include "demod.h"
#include <math.h>

#define NSAMP 100//cantidad de muestras por 1.2 KHz.
#define PI 3.141593
#define PERIOD_1	(833)		//El periodo del 1 es de 833u
#define PERIOD_0	(417)		//El periodo del 0
#define SEMIPERIOD_1	(417)		//El semiperiodo del 1 es de 417u
#define SEMIPERIOD_0	(208)		//El semiperiodo del 0
#define EPSILON		(50)		//Umbral de decision
#define CNT2USEG(x)	((x)/50)	//Ticks del FTM a tiempo (con division de 1)
#define STREAM_LEN	(11)
#define BUFFER_LEN	(3500)


typedef enum {IDLE, DEMODULATING} state_t;
typedef enum {DATA_0, DATA_1, USELESS, BUG} data_t;
typedef enum {COMPLETE, SEMI} conf_t;			// Configuracion por si la ftm esta configurada en RISING o BOTH
static char demod_buffer[BUFFER_LEN];								// Buffer interno de paquetes recibidos
static bool demod_bitstream[11];
static uint16_t demod_write_index = 0;
uint16_t demod_read_index = 0;
bool reading0 = false;							// Como la señal del 0 se repite 2 veces hay que tener guarda
state_t demod_state = IDLE;
conf_t conf;


data_t decide_0_or_1(uint16_t time, conf_t conf);
char bitstream2char(bool* bitstream);

/* Función que se llama 1 vez, al comienzo del programa */
void demod_Init (void)
{
	FTMconfigure(FTM_3, FTM_UP, FTM_DIV1, 0, 0xFFFF-1);
	FTMinputcapture(FTM_3, FTM_CH5, FTM_CH_BOTH);
	enableFTMinterrupt(FTM_3);
	FTMstart(FTM_3);

	demod_state = IDLE;
	demod_write_index = 0;
	demod_read_index = 0;
	reading0 = false;
	conf = SEMI;
}


void demod_Run (void) {
	static uint16_t cnt = 0;
	static uint16_t time = 0;
	static uint8_t bit_index = 0;
	static data_t data;


	if(getFTMdata2read(FTM_3, FTM_CH5))
	{
		cnt = getFTMdata(FTM_3, FTM_CH5);
		time = CNT2USEG(cnt);
		data = decide_0_or_1(time, conf);

		if((data == DATA_0) || (data == DATA_1))
		{
			//time = 0;
			switch (demod_state)
				{
				case IDLE:
					if(data == DATA_0 && !reading0)
					{
						demod_state = DEMODULATING;
						demod_bitstream[bit_index] = 0;
						bit_index++;
					}
					break;
				case DEMODULATING:
					if(data == DATA_0 && !reading0)
					{
						demod_bitstream[bit_index] = 0;
						bit_index++;
					}
					else if(data == DATA_1)
					{
						demod_bitstream[bit_index] = 1;
						bit_index++;
					}
					if(bit_index == STREAM_LEN)
					{
						bit_index = 0;
						demod_buffer[demod_write_index] = bitstream2char(demod_bitstream);
						demod_write_index = (demod_write_index + 1)%BUFFER_LEN;
						demod_state = IDLE;
					}
					break;
				}
		}
	}
}


data_t decide_0_or_1(uint16_t time, conf_t conf)
{
	static uint8_t unos = 0;
	static uint8_t ceros = 0;


	switch(conf)
	{
	case COMPLETE:
		if(time < (PERIOD_0 + EPSILON) && time > (PERIOD_0 - EPSILON))		//Si estamos en estado IDLE y vemos un 0 pasamos a DEMODULATING
		{
			reading0 = !reading0;
			return DATA_0;
		}
		else if(time < (PERIOD_1 + EPSILON) && time > (PERIOD_1 - EPSILON))
			return DATA_1;
		else
			return BUG;
	case SEMI:
		if((time < (SEMIPERIOD_0 + EPSILON)) && (time > (SEMIPERIOD_0 - EPSILON)))		//Si estamos en estado IDLE y vemos un 0 pasamos a DEMODULATING
		{
			unos = 0;
			if(ceros == 0)
			{
				//ceros = 0;
				ceros++;
				ceros %= 4;
				return DATA_0;
			}
			else
			{
				//unos = 0;
				ceros++;
				ceros %= 4;
				return USELESS;
			}
		}
		else if((time < (SEMIPERIOD_1 + EPSILON)) && (time > (SEMIPERIOD_1 - EPSILON)))
		{
			ceros = 0;
			if(unos == 0)
			{
				unos++;
				unos %= 2;
				return DATA_1;
			}
			else
			{
				unos++;
				unos %= 2;
				return USELESS;
			}
		}
		else
		{
			unos = 0;
			return BUG;
		}
	}
	return USELESS;
}

char bitstream2char(bool* bitstream)
{
	char a = 0;
	int i;
	for(i = 0; i < 8; i++)
	{
		a |= (demod_bitstream[i+1])<<(7-i);		//convierte los 8 bits de dato de la trama al char
	}
	return a;
}


uint8_t demod_newData(void)
{
	if(demod_write_index >= demod_read_index)
		return (demod_write_index - demod_read_index);
	else
		return ((demod_write_index + BUFFER_LEN) - demod_read_index);
}

void demod_getData(char* str, uint8_t cant)
{

	int i;
	for(i = 0; i < cant ; i++)
	{
		if(demod_read_index != demod_write_index)
		{
			str[i] = demod_buffer[demod_read_index];
			demod_read_index++;
			demod_read_index %= BUFFER_LEN;
		}
	}
}
