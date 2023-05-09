
#ifndef DAC_H_
#define DAC_H_

#include "hardware.h"

typedef DAC_Type *DAC_t;
typedef uint16_t DACData_t;

typedef enum {RAMP, SIN, SQUARE_50} signal_t;
typedef enum {RES_4096, RES_2048, RES_1024, RES_512, RES_256, RES_128, RES_64, RES_32} res_t;		// puntos de resolucion del DAC

void DAC_Init (void);

void DAC_SetRes (res_t);

void DAC_SetData (DAC_t, DACData_t);

void DAC_SetSignal (DAC_t, signal_t,  uint32_t period_ns);

uint8_t DAC_getStep();

void DAC_EnableDMA(DAC_t dac);

#endif /* void DAC_H_ */
