#include "F28x_Project.h"
#include <math.h>

#define PWM_FREQ     400000
#define SAMPLE_SIZE  200
#define DEAD_BAND_NS 200

float tbprd;
Uint16 sine_index = 0, i;
float sine_table[SAMPLE_SIZE];

__interrupt void epwm1_isr(void)
{
    Uint16 cmp_val = (Uint16)((sine_table[sine_index] + 1.0f) * (tbprd / 2.0f));
    EPwm1Regs.CMPA.bit.CMPA = cmp_val;
    EPwm2Regs.CMPA.bit.CMPA = cmp_val;

    sine_index = (sine_index + 1) % SAMPLE_SIZE;

    EPwm1Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

void main(void)
{
    InitSysCtrl();
    InitPieCtrl();
    InitPieVectTable();

    DINT;

    EALLOW;
    PieVectTable.EPWM1_INT = &epwm1_isr;
    EDIS;

    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;  // ePWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;  // ePWM2A
    EDIS;

    tbprd = (200e6 / (PWM_FREQ * 2.0f)) - 1;

    for (i = 0; i < SAMPLE_SIZE; i++) {
        sine_table[i] = sinf(2.0f * M_PI * i / SAMPLE_SIZE);
    }

    EPwm1Regs.TBPRD = (Uint16)tbprd;
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;
    EPwm1Regs.TBCTL.bit.PHSEN = 0;
    EPwm1Regs.TBCTL.bit.PRDLD = 1;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = 0;

    EPwm1Regs.CMPCTL.bit.SHDWAMODE = 1;
    EPwm1Regs.CMPA.bit.CMPA = (Uint16)(tbprd / 2.0);

    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;

    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;
    EPwm1Regs.ETSEL.bit.INTEN = 1;
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;

    EPwm2Regs.TBPRD = (Uint16)tbprd;
    EPwm2Regs.TBCTL.bit.CTRMODE = 2;
    EPwm2Regs.TBCTL.bit.PHSEN = 0;
    EPwm2Regs.TBCTL.bit.PRDLD = 1;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = 1;

    EPwm2Regs.CMPCTL.bit.SHDWAMODE = 1;
    EPwm2Regs.CMPA.bit.CMPA = (Uint16)(tbprd / 2.0);

    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;

    Uint16 deadband_clocks = (Uint16)(DEAD_BAND_NS * 200e6 / 1e9);
    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm2Regs.DBCTL.bit.POLSEL = 2;
    EPwm2Regs.DBRED.bit.DBRED = deadband_clocks;
    EPwm2Regs.DBFED.bit.DBFED = deadband_clocks;

    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
    IER |= M_INT3;

    EINT;


    while (1) {

    }
}
