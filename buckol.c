#include "F28x_Project.h"
float tbprd_val= 500.0;
float duty_percent = 50.0;
float cmpa;
void main(void) {
    InitSysCtrl();

    EALLOW;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    EDIS;

    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; // GPIO0 = ePWM1A
    EDIS;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    EPwm1Regs.TBCTL.bit.CTRMODE = 2;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while(1){
        EPwm1Regs.TBPRD = tbprd_val;
        cmpa = (duty_percent*0.01)*tbprd_val;
        EPwm1Regs.CMPA.bit.CMPA = cmpa;
    }
}
