#include "uart.h"
#include "portpin.h"
#include "hardware.h"
#include "measure.h"

//UART0
#define PTB17 PORTNUM2PIN(PORT_B, 17) //TX
#define PTB16 PORTNUM2PIN(PORT_B, 16) //RX
//UART1
#define PTC4 PORTNUM2PIN(PORT_C, 4) //TX
#define PTC3 PORTNUM2PIN(PORT_C, 3) //RX
//UART2
#define PTD3 PORTNUM2PIN(PORT_D, 3) //TX
#define PTD2 PORTNUM2PIN(PORT_D, 2) //RX
//UART3
#define PTC17 PORTNUM2PIN(PORT_C, 17) //TX
#define PTC16 PORTNUM2PIN(PORT_C, 16) //RX
//UART4
#define PTE24 PORTNUM2PIN(PORT_E, 24) //TX
#define PTE25 PORTNUM2PIN(PORT_E, 25) //RX

#define UART_DEFAULT_BAUDRATE 9600
#define BUFFER_LEN	(3500)

//TXFIFO (circular buffer)
typedef struct {
	char buff[BUFFER_LEN];
	uint16_t first;
	uint16_t last;
	uint16_t Nbytes;
}TXFIFO;
static TXFIFO txfifo;

//TXFIFO (circular buffer)
typedef struct {
	char buff[BUFFER_LEN];
	uint16_t first;
	uint16_t last;
	uint16_t Nbytes;
}RXFIFO;
static RXFIFO rxfifo;

static UART_Type * uart[5] = {UART0, UART1, UART2, UART3, UART4};

void baudrateCalculator (uint8_t uart_, uint16_t baudrate);

void UARTconfigure (uint8_t uart_, uint16_t baudrate, uint8_t parity) {
	if(uart_ == UART_4)
		SIM->SCGC1 |= SIM_SCGC1_UART4_MASK;
	else
		SIM->SCGC4 = SIM_SCGC4_UART0_MASK << uart_;

	uart[uart_]->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);

	//Configure the UART for 8-bit mode, no parity.
	baudrateCalculator(uart_, baudrate);

	if(parity != UART_PARITY_DISABLE){
		uart[uart_]->C1 = UART_C1_PE_MASK | UART_C1_M_MASK;
		uart[uart_]->C4 &= ~UART_C4_M10_MASK;
		uart[uart_]->BDH &= ~UART_BDH_SBNS_MASK;

		if(parity == UART_PARITY_ODD)
			uart[uart_]->C1 |= UART_C1_PT_MASK;
	}
	else
		uart[uart_]->C1 = 0;

	uart[uart_]->C2 = (UART_C2_TE_MASK | UART_C2_RE_MASK | UART_C2_RIE_MASK);

	switch (uart_) {
	case UART_0:
		PINconfigure(PTB17, PIN_MUX3, PIN_IRQ_DISABLE);
		PINconfigure(PTB16, PIN_MUX3, PIN_IRQ_DISABLE);
		break;
	case UART_1:
		PINconfigure(PTC4, PIN_MUX3, PIN_IRQ_DISABLE);
		PINconfigure(PTC3, PIN_MUX3, PIN_IRQ_DISABLE);
		break;
	case UART_2:
		PINconfigure(PTD3, PIN_MUX3, PIN_IRQ_DISABLE);
		PINconfigure(PTD2, PIN_MUX3, PIN_IRQ_DISABLE);
		break;
	case UART_3:
		PINconfigure(PTC17, PIN_MUX3, PIN_IRQ_DISABLE);
		PINconfigure(PTC16, PIN_MUX3, PIN_IRQ_DISABLE);
		break;
	case UART_4:
		PINconfigure(PTE24, PIN_MUX3, PIN_IRQ_DISABLE);
		PINconfigure(PTE25, PIN_MUX3, PIN_IRQ_DISABLE);
		break;
	}

	txfifo.first = 0;
	txfifo.last = 0;
	txfifo.Nbytes = 0;
	rxfifo.first = 0;
	rxfifo.last = 0;
	rxfifo.Nbytes = 0;
}

void baudrateCalculator (uint8_t uart_, uint16_t baudrate) {
	uint16_t sbr, brfa;
	uint32_t clk;

	clk = ((uart_ == UART_0) || (uart_ == UART_1)) ? (__CORE_CLOCK__):(__CORE_CLOCK__>>1); //set clock to UART.

	baudrate = ((baudrate == 0)?(UART_DEFAULT_BAUDRATE):((baudrate > 0x1FFF)?(UART_DEFAULT_BAUDRATE):(baudrate)));

	sbr = clk / (baudrate << 4);
	brfa = (clk << 1) / baudrate - (sbr << 5);

	uart[uart_]->BDH = UART_BDH_SBR(sbr >> 8);
	uart[uart_]->BDL = UART_BDL_SBR(sbr);
	uart[uart_]->C4 = (uart[uart_]->C4 & ~UART_C4_BRFA_MASK) | UART_C4_BRFA(brfa);
}

void enableUARTinterrupt(uint8_t uart_) {
	if(uart_ == UART_4)
		NVIC_EnableIRQ(UART4_RX_TX_IRQn);
	else
		NVIC_EnableIRQ(UART0_RX_TX_IRQn + 2*uart_);
}

void disableUARTinterrupt(uint8_t uart_) {
	if(uart_ == UART_4)
		NVIC_EnableIRQ(UART4_RX_TX_IRQn);
	else
		NVIC_EnableIRQ(UART0_RX_TX_IRQn + 2*uart_);
}

uint8_t UARTwriteMSG(uint8_t uart_, const char* msg, uint16_t Nbytes) { //when we transmit, we write.
	uint8_t i;

	disableUARTinterrupt(uart_);
	for(i = 0; i < Nbytes; i++){
		txfifo.buff[txfifo.last] = msg[i];
		if(txfifo.last++ == BUFFER_LEN)
			txfifo.last = 0;
		txfifo.Nbytes++;
	}
	uart[uart_]->C2 |= UART_C2_TIE_MASK; //to generate interrupt and send the msg.
	enableUARTinterrupt(uart_);
	return 1;
}

uint16_t UARTreadMSG(uint8_t uart_, char* msg, uint16_t Nbytes) { //when we recieve, we read.
	uint8_t n = 0; //counts the amount of bytes that are read.

	disableUARTinterrupt(uart_);
	while (Nbytes--) {
		if (rxfifo.Nbytes == 0)
			break;
		*msg++ = rxfifo.buff[rxfifo.first];
		if(rxfifo.first++ == BUFFER_LEN)
			rxfifo.first = 0;
		rxfifo.Nbytes--;
		n++;
	}
	enableUARTinterrupt(uart_);
	return n;
}

__ISR__ UART0_RX_TX_IRQHandler (void) {
	unsigned char temp; //para guardar los estados.

	temp = UART0->S1;
	if(temp & UART_S1_RDRF_MASK) { //si recive (RX)
		rxfifo.buff[rxfifo.last] = UART0->D;
		if(rxfifo.last++ == BUFFER_LEN)
			rxfifo.last = 0;
		rxfifo.Nbytes++;
	}

	if (temp & UART_S1_TDRE_MASK) { //si transmite (TX)
		if (txfifo.Nbytes <= 1) //stop generating interrupts when we are runing out of bytes.
			UART0->C2 &= ~UART_C2_TIE_MASK;
		if (txfifo.Nbytes) {
			txfifo.Nbytes--;
			UART0->D = (uint8_t) txfifo.buff[txfifo.first]; // Transmit
			if(txfifo.first++ == BUFFER_LEN)
				txfifo.first = 0;
		}
	}
}
