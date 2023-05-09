#include "timer.h"
#include "adc.h"
#include "fsk.h"
#include "UART.h"
#include "PIT_driver.h"
#include "FSK_MOD.h"
#include "DMA.h"
#include "portpin.h"

#define NSAMP 10 //amount of samples each bit.
#define NBITFRAME 11 //amount of bits of the UART bitframe.
#define NSAMPFRAME NSAMP*NBITFRAME //total amount of sample each UART bitframe.

char bitstream2char(uint8_t* bitstream);
void adc_start(void);
char demod_run(void);

//static uart_cfg_t uartcfg = {.baudrate = 1200, .parity = even_parity, .stopBit = 1};

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void) {
	//PORT_Init();
	UARTconfigure(UART_0, 1200, UART_PARITY_EVEN);
	ADCconfigure(ADC_0, ADC_10BITS, ADC_DIV8, ADC_16CYCLES); //convertion at 11.3 usec.
	DMA_Init();
	ADC_enableDMA(ADC_0,2);
	FSKconfiguration();

	FSK_mod_init();
	FSK_config_parity(ODD);

	PIT_init();
	PIT_setRutine(PIT2, adc_start);
	PIT_start(PIT2, 83333); //83.333 usec //capaz hay que cambiarlo a 83328.

#ifdef DEBUG_
	PINconfigure(PORTNUM2PIN(PORT_C,8), PIN_MUX1, PIN_IRQ_DISABLE);
	PINmode(PORTNUM2PIN(PORT_C,8), PIN_OUTPUT);
	PINwrite(PORTNUM2PIN(PORT_C,8),1);
#endif
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void) {
	static char msg_r[1024]; //msg from uart.
	static uint16_t index = 0;
	static uint16_t Nread = 0; //cantidad de bytes que se leyeron.
	char msg_out; //from demodulator to UART

	//char msg__[] = "a";
	//static uint16_t n = 0;
	//char msg__[] = "hereare000hereare010hereare020hereare030hereare040hereare050hereare060hereare070hereare080hereare090hereare100hereare110hereare120hereare130hereare140hereare150hereare160hereare170hereare180hereare190hereare200hereare210hereare220hereare230hereare240hereare250hereare260hereare270hereare280hereare290hereare300hereare310hereare320hereare330hereare340hereare350hereare360hereare370hereare380hereare390hereare400hereare410hereare420hereare430hereare440hereare450hereare460hereare470";

	//MODULATOR
	FSK_update();
	Nread = UARTreadMSG(UART_0, &(msg_r[index]), 1);
	if(Nread > 0) {
		FSK_modulate_char_array(&(msg_r[index]), Nread);
		index += Nread;
		index %= 1024;
		Nread = 0;
	}
//	if (n < sizeof(msg__)) {
//		FSK_modulate_char_array(&(msg__[n]), 10);
//		n+=10;
//	}


	//DEMODULATOR
	msg_out = demod_run();
	if(msg_out != 0)
		UARTwriteMSG(UART_0, &msg_out, 1);
}

char bitstream2char(uint8_t* bitstream) {
	char a = 0;
	int i;
	for(i = 1; i < 9; i++) {
		a |= (bitstream[i])<<(8-i);		//convierte los 8 bits de dato de la trama al char
	}
	return a;
}

void adc_start(void) {
#ifdef DEBUG_
	PINwrite(PORTNUM2PIN(PORT_C,8),0);
#endif

	ADCstart(ADC_0, ADC_CHDP1);

#ifdef DEBUG_
	PINwrite(PORTNUM2PIN(PORT_C,8),1);
#endif
}


char demod_run(void) {
	uint16_t data; //value after ADC and after comparator.
	float data_; //value after demodulator.
	char msg = 0; //the byte to be sent by UART.
	static uint8_t frame[NBITFRAME];
	static uint8_t bit = 0; //current bit.
	static bool startbit = false, stopbit = false;

	if(ADCdata2read(ADC_0) == TRUE) {
		data = getADCdata(ADC_0);
		data_ = FSKdemodulator(data);

		//comparator
		if (data_ > 0.25) {
			data = 0;
			startbit = true; //start bit is always 0.
		}
		else
			data = 1;

		//bulding the frame.
		if(startbit) {
			frame[(int)(bit/NSAMP)] += data;
			bit++;
			if(bit == NSAMP*NBITFRAME) {
				startbit = false;
				stopbit = true;
			}
		}
	}
	if (stopbit) {
		for (bit = 0; bit < NBITFRAME; bit++) {
			if(frame[bit] >= 5)
				frame[bit] = 1;
			else
				frame[bit] = 0;
		}
		stopbit = false;

		msg = bitstream2char(frame); //converting the information of the array in a char.

		for (bit = 0; bit < NBITFRAME; bit++) {
			frame[bit] = 0; //clear frame.
		}
		bit = 0; //clear bit.
	}

	return msg;
}
