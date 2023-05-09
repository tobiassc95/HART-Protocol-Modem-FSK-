#ifndef FTM_H
#define FTM_H

#include <stdint.h>
#include <stdbool.h>

//counter mode
#define FTM_UP 0
#define FTM_UPDOWN 1

//pwm mode
#define FTM_PWMEDGE 0
#define FTM_PWMCENTER 1

typedef enum {
	FTM_0,
	FTM_1,
	FTM_2,
	FTM_3,
}FTM_MODULE;

typedef enum { //prescaler
	FTM_DIV1,
	FTM_DIV2,
	FTM_DIV4,
	FTM_DIV8,
	FTM_DIV16,
	FTM_DIV32,
	FTM_DIV64,
	FTM_DIV128
}FTM_PRESCALER;

typedef enum {
	FTM_CH0,
	FTM_CH1,
	FTM_CH2,
	FTM_CH3,
	FTM_CH4,
	FTM_CH5,
	FTM_CH6,
	FTM_CH7
}FTM_CHANNEL;

typedef enum {
	FTM_CH_RISING = 1,
	FTM_CH_FALLING,
	FTM_CH_BOTH
}FTM_INCAPT;

typedef enum {
	FTM_CH_TOGGLE = 1,
	FTM_CH_CLEAR,
	FTM_CH_SET
}FTM_OUTCOMP;

typedef enum {
	FTM_PWM_EDGE_CLEAR = 2,
	FTM_PWM_EDGE_SET,
	FTM_PWM_CENTER_CLEAR,
	FTM_PWM_CENTER_SET
}FTM_PWM;

void FTMconfigure (uint8_t ftm_, uint8_t mode, uint8_t ps, uint16_t initval, uint16_t overfval);
void FTMinputcapture(uint8_t ftm_, uint8_t ch_, uint8_t mode);
void FTMoutputcompare(uint8_t ftm_, uint8_t ch_, uint8_t mode, uint16_t compval);
void FTMpwm(uint8_t ftm_, uint8_t ch_, uint8_t mode, uint16_t compval);
void FTMstart(uint8_t ftm_);
void FTMstop(uint8_t ftm_); //disables counter of FTM.
void setFTMoverfval(uint8_t ftm_, uint16_t overfval);
uint16_t getFTMoverfval(uint8_t ftm_);
void setFTMcompval(uint8_t ftm_, uint8_t ch_, uint16_t compval);
uint16_t getFTMcompval(uint8_t ftm_, uint8_t ch_);
//uint16_t getFTMcnt(uint8_t ftm_);
uint16_t getFTMdata(uint8_t ftm_, uint8_t ch_); //data = duration
uint16_t getFTMdata2read(uint8_t ftm_, uint8_t ch_); //to know if there is a new data in input capture.
void enableFTMinterrupt(uint8_t ftm_);
void disnableFTMinterrupt(uint8_t ftm_);

#endif //FTM_H
