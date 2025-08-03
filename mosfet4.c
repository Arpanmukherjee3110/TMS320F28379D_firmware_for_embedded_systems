#include "F28x_Project.h"
float tbprd_val, cmpa_val=250.0, x=20, desired_freq=100000.0;
float epwm_clk= 100e6;
void main(void){
    InitSysCtrl();

    tbprd_val = epwm_clk/(2*desired_freq);
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;
    EDIS;
    InitEPwm5Gpio();

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    EPwm1Regs.TBCTL.bit.CTRMODE = 1;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm1Regs.TBCTL.bit.PHSEN = 0;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = 0;

    EPwm2Regs.TBCTL.bit.CTRMODE = 1;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.PRD = AQ_SET;
    EPwm2Regs.TBCTL.bit.PHSEN = 1;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = 1;

    EPwm3Regs.TBCTL.bit.CTRMODE = 1;
    EPwm3Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm3Regs.AQCTLA.bit.PRD = AQ_SET;
    EPwm3Regs.TBCTL.bit.PHSEN = 0;
    EPwm3Regs.TBCTL.bit.SYNCOSEL = 0;

    EPwm4Regs.TBCTL.bit.CTRMODE = 1;
    EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm4Regs.AQCTLA.bit.PRD = AQ_CLEAR;
    EPwm4Regs.TBCTL.bit.PHSEN = 0;
    EPwm4Regs.TBCTL.bit.SYNCOSEL = 1;

    EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
        EPwm5Regs.CMPA.bit.CMPA = 0;
        EPwm5Regs.CMPB.bit.CMPB = 0;
        EPwm5Regs.AQCTLA.bit.CAU = AQ_SET;    // ON at CMPA
        EPwm5Regs.AQCTLA.bit.CBU = AQ_CLEAR;  // OFF at CMPB
        EPwm5Regs.TBCTL.bit.PHSEN = TB_ENABLE;
        EPwm5Regs.TBPHS.bit.TBPHS = 0;
        EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while(1){
        tbprd_val = epwm_clk/(2*desired_freq);
        EPwm1Regs.TBPRD = tbprd_val;
        EPwm2Regs.TBPRD = tbprd_val;
        EPwm3Regs.TBPRD = tbprd_val;
        EPwm4Regs.TBPRD = tbprd_val;
        EPwm5Regs.TBPRD = tbprd_val;
        EPwm2Regs.TBPHS.bit.TBPHS=2*x;
        EPwm1Regs.CMPA.bit.CMPA = cmpa_val;
        EPwm2Regs.CMPA.bit.CMPA = cmpa_val;
        EPwm3Regs.CMPA.bit.CMPA = cmpa_val-x;
        EPwm4Regs.CMPA.bit.CMPA = cmpa_val-2*x;
        Uint16 on_time = cmpa_val+2*x;
        Uint16 off_time = tbprd_val - x;  // 10 µs before end
            if (off_time <= on_time) off_time = on_time + 1;

        EPwm5Regs.CMPA.bit.CMPA = on_time;
        EPwm5Regs.CMPB.bit.CMPB = off_time;

        CpuTimer0Regs.TCR.bit.TIF = 1;
            PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

    }
}
