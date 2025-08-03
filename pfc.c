#include "F28x_Project.h"
#include <math.h>

#define PWM_FREQ        100000
#define DEADTIME_NS     200
#define ADC_VREF        3.3f
#define ADC_RES         4095.0f
#define VREF_TARGET     2.0f

float tbprd;
float Vref = VREF_TARGET;
float Vfb = 0;
float error = 0, integral = 0;
float Kp = 0.5f, Ki = 0.01f;
float duty = 0.5f;
float cmpa_val;

void InitADC(void)
{
    EALLOW;
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    DELAY_US(1000);

    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14;
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;
    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;
    AdcaRegs.ADCINTSEL1N2.bit.INT1CONT = 0;
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    EDIS;
}

void InitPWM(void)
{
    tbprd = (200e6 / (PWM_FREQ * 2)) - 1;
    float deadband = DEADTIME_NS * (200e6 / 1e9);

    EPwm1Regs.TBPRD = (Uint16)tbprd;
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;
    EPwm1Regs.TBCTL.bit.PHSEN = 0;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = 1;
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = 1;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = 0;
    EPwm1Regs.CMPA.bit.CMPA = (Uint16)(duty * tbprd);
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm1Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm1Regs.DBCTL.bit.POLSEL = 2;
    EPwm1Regs.DBRED.bit.DBRED = (Uint16)deadband;
    EPwm1Regs.DBFED.bit.DBFED = (Uint16)deadband;

    EPwm2Regs.TBPRD = (Uint16)tbprd;
    EPwm2Regs.TBCTL.bit.CTRMODE = 2;
    EPwm2Regs.TBCTL.bit.PHSEN = 1;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = 1;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = 0;
    EPwm2Regs.CMPA.bit.CMPA = (Uint16)(duty * tbprd);
    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;

    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm2Regs.DBCTL.bit.POLSEL = 2;
    EPwm2Regs.DBRED.bit.DBRED = (Uint16)deadband;
    EPwm2Regs.DBFED.bit.DBFED = (Uint16)deadband;

    EPwm1Regs.ETSEL.bit.SOCAEN = 1;
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_ZERO;
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST;
}

void InitGPIO(void)
{
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;  // EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;  // EPWM2A
    EDIS;
}

void main(void)
{
    InitSysCtrl();

    EALLOW;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
    CpuSysRegs.PCLKCR13.bit.ADC_A = 1;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    InitGPIO();
    InitADC();
    InitPWM();

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while (1)
    {
        AdcaRegs.ADCSOCFRC1.bit.SOC0 = 1;
        while (AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0);
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

        Vfb = (AdcaResultRegs.ADCRESULT0 * ADC_VREF) / ADC_RES;
        error = Vref - Vfb;
        integral += error;

        duty += Kp * error + Ki * integral;
        if (duty > 0.95f) duty = 0.95f;
        if (duty < 0.05f) duty = 0.05f;

        cmpa_val = duty * tbprd;
        EPwm1Regs.CMPA.bit.CMPA = (Uint16)cmpa_val;
        EPwm2Regs.CMPA.bit.CMPA = (Uint16)cmpa_val;

        DELAY_US(50);
    }
}
