#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

enum {
	ONCEISR, //interrupcion NO PERIODICA (se llama una vez).
	PERIODICISR //interrupcion PERIODICA.
};

//
void loadISRtimer(uint8_t i, void (*func)(void), uint32_t T, uint8_t mode);
//Empieza el timer activando el systick.
void timerStart(uint8_t i);
//Para el timer.
void timerStop(uint8_t i);
//void timerReset(uint8_t i);
//Genera un delay. El delay tiene que estar en milisegundos.
void timerDelay(uint32_t delay);

#endif //TIMER_H
