#include "F28x_Project.h"
float x=200, y=0;
float tbprd_val = 1000;
float cmpa_val;
float duty_percent = 50;   // 50% duty cycle
float deadband_delay=50;

void main(void)
{
    InitSysCtrl();

    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; // ePWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1; // ePWM2A
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1; // ePWM3A
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1; // ePWM4A
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;
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
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm2Regs.TBCTL.bit.PHSEN = 0;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;

    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm2Regs.DBCTL.bit.POLSEL = 2;

    EPwm3Regs.TBCTL.bit.CTRMODE = 2;
    EPwm3Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm3Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm3Regs.TBCTL.bit.PHSEN = 1;
    EPwm3Regs.TBCTL.bit.SYNCOSEL = 1;

    EPwm4Regs.TBCTL.bit.CTRMODE = 2;
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm4Regs.TBCTL.bit.PHSEN = 1;
    EPwm4Regs.TBCTL.bit.SYNCOSEL = 0;

    EPwm4Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm4Regs.DBCTL.bit.POLSEL = 2;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while (1){
        EPwm1Regs.TBPRD = tbprd_val;
        EPwm2Regs.TBPRD = tbprd_val;
        EPwm3Regs.TBPRD = tbprd_val;
        EPwm4Regs.TBPRD = tbprd_val;

        cmpa_val = (duty_percent*0.01)*tbprd_val;
        EPwm1Regs.CMPA.bit.CMPA = cmpa_val;
        EPwm2Regs.CMPA.bit.CMPA = cmpa_val-deadband_delay;
        EPwm3Regs.CMPA.bit.CMPA = cmpa_val;
        EPwm4Regs.CMPA.bit.CMPA = cmpa_val-deadband_delay;

        EPwm1Regs.TBPHS.bit.TBPHS = y;
        EPwm2Regs.TBPHS.bit.TBPHS = y;
        EPwm3Regs.TBPHS.bit.TBPHS = x;
        EPwm4Regs.TBPHS.bit.TBPHS = x;

        EPwm2Regs.DBRED.all = deadband_delay;
        EPwm2Regs.DBFED.all = deadband_delay;

        EPwm4Regs.DBRED.all = deadband_delay;
        EPwm4Regs.DBFED.all = deadband_delay;
    }
}
