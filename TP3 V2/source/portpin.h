#ifndef PORTPIN_H
#define PORTPIN_H

#include <stdint.h>
#include <stdbool.h>

//PORT -> es el puerto (portA = 0 - portE = 4)
//NUM -> es el numero del pin del puerto (0 - 31)
//PIN -> no se que es...
// Convert port and number into pin ID
// Ex: PTB5  -> PORTNUM2PIN(PB,5)  -> 0x25
//     PTC22 -> PORTNUM2PIN(PC,22) -> 0x56
#define PORTNUM2PIN(p,n) (((p)<<5) + (n))

//pin mode
#define PIN_INPUT 0
#define PIN_OUTPUT 1

//pin pull
#define PIN_PULLDOWN 0
#define PIN_PULLUP 1

//macros
#define HIGH 1
#define LOW 0

typedef enum {
	PORT_A,
	PORT_B,
	PORT_C,
	PORT_D,
	PORT_E
}PORT;

typedef enum {
	PIN_MUX0, //ANALOG
	PIN_MUX1, //GPIO
	PIN_MUX2,
	PIN_MUX3,
	PIN_MUX4,
	PIN_MUX5,
	PIN_MUX6,
	PIN_MUX7
}PIN_MUX;

typedef enum {
    PIN_IRQ_DISABLE,
	PIN_IRQ_LOGIC0 = 0x8,
	PIN_IRQ_RISING, //interruption in rising edge
	PIN_IRQ_FALLING, //interruption in falling edge.
	PIN_IRQ_BOTH, //interruption in both edges.
	PIN_IRQ_LOGIC1
}PIN_IRQ;

void PINconfigure (uint8_t pin, uint8_t mux, uint8_t irqc);
void PINpull (uint8_t pin, bool pull);
void PINopendrain (uint8_t pin);
void PINmode (uint8_t pin, bool mode);
void PINwrite (uint8_t pin, bool value);
void PINtoggle (uint8_t pin);
bool PINread (uint8_t pin);

//para cargar una funcion ISR externa.
void loadISRpin(uint8_t i, void (*func)(void), uint8_t pin);

/***********************************************/

//void pinPCRconfigure (uint8_t pin, uint32_t pcr);

#endif //PORTPIN_H
