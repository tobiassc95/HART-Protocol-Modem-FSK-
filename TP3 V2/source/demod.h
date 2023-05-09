/*
 * demod.h
 *
 *  Created on: 29 oct. 2019
 *      Author: guido
 */

#ifndef DEMOD_H_
#define DEMOD_H_

#include <stdbool.h>
#include <stdint.h>

void demod_Init (void);

void demod_Run (void);

uint8_t demod_newData(void);
/*
 * Esta funcion devuelve la cantidad de datos demodulados y no solicitados por el user
 */

void demod_getData(char* str, uint8_t cant);
/*
 * Esta funcion decibe un puntero a char donde se van a almacenar los datos demodulador
 * y la cantidad de datos que se desean que sean leidos
 */

#endif /* DEMOD_H_ */
