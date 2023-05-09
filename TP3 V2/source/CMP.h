#ifndef CMP_H_
#define CMP_H_
#include <stdbool.h>

#include "hardware.h"

typedef struct
{
	bool SE_enable;					// Sample Mode
	bool WE_enable;					// Windowing mode
	uint8_t FilterCount;			//Filter Sample Count (recomiendo poner al mango)
	uint8_t hysteresis;				//Comparator hard block hysteresis control (no me cambio nada)
	uint8_t FilterPeriod;			//Filter Sample Period (cuanto mas mejor, no se porque)
	bool DAC_enable;				//Habilita el DAC del CMP para usar como referencia
	uint8_t VDD_pecentage;			//Porcentaje de VDD para comparar | 50 -> 3.3V/2
}CMP_config_t;

void CMP_Init (void);
void CMP_config (CMP_config_t*);
/*
 * Si no se utiliza el DAC_enable los pines de entrada seran los 2 y 3 del puerto C
 * Si si se utiliza el DAC el unico pin de entrada sera el 2 del puerto C
 * En todo caso la salida sera el pin 4 del puerto C
 */

#endif /* void DAC_H_ */
