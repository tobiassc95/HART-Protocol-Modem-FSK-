/*
 * FSK_MOD.h
 *
 *  Created on: 23 oct. 2019
 *      Author: guido
 */

#ifndef FSK_MOD_H_
#define FSK_MOD_H_
#include <stdint.h>
#include <stdbool.h>

typedef enum {EVEN, ODD} parity_t;

void FSK_mod_init(void);

void FSK_config_parity(parity_t);

void FSK_modulate_char_array(char* str, uint8_t lentgh);		// Se envia un array de char para modular

void FSK_update(void);		// Si hay chars pendientes para modular se modulan, y si no se queda en IDLE

void FSK_modulate_char(char, parity_t);			// Esta funcion hace uso del DAC para modulara un bitstream a una señal analogica modulada en FSK

#endif /* FSK_MOD_H_ */
