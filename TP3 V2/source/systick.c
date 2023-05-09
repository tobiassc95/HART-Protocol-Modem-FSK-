#include "systick.h"
#include "hardware.h" //ESTE ARCHIVO TIENE DE TODO.
#include "measure.h"

//SysTick_Type *SysTick;
//SysTick = SysTick_BASE;

static void (*funcptr) (void);

void systickConfigure (uint32_t load, void (*func) (void)) {
	static uint32_t loadmode = UINT32_MAX; //lo inicialimos con un valor que esta fuera del rango de SYSTICK.

	if(loadmode != load || funcptr != func) { //si se quiere un configuracion diferente...
		loadmode = load;
		funcptr = func;
		systickStop();
		SysTick->LOAD = load - 1;
		SysTick->VAL = 0x0;
	}

//	if(!configured) {
//		load = period;
//		SysTick->LOAD = load - 1;
//		SysTick->VAL = 0x0;
//		configured = true;
//	}
//	else {
//		if(load != period) { //si se quiere cambiar el load del systick entonces carga el systick con los nuevos valores.
//			systickStop();
//			load = period;
//			SysTick->LOAD = load - 1; //1 milisegundo.
//			SysTick->VAL = 0x0;
//			configured = true;
//		}
//	}
}

void systickStart (void) {
	//if (configured)
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	//LAS MASCARAS QUE ESTAN EN SysTick->CTRL SON PARA HABILITAR CIERTOS FLAGS DEL REGISTRO. VER PAG 249 DEL ARM CORTEX USER GUIDE.
	//EL CLKSOURCE ES PARA ESPECIFICAR SI USAS EL CONTADOR INTERNO (DEL PROCESADOR) (1) O UNO EXTERNO (0).
	//EL TICKINT ES PARA QUE CUANDO EL CONTADOR LLEGA A CERO ATIENDA AL PEDIDO DE INTERRUPCION (CREO).
	//EL ENABLE ES PARA HABILITAR EL CONTADOR.
}

void systickStop (void) {
	SysTick->CTRL = 0x0;
}

void systickIRQpriority(uint8_t priority) {
	NVIC_SetPriority(SysTick_IRQn, priority);
}

__ISR__ SysTick_Handler (void) {
	funcptr();
}
