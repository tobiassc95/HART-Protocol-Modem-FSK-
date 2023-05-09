/*
 * PIT_driver.h
 *
 *  Created on: 24 oct. 2019
 *      Author: guido
 */

#ifndef PIT_DRIVER_H_
#define PIT_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "hardware.h"

typedef enum {PIT0, PIT1, PIT2, PIT3} PIT_num;

void PIT_init(void);						// inicializa el PIT

void PIT_start(PIT_num, uint32_t period_us);	// Se inicia uno de los 4 PITS, con el periodo de interrupcion deseado en micros, OJO QUE SI NO TENE CALLBACK SE VA A ROMPER

void PIT_stop(PIT_num num);

void PIT_setPeriod(PIT_num, uint32_t period_ns);

void PIT_setRutine(PIT_num num, void (*func)(void));		// Se le manda el callback para que llame en cada IRQHandler

#endif /* PIT_DRIVER_H_ */
