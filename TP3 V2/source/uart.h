#ifndef UART_H
#define UART_H

#include <stdint.h>

typedef enum {
	UART_0,
	UART_1,
	UART_2,
	UART_3,
	UART_4,
}UART_MODULE;

typedef enum {
	UART_PARITY_DISABLE, //no parity
	UART_PARITY_EVEN,
	UART_PARITY_ODD
}UART_PARITY;

void UARTconfigure (uint8_t uart_, uint16_t baudrate, uint8_t parity);
uint16_t UARTreadMSG(uint8_t uart_, char* msg, uint16_t Nbytes);
uint8_t UARTwriteMSG(uint8_t uart_, const char* msg, uint16_t Nbytes);
void enableUARTinterrupt(uint8_t uart_);
void disableUARTinterrupt(uint8_t uart_);

#endif //UART_H
