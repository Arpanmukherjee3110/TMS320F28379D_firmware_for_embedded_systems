#include "F28x_Project.h"

volatile int x = 0;
float tbprd_val = 1000;
float cmpa_val;
float duty_percent=50;

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

    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    EPwm1Regs.TBPRD = tbprd_val;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;

    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    EPwm2Regs.TBPRD = tbprd_val;
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;

    EPwm2Regs.TBCTR = 0;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while (1)
    {
        cmpa_val = (duty_percent*0.01f)*tbprd_val;
        EPwm1Regs.CMPA.bit.CMPA = cmpa_val;
        EPwm2Regs.CMPA.bit.CMPA = cmpa_val;
        EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
        EPwm2Regs.TBPHS.bit.TBPHS = (tbprd_val - x);

    }
}
