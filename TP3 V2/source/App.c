/*
 * App.c
 *
 *  Created on: 26 oct. 2019
 *      Author: guido
 */

#include "demod.h"
#include "FTM.h"
#include "UART.h"
#include "CMP.h"
#include "FSK_MOD.h"
#include "PIT_driver.h"
#include "SysTick.h"

#define BUFFER_LEN	(1024)
#define READ_DATA	(10)

CMP_config_t CMP_configuration = {.SE_enable = false, .WE_enable = false, .hysteresis = 3, .FilterCount = 7, .FilterPeriod = 7, .DAC_enable = true, .VDD_pecentage = 50};


char msj_rx[BUFFER_LEN];
char msj_tx[BUFFER_LEN];

void Get_data_rx(void);
void Send_data_tx(void);

void App_init(void)
{
	UARTconfigure(0, 1200, UART_PARITY_EVEN);
	PIT_init();
	mod_Init();
	demod_Init();
	CMP_Init();
	CMP_config(&CMP_configuration);
	mod_config_parity(EVEN);
}

void App_Run(void)
{
	Get_data_rx();				//Se reciben los datos via UART

	mod_Run();					//Se modula

	demod_Run();				//Se demodula

	Send_data_tx();				//Se envian los datos via UART
}


void Get_data_rx(void)
{
	static int app_index_rx = 0;
	static uint8_t charas_read = 0;

	charas_read = UARTreadMSG(UART_0, &(msj_rx[app_index_rx]), READ_DATA);			//Leemos datos recibidos por UART

	if(charas_read > 0)														//Si se leyeron datos
	{
		modulate_char_array(&(msj_rx[app_index_rx]), charas_read);			//Se modulan los datos recibidos
		app_index_rx += charas_read;
		app_index_rx %= BUFFER_LEN;
	}
}


void Send_data_tx(void)
{
	static int app_index_tx = 0;

	if(demod_newData() > 0)											// Si se demodulo algun dato se lee
	{
		uint8_t demod_datas = demod_newData();
		demod_getData(&(msj_tx[app_index_tx]), demod_datas);		// Se toman los datos demodulados
		UARTwriteMSG(UART_0, &(msj_tx[app_index_tx]), demod_datas);		// Se envian por UART
		app_index_tx += demod_datas;
		app_index_tx %= BUFFER_LEN;
	}
}
