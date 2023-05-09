#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>
#include <stdbool.h>

#define LOAD10MSEC 1000000 //10 milisegundos.
#define LOAD1MSEC 100000 //1 milisegundo.
#define LOAD100USEC 10000 //100 microsegundos.

//Configura el systick. Esta funcion solo se llama una vez.
void systickConfigure (uint32_t load, void (*func)(void));
//El sistick empieza el downcount.
void systickStart (void);
//Detiene el systick para no generar más interrupciones periodicas internas.
void systickStop (void);
void systickIRQpriority(uint8_t priority);

#endif //INTERRUPT_H
