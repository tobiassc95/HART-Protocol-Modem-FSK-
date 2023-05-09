#ifndef ADC_H
#define ADC_H

//#define DEBUG_

#include <stdint.h>
#include <stdbool.h>

//sample mode
//#define SHORTSAMPLE 0
//#define LONGSAMPLE 1

typedef enum {
	ADC_0,
	ADC_1
}ADC_MODULE;

typedef enum {
	ADC_DIV1,
	ADC_DIV2,
	ADC_DIV4,
	ADC_DIV8
}ADC_CLKDIV;

typedef enum { //resolution
	ADC_8BITS,
	ADC_12BITS,
	ADC_10BITS,
	ADC_16BITS
}ADC_MODE;

typedef enum { //sample time = convertion time. //cycles = clock cycles.
	ADC_24CYCLES,
	ADC_16CYCLES,
	ADC_10CYCLES,
	ADC_6CYCLES,
	ADC_4CYCLES,
}ADC_SAMPTIME;

typedef enum { //SEE CHIP CONFIGURATION.
	ADC_CHDP0,
	ADC_CHDP1,
	ADC_CHDP2,
	ADC_CHDP3
}ADC_CHANNEL;

void ADCconfigure (uint8_t adc_, uint8_t mode, uint8_t clkdiv, uint8_t sampt);
void ADCstart (uint8_t adc_, uint8_t ch_);
void ADCstop (uint8_t adc_); //disables ADC
void enableADCinterrupt(uint8_t adc_);
void disableADCinterrupt(uint8_t adc_);
uint16_t getADCdata(uint8_t adc_);
uint8_t ADCdata2read(uint8_t adc_);
void ADC_enableDMA(uint8_t adc_, uint8_t dmaCh);

#endif //ADC_H
