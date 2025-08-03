#include "F28x_Project.h"

void main(void) {
    InitSysCtrl();

    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; // GPIO0 = ePWM1A
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
    EDIS;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    EPwm1Regs.TBCTL.bit.CTRMODE = 2;
    EPwm1Regs.TBPRD = 1000;
    EPwm1Regs.CMPA.bit.CMPA = 500;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while(1);
}
