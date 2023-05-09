#include <measure.h>
#include "portpin.h"

void TEST_Init()
{
	PINconfigure(TESTPIN1, PIN_MUX1, PIN_IRQ_DISABLE);
	PINmode(TESTPIN1, PIN_OUTPUT);
	PINwrite(TESTPIN1, LOW);
	PINconfigure(TESTPIN2, PIN_MUX1, PIN_IRQ_DISABLE);
	PINmode(TESTPIN2, PIN_OUTPUT);
	PINwrite(TESTPIN2, LOW);
}
void TEST_On(TEST_t pin)
{
	switch(pin) {
	case TEST1:
		PINwrite(TESTPIN1, HIGH);
		break;
	case TEST2:
		PINwrite(TESTPIN2, HIGH);
		break;
	}
}

void TEST_Off(TEST_t pin)
{
	switch(pin) {
	case TEST1:
		PINwrite(TESTPIN1, LOW);
		break;
	case TEST2:
		PINwrite(TESTPIN2, LOW);
		break;
	}
}
