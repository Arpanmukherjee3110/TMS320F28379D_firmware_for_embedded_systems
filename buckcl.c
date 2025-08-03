#include "F28x_Project.h"

#define MAX_DUTY 90.0f
#define MIN_DUTY 10.0f
#define ADC_VREF 3.3f
#define ADC_RES 4095.0f

float tbprd_val = 4000;
float duty_percent = 50;
float cmpa_val;

float Vref = 0.25f;
float Vfb = 0.0f;
float error = 0.0f;
float integral = 0.0f;
float Kp = 0.5f;
float Ki = 0.05f;

void InitAdc(void) {
    EALLOW;
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    EDIS;
}

void main(void) {
    InitSysCtrl();

    EALLOW;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR13.bit.ADC_A = 1;
    EDIS;

    InitAdc();

    InitGpio();
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO0 = 0;
    EDIS;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    EPwm1Regs.TBCTL.bit.CTRMODE = 0;
    EPwm1Regs.TBCTL.bit.PHSEN = 0;
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm1Regs.TBPRD = tbprd_val;
    EPwm1Regs.CMPA.bit.CMPA = (Uint16)((duty_percent * 0.01f) * tbprd_val);

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    EALLOW;
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    DELAY_US(1000);
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14;
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 0;
    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;
    AdcaRegs.ADCINTSEL1N2.bit.INT1CONT = 0;
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    EDIS;

    while (1) {
        AdcaRegs.ADCSOCFRC1.bit.SOC0 = 1;
        while (AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0);
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

        Vfb = (AdcaResultRegs.ADCRESULT0 * ADC_VREF) / ADC_RES;
        error = Vref - Vfb;
        integral += error;
        duty_percent += (Kp * error + Ki *integral);

        if (duty_percent > MAX_DUTY) {
            duty_percent = MAX_DUTY;
        }
        if (duty_percent < MIN_DUTY) {
            duty_percent = MIN_DUTY;
        }

        EPwm1Regs.TBPRD = tbprd_val;
        cmpa_val = (duty_percent * 0.01f) * tbprd_val;
        EPwm1Regs.CMPA.bit.CMPA = cmpa_val;

        DELAY_US(100);
    }
}
