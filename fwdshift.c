#include "F28x_Project.h"
float x=0;
float tbprd_val = 1000;
float cmpa_val;
float duty_percent = 50;   // 50% duty cycle
void main(void)
{
    InitSysCtrl();

    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; // ePWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1; // ePWM2A
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;
    EDIS;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    EPwm1Regs.TBCTL.bit.CTRMODE = 2;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm1Regs.TBCTL.bit.PHSEN = 0;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = 1;

    EPwm2Regs.TBCTL.bit.CTRMODE = 2;
    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm2Regs.TBCTL.bit.PHSEN = 1;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while (1){
        EPwm1Regs.TBPRD = tbprd_val;
        EPwm2Regs.TBPRD = tbprd_val;
        cmpa_val = (duty_percent*0.01)*tbprd_val;
        EPwm1Regs.CMPA.bit.CMPA = cmpa_val;
        EPwm2Regs.CMPA.bit.CMPA = cmpa_val;
        EPwm2Regs.TBPHS.bit.TBPHS = x;
    }
}
