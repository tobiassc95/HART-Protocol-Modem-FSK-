#include "ftm.h"
#include "portpin.h"
#include "hardware.h"
#include <stdlib.h>

//FTM0
#define PTC1 PORTNUM2PIN(PORT_C, 1) //CH0
#define PTC2 PORTNUM2PIN(PORT_C, 2) //CH1
#define PTC3 PORTNUM2PIN(PORT_C, 3) //CH2
#define PTC4 PORTNUM2PIN(PORT_C, 4) //CH3
#define PTA0 PORTNUM2PIN(PORT_A, 0) //CH5
#define PTA2 PORTNUM2PIN(PORT_A, 2) //CH7
//FTM3
#define PTD0 PORTNUM2PIN(PORT_D, 0) //CH0
#define PTD1 PORTNUM2PIN(PORT_D, 1) //CH1
#define PTD2 PORTNUM2PIN(PORT_D, 2) //CH2
#define PTD3 PORTNUM2PIN(PORT_D, 3) //CH3
#define PTC9 PORTNUM2PIN(PORT_C, 9) //CH5

//TEST PIN
#define PTC8 PORTNUM2PIN(PORT_C, 8)

#define N_FTM 2//number of FTM.
#define N_CH 8//number of channels

static uint16_t data[N_FTM][N_CH]; //data = duration
static bool data2read[N_FTM][N_CH];

static FTM_Type* ftm[4] = {FTM0, FTM1, FTM2, FTM3};

void ISR_INCAPT (uint8_t ftm_, uint8_t ch_);
void ISR_OUTCOMP (uint8_t ftm_, uint8_t ch_);
void ISR_PWM (uint8_t ftm_, uint8_t ch_);

void FTMconfigure (uint8_t ftm_, uint8_t mode, uint8_t ps, uint16_t initval, uint16_t overfval) {
	uint8_t Nch, Nftm;

	if (overfval < initval) //end must be > begin.
		return;

	//turn on CLOCK GATING. //NOTE: always at the begining.
	if(ftm_ != 3)
		SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK << ftm_;
	else
		SIM->SCGC3 |= SIM_SCGC3_FTM3_MASK;

	//CONFIG.
	ftm[ftm_]->MODE = FTM_MODE_WPDIS_MASK | FTM_MODE_FTMEN_MASK;
	ftm[ftm_]->SC = FTM_SC_PS(ps) | FTM_SC_TOIE_MASK;
	if (mode == FTM_UP)
		ftm[ftm_]->SC &= ~FTM_SC_CPWMS_MASK;
	else if (mode == FTM_UPDOWN)
		ftm[ftm_]->SC |= FTM_SC_CPWMS_MASK;
	ftm[ftm_]->QDCTRL = FTM_QDCTRL_QUADEN(0);

	setFTMoverfval(ftm_, overfval);
	ftm[ftm_]->CNTIN = initval;
	ftm[ftm_]->CNT = 0; //to reset the FTM counter.

	ftm[ftm_]->MODE &= ~FTM_MODE_FTMEN_MASK;

	//PIN CONFIG
	switch (ftm_) {
	case FTM_0:
		PINconfigure(PTC1, PIN_MUX4, PIN_IRQ_DISABLE);
		PINconfigure(PTC2, PIN_MUX4, PIN_IRQ_DISABLE);
		PINconfigure(PTC3, PIN_MUX4, PIN_IRQ_DISABLE);
		PINconfigure(PTC4, PIN_MUX4, PIN_IRQ_DISABLE);
		break;
	case FTM_1:
		break;
	case FTM_2:
		break;
	case FTM_3:
		PINconfigure(PTD0, PIN_MUX4, PIN_IRQ_DISABLE);
		PINconfigure(PTD1, PIN_MUX4, PIN_IRQ_DISABLE);
		PINconfigure(PTD2, PIN_MUX4, PIN_IRQ_DISABLE);
		PINconfigure(PTD3, PIN_MUX4, PIN_IRQ_DISABLE);
		PINconfigure(PTC9, PIN_MUX3, PIN_IRQ_DISABLE);
		break;
	}

	for(Nftm = N_FTM; Nftm > 0; --Nftm) {
		for(Nch = N_FTM; Nch > 0; --Nch) {
			data[Nftm][Nch] = 0;
		}
	}

	//TEST PIN
	PINconfigure(PTC8, PIN_MUX1, PIN_IRQ_DISABLE);
	PINmode(PTC8, PIN_OUTPUT);
	PINwrite(PTC8, 1);
}

void FTMinputcapture(uint8_t ftm_, uint8_t ch_, uint8_t mode) {
	ftm[ftm_]->CONTROLS[ch_].CnSC = FTM_CnSC_MSB(0) | FTM_CnSC_MSA(0) | FTM_CnSC_ELSB(mode>>1) | FTM_CnSC_ELSA(mode) | FTM_CnSC_CHIE_MASK;
}

void FTMoutputcompare(uint8_t ftm_, uint8_t ch_, uint8_t mode, uint16_t compval) {
	ftm[ftm_]->CONTROLS[ch_].CnSC = FTM_CnSC_MSB(0) | FTM_CnSC_MSA(1) | FTM_CnSC_ELSB(mode>>1) | FTM_CnSC_ELSA(mode) | FTM_CnSC_CHIE_MASK;
	setFTMcompval(ftm_, ch_, compval);
}

void FTMpwm(uint8_t ftm_, uint8_t ch_, uint8_t mode, uint16_t compval) {
	ftm[ftm_]->PWMLOAD = FTM_PWMLOAD_LDOK_MASK | FTM_PWMLOAD_CH0SEL_MASK<<ch_;
	if(mode <= FTM_PWM_EDGE_SET) { //EDGE mode
		ftm[ftm_]->SC &= ~FTM_SC_CPWMS_MASK;
		ftm[ftm_]->CONTROLS[ch_].CnSC = FTM_CnSC_MSB(1) | FTM_CnSC_ELSB(mode>>1) | FTM_CnSC_ELSA(mode) | FTM_CnSC_CHIE_MASK;
		ftm[ftm_]->CONTROLS[ch_].CnV = compval;
	}
	else { //CENTER mode
		ftm[ftm_]->SC |= FTM_SC_CPWMS_MASK;
		ftm[ftm_]->CONTROLS[ch_].CnSC = FTM_CnSC_MSB(1) | FTM_CnSC_ELSB((mode-2)>>1) | FTM_CnSC_ELSA((mode-2)) | FTM_CnSC_CHIE_MASK;
		ftm[ftm_]->CONTROLS[ch_].CnV = compval;
	}
}

void FTMstart(uint8_t ftm_) {
	ftm[ftm_]->SC |= FTM_SC_CLKS(1);
}

void FTMstop(uint8_t ftm_) {
	ftm[ftm_]->SC &= ~FTM_SC_CLKS_MASK; //FTM_SC_CLKS(0)
}

void setFTMoverfval(uint8_t ftm_, uint16_t overfval) {
	ftm[ftm_]->MOD = overfval;
}

uint16_t getFTMoverfval(uint8_t ftm_) {
	return (uint16_t)ftm[ftm_]->MOD;
}

void setFTMcompval(uint8_t ftm_, uint8_t ch_, uint16_t compval) {
	ftm[ftm_]->CONTROLS[ch_].CnV = compval;
}

uint16_t getFTMcompval(uint8_t ftm_, uint8_t ch_) {
	return (uint16_t)ftm[ftm_]->CONTROLS[ch_].CnV;
}

//uint16_t getFTMcnt(uint8_t ftm_) {
//	return (uint16_t)ftm[ftm_]->CNT;
//}

uint16_t getFTMdata(uint8_t ftm_, uint8_t ch_) {
	if(ftm_ == FTM_0)
		return data[0][ch_];
	else if(ftm_ == FTM_3)
		return data[1][ch_];
	return 0;
}

uint16_t getFTMdata2read(uint8_t ftm_, uint8_t ch_) { //to know if there is a new data in input capture.
	if(ftm_ == FTM_0) {
		if(data2read[0][ch_]) {
			data2read[0][ch_] = false;
			return true;
		}
		else
			return false;
	}
	else if(ftm_ == FTM_3) {
		if(data2read[1][ch_]) {
			data2read[1][ch_] = false;
			return true;
		}
		else
			return false;
	}
	return false;
}

void enableFTMinterrupt(uint8_t ftm_) {
	if(ftm_ != 3)
		NVIC_EnableIRQ(FTM0_IRQn + ftm_);
	else
		NVIC_EnableIRQ(FTM3_IRQn);
}

void disnableFTMinterrupt(uint8_t ftm_) {
	if(ftm_ != 3)
		NVIC_DisableIRQ(FTM0_IRQn + ftm_);
	else
		NVIC_DisableIRQ(FTM3_IRQn);
}

void ISR_INCAPT (uint8_t ftm_, uint8_t ch_) {
	uint8_t temp1, temp2;
	static uint16_t lastcnv;
	static bool loadcnv = true;

	temp1 = ftm[ftm_]->SC;
	temp2 = ftm[ftm_]->CONTROLS[ch_].CnSC; //just channel 5.

	if (temp1 & FTM_SC_TOF_MASK) { //when CNT == MOD.
		ftm[ftm_]->SC &= ~FTM_SC_TOF_MASK; //to clear the TOF flag.

	}
	if (temp2 & FTM_CnSC_CHF_MASK) { //when CNT == CnV.
		ftm[ftm_]->CONTROLS[ch_].CnSC &= ~FTM_CnSC_CHF_MASK; //to clear the CHF flag.
		if(loadcnv) {
			lastcnv = getFTMcompval(ftm_, ch_);
			loadcnv = false;
		}
		else {
			if(getFTMcompval(ftm_, ch_) >= lastcnv)
				data[1][ch_] = getFTMcompval(ftm_, ch_) - lastcnv; //just FTM3
			else
				data[1][ch_] = getFTMcompval(ftm_, ch_) + (getFTMoverfval(ftm_) + 1) - lastcnv; //just FTM3
			lastcnv = getFTMcompval(ftm_, ch_); //update.
			data2read[1][ch_] = true;
			//cnv = (uint16_t)ftm[ftm_]->CONTROLS[ch_].CnV;
			//loadcnv = true; //se saltea un periodo.
		}
	}
}

void ISR_OUTCOMP(uint8_t ftm_, uint8_t ch_) {
	uint8_t temp1, temp2;
	//static uint8_t temp = 2;
	temp1 = (uint8_t)ftm[ftm_]->SC;
	temp2 = (uint8_t)ftm[ftm_]->CONTROLS[ch_].CnSC; //just channel 0.

	if (temp1 & FTM_SC_TOF_MASK) { //when CNT == MOD.
		ftm[ftm_]->SC &= ~FTM_SC_TOF_MASK; //to clear the TOF flag.
		//ftm[ftm_]->MODE |= FTM_MODE_FTMEN_MASK; //a better way to update the CnV value. SEE TABLE 40-7 ON PAGE 1054.
		//PINtoggle(PTC8);
	}
	if (temp2 & FTM_CnSC_CHF_MASK) { //when CNT == CnV.
		ftm[ftm_]->CONTROLS[ch_].CnSC &= ~FTM_CnSC_CHF_MASK; //to clear the CHF flag.
		//ftm[ftm_]->MODE &= ~FTM_MODE_FTMEN_MASK;
//		temp--;
//		if(temp == 0) {
//			if ((uint16_t)ftm[ftm_]->MOD == 5208) {
//				ftm[ftm_]->MOD = 2604;
//				temp = 4;
//			}
//			else if ((uint16_t)ftm[ftm_]->MOD == 2604) {
//				ftm[ftm_]->MOD = 5208;
//				temp = 2;
//			}
//		}
		//PINtoggle(PTC8);
		//ftm[ftm_]->CONTROLS[ch_].CnV = (uint16_t)ftm[ftm_]->CONTROLS[ch_].CnV + 100;
	}
}

void ISR_PWM(uint8_t ftm_, uint8_t ch_) {
	uint8_t temp1, temp2;
	temp1 = (uint8_t)ftm[ftm_]->SC;
	temp2 = (uint8_t)ftm[ftm_]->CONTROLS[ch_].CnSC; //just channel 0.

	if (temp1 & FTM_SC_TOF_MASK) { //when CNT == MOD.
		ftm[ftm_]->SC &= ~FTM_SC_TOF_MASK; //to clear the TOF flag.
		//ftm[ftm_]->MODE |= FTM_MODE_FTMEN_MASK; //a better way to update the CnV value. SEE TABLE 40-7 ON PAGE 1054.
//		FTMstop(FTM_0); //easiest and the worst way to update the CnV value. SEE TABLE 40-7 ON PAGE 1054.
//		if ((uint16_t)ftm[ftm_]->CONTROLS[ch_].CnV + 1000 > (uint16_t)ftm[ftm_]->MOD)
//			ftm[ftm_]->CONTROLS[ch_].CnV = (uint16_t)ftm[ftm_]->CNTIN + 1000;
//		else
//			ftm[ftm_]->CONTROLS[ch_].CnV = (uint16_t)ftm[ftm_]->CONTROLS[ch_].CnV + 1000;
//		FTMstart(FTM_0);
	}
	if (temp2 & FTM_CnSC_CHF_MASK) { //when CNT == CnV
		ftm[ftm_]->CONTROLS[ch_].CnSC &= ~FTM_CnSC_CHF_MASK; //to clear the CHF flag.
		//ftm[ftm_]->MODE &= ~FTM_MODE_FTMEN_MASK;
//		if (getFTMcompval(ftm_, ch_) + 1000 > (uint16_t)ftm[ftm_]->MOD)
//			setFTMcompval(ftm_, ch_, (uint16_t)ftm[ftm_]->CNTIN + 1000);
//		else
//			setFTMcompval(ftm_, ch_, getFTMcompval(ftm_, ch_) + 1000);
	}
}

__ISR__ FTM0_IRQHandler (void) {
	NVIC_DisableIRQ(FTM0_IRQn);

	ISR_OUTCOMP(FTM_0, FTM_CH0);
	//ISR_PWM(FTM_0, FTM_CH0);

	NVIC_EnableIRQ(FTM0_IRQn);
}

__ISR__ FTM3_IRQHandler (void) {
	NVIC_DisableIRQ(FTM3_IRQn);

	ISR_INCAPT(FTM_3, FTM_CH5);

	NVIC_EnableIRQ(FTM3_IRQn);
}
