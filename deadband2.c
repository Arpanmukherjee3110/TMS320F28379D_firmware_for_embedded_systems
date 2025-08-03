#include "F28x_Project.h"

float x = 0;
float tbprd_val = 500;
float cmpa_val;
float duty_percent = 50;
float deadband_delay = 50;

void main(void)
{
    InitSysCtrl();

    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; // ePWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1; // ePWM2A
    EDIS;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    EPwm1Regs.TBCTL.bit.CTRMODE = 0;
    EPwm1Regs.TBPRD = tbprd_val;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm1Regs.TBCTL.bit.PHSEN = 0;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = 1;

    EPwm2Regs.TBCTL.bit.CTRMODE = 0;
    EPwm2Regs.TBPRD = tbprd_val;
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm2Regs.TBCTL.bit.PHSEN = 0;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = 1;

    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm2Regs.DBCTL.bit.POLSEL = 2;


    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while (1)
    {
        cmpa_val = (duty_percent * 0.01) * tbprd_val;
        EPwm1Regs.CMPA.bit.CMPA = cmpa_val;
        EPwm2Regs.CMPA.bit.CMPA = cmpa_val;
        EPwm2Regs.DBRED.all = deadband_delay;
        EPwm2Regs.DBFED.all = deadband_delay;

    }
}
