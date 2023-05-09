#include "portpin.h"
#include "hardware.h"

#define PIN2PORT(p)         (((p)>>5) & 0x07)
#define PIN2NUM(p)          ((p) & 0x1F) //0x1F = 32.

#define N_ISRPINS 5

//SIM_Type *SIM; //para el clock del puerto.
//SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK<<PIN2PORT(pin);
//PORT_Type *PORT;
//PORT = (PORT_Type*)(PORTA_BASE + 0x1000*PIN2PORT(pin));
//GPIO_Type *GPIO;
//GPIO = (GPIO_Type*)(GPIOA_BASE + 0x40*PIN2PORT(pin)); //HAGO LO MISMO QUE HICE CON PORT. LO REFERENCIO.

//Estructura de las funciones interuptoras externas del puerto.
typedef struct {
	uint8_t pin;
	void (*funcptr)(void);
}ISRpin;
//tareas que se llaman por interrupcion externa del puerto.
static ISRpin isrpin[N_ISRPINS];

//arreglo que contiene los puertos.
static PORT_Type *port[5] = {PORTA, PORTB, PORTC, PORTD, PORTE};
//arreglo de gpios.
static GPIO_Type *gpio[5] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE};

void PINconfigure (uint8_t pin, uint8_t mux, uint8_t irqc) {
	//turn on CLOCK GATING.
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK<<PIN2PORT(pin);

	//CONFIGURE
	//NVIC_DisableIRQ(PORTA_IRQn + PIN2PORT(pin));
	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] = PORT_PCR_ISF_MASK; //clear ISF.
	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] = PORT_PCR_MUX(mux) | PORT_PCR_IRQC(irqc);
	//NVIC_EnableIRQ(PORTA_IRQn + PIN2PORT(pin));
}

void PINpull (uint8_t pin, bool pull) {
	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_PE_MASK;
	if(pull == PIN_PULLDOWN)
		port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] &= ~PORT_PCR_PS_MASK;
	else //PIN_PULLUP
		port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_PS_MASK;
}

void PINopendrain (uint8_t pin) {
	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_ODE_MASK;
}

void PINmode (uint8_t pin, bool mode) {
	gpio[PIN2PORT(pin)]->PDDR = (gpio[PIN2PORT(pin)]->PDDR & ~((uint32_t)1<<PIN2NUM(pin))) | mode<<PIN2NUM(pin);

	//TAMBIEN LO PUEDO HACER ASI.
//	if (mode == OUTPUT)
//		gpio[PIN2PORT(pin)]->PDDR |= 1<<PIN2NUM(pin);
//	else if (mode == INPUT)
//		gpio[PIN2PORT(pin)]->PDDR &= ~1<<PIN2NUM(pin);
}

void PINwrite (uint8_t pin, bool value) {
	gpio[PIN2PORT(pin)]->PDOR = (gpio[PIN2PORT(pin)]->PDOR & ~((uint32_t)1<<PIN2NUM(pin))) | value<<PIN2NUM(pin);

	//TAMBIEN LO PUEDO HACER ASI.
//	if(state == HIGH) //SET.
//		gpio[PIN2PORT(pin)]->PSOR = (1<<PIN2NUM(pin)); //SHIFTEO A LA IZQ. EL 1 POR "PIN2NUM(pin)" VECES.
//	else //CLEAR.
//		gpio[PIN2PORT(pin)]->PCOR = (1<<PIN2NUM(pin)); //SHIFTEO A LA IZQ. EL 1 POR "PIN2NUM(pin)" VECES.
}

void PINtoggle (uint8_t pin) {
	gpio[PIN2PORT(pin)]->PTOR = (1<<PIN2NUM(pin)); //SHIFTEO A LA IZQ. EL 1 POR "PIN2NUM(pin)" VECES.
}

bool PINread (uint8_t pin) {
	return (gpio[PIN2PORT(pin)]->PDIR & 1<<PIN2NUM(pin))>>PIN2NUM(pin);
}

void loadISRpin(uint8_t i, void (*func)(void), uint8_t pin) {
	isrpin[i].funcptr = func;
	isrpin[i].pin = pin;
}

__ISR__ PORTA_IRQHandler(void) {
	uint8_t i;

	NVIC_DisableIRQ(PORTA_IRQn);

	for (i = 0; i < N_ISRPINS; i++) {
		if (PIN2PORT(isrpin[i].pin) == PORT_A) {
			if(port[PORT_A]->PCR[PIN2NUM(isrpin[i].pin)] & PORT_PCR_ISF_MASK) { //nos fijamos si es el pin correspondiente a la interrupcion
				port[PORT_A]->PCR[PIN2NUM(isrpin[i].pin)] |= PORT_PCR_ISF_MASK; //ponemos en cero el interrupt flag. w1c.
				isrpin[i].funcptr();
			}
		}
	}
	NVIC_EnableIRQ(PORTA_IRQn);
}






/*********************************************************/


void pinPCRconfigure (uint8_t pin, uint32_t pcr) {
	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] = 0x01000000; //LIMPIAMOS EL REGISTRO DE 32 BITS (PCR) (RESET). PARA LUEGO CONFIGURARLO.

	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] = pcr;


//	pinIRQCconfigure(pin, pcrcfg.irqc);
//	//falta lk... poner 0 si no se usa.
//	pinMUXconfigure(pin, pcrcfg.mux);
//	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] = (port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] & ~PORT_PCR_PFE_MASK) | PORT_PCR_DSE(pcrcfg.dse);
//	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] = (port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] & ~PORT_PCR_PFE_MASK) | PORT_PCR_DSE(pcrcfg.ode);
//	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] = (port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] & ~PORT_PCR_PFE_MASK) | PORT_PCR_PFE(pcrcfg.pfe);
//	//falta sre... poner 0 si no se usa.
//	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] = (port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] & ~PORT_PCR_PE_MASK) | PORT_PCR_PE(pcrcfg.pe);
//	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] = (port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] & ~PORT_PCR_PS_MASK) | PORT_PCR_PS(pcrcfg.ps);

	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK<<PIN2PORT(pin);
}
