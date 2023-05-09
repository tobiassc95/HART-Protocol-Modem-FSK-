#include "timer.h"
#include "systick.h"

#define N_TIMERS 5

//Estructura del timer por software.
typedef struct {
	uint32_t period; //es el tiempo en el cual empieza el countdown.
	uint32_t count; //Contiene la cuenta del timer.
	uint8_t mode; //si se quiere que la ISR sea periodica o no.
	void (*funcptr) (void);
	bool enable; //Estado del timer.
}ISRtimer;
//estructura del timer para el delay.
typedef struct {
	uint32_t count;
	bool enable;
}DLYtimer; //temporizador aparte para el delay. creará una interrupcion dedicada.
//arreglo de timers por software.
static ISRtimer isrtimer[N_TIMERS];
static DLYtimer dlytimer;

//para systick (ISR).
void updateTimers(void);

void loadISRtimer(uint8_t i, void (*func)(void), uint32_t T, uint8_t mode) {
	static bool loadsyst = false;

	if(!loadsyst) {
		systickConfigure(LOAD1USEC, updateTimers);
		loadsyst = true;
	}

//	if (T > UINT32_MAX)
//		T = UINT32_MAX;

	isrtimer[i].period = T;
	isrtimer[i].count = T;
	isrtimer[i].funcptr = func;
	isrtimer[i].mode = mode;
}

void timerStart(uint8_t i) {
    isrtimer[i].enable = true;
    systickStart();
}

void timerStop(uint8_t i) {
	isrtimer[i].enable = false;
}

void timerDelay(uint32_t delay) {
	static bool loadsyst = false;

	if(!loadsyst) {
		systickConfigure(LOAD1MSEC, updateTimers);
		loadsyst = true;
	}

//	if(delay > UINT32_MAX)
//		delay = UINT32_MAX;

	dlytimer.count = delay;
	dlytimer.enable = true;
	systickStart();
    while (dlytimer.count != 0);
}

void updateTimers(void) {
	uint8_t i;
	if (dlytimer.enable) {
		dlytimer.count--;
		if(dlytimer.count == 0)
			dlytimer.enable = false;
	}
	for (i = 0; i < N_TIMERS; i++) {
		if(isrtimer[i].enable) {
			isrtimer[i].count--;
			if (isrtimer[i].count == 0) {
				if(isrtimer[i].mode == ONCEISR) //si la funcion solo se corre una vez...
					isrtimer[i].enable = false;
				else if(isrtimer[i].mode == PERIODICISR) //si la funcion se corre periodicamente...
					isrtimer[i].count = isrtimer[i].period;
				isrtimer[i].funcptr();
			}
		}
    }
}
