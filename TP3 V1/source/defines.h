/*
 * defines.h
 *
 *  Created on: 1 nov. 2019
 *      Author: guido
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#define PIN2PORT(p)         (((p)>>5) & 0x07)
#define PIN2NUM(p)          ((p) & 0x1F) //0x1F = 32.
#define PORTNUM2PIN(p,n) 	(((p)<<5) + (n))

#endif /* DEFINES_H_ */
