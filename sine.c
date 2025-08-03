#include "F28x_Project.h"
#include <math.h>

#define PWM_FREQ     400000
#define SAMPLE_SIZE  200
Uint16 i;
float tbprd;
Uint16 sine_index = 0;
float sine_table[SAMPLE_SIZE];

__interrupt void epwm1_isr(void)
{
    EPwm1Regs.CMPA.bit.CMPA = (Uint16)((sine_table[sine_index] + 1.0f) * (tbprd / 2.0f));
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
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;
    EDIS;

    tbprd = (200e6 / (PWM_FREQ * 2)) - 1;

    for (i = 0; i < SAMPLE_SIZE; i++) {
        sine_table[i] = sinf(2.0f * 3.14159265f * i / SAMPLE_SIZE);  // Range [-1, 1]
    }

    EPwm1Regs.TBPRD = (Uint16)tbprd;
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;
    EPwm1Regs.TBCTL.bit.PHSEN = 0;
    EPwm1Regs.TBCTL.bit.PRDLD = 1;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = 3;

    EPwm1Regs.CMPCTL.bit.SHDWAMODE = 1;
    EPwm1Regs.CMPA.bit.CMPA = (Uint16)(tbprd / 2.0f);

    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm1Regs.ETSEL.bit.INTSEL = 1;
    EPwm1Regs.ETSEL.bit.INTEN = 1;
    EPwm1Regs.ETPS.bit.INTPRD = 1;


    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;

    IER |= M_INT3;
    EINT;
    ERTM;

    EPwm1Regs.TBCTL.bit.CTRMODE = 2;

    while (1) {

    }
}
