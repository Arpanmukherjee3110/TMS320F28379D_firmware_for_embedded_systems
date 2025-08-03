#include "F28x_Project.h"

float cmpa_val, PWM_PERIOD = 1000;
float desired_freq = 40000.0;// change to adjust frequency TBPRD
float epwm_clk = 100e6;
int TBPRD_val= 1000;
float duty_percent=50.0;

void main(void) {

    InitSysCtrl();

    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; // GPIO0 = ePWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1; // GPIO2 = ePWM2A
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;
    EDIS;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    EPwm1Regs.TBCTL.bit.CTRMODE = 2;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm1Regs.TBCTL.bit.PHSEN = 0;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = 1;

    EPwm2Regs.TBCTL.bit.CTRMODE = 2;
    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm2Regs.TBCTL.bit.PHSEN = 1;
    EPwm2Regs.TBPHS.bit.TBPHS = 0;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while (1) {
        TBPRD_val = (int)(epwm_clk / (2 * desired_freq));
        cmpa_val= (duty_percent*0.01)*TBPRD_val;
        EPwm1Regs.TBPRD = TBPRD_val;
        EPwm2Regs.TBPRD = TBPRD_val;
        EPwm1Regs.CMPA.bit.CMPA = cmpa_val;
        EPwm2Regs.CMPA.bit.CMPA = cmpa_val;
    }
}
