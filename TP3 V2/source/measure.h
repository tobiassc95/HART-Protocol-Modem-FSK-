/*
 * defines.h
 *
 *  Created on: 1 nov. 2019
 *      Author: guido
 */

#ifndef MEASURE_H_
#define MEASURE_H_
#define DEBUG_
#define PIN2PORT(p)         (((p)>>5) & 0x07)
#define PIN2NUM(p)          ((p) & 0x1F) //0x1F = 32.
#define PORTNUM2PIN(p,n) 	(((p)<<5) + (n))

#define TESTPIN1 PORTNUM2PIN(PORT_C,8)
#define TESTPIN2 PORTNUM2PIN(PORT_C,16)

typedef enum {
	TEST1,
	TEST2
}TEST_t;

void TEST_Init();
void TEST_On(TEST_t pin);
void TEST_Off(TEST_t pin);
#endif /* MEASURE_H_ */
