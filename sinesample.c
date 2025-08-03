#include "F28x_Project.h"
#include <math.h>

#define PWM_FREQ      400000
#define SAMPLE_SIZE   200
#define DEADBAND_NS   200
#define ADC_VREF      3.3f
#define ADC_RES       4095.0f

float tbprd;
Uint16 sine_index = 0;
float sine_table[SAMPLE_SIZE];

// PI Controller variables
float Vref = 1.5f;
float Vfb = 0.0f;
float error = 0.0f;
float integral = 0.0f;
float Kp = 0.4f;
float Ki = 0.01f;
float modulation_amplitude = 1.0f;

__interrupt void epwm1_isr(void) {
    // Trigger ADC
    AdcaRegs.ADCSOCFRC1.bit.SOC0 = 1;
    while (AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0);
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

    // Read ADC and compute PI control
    Vfb = ((float)AdcaResultRegs.ADCRESULT0 * ADC_VREF) / ADC_RES;
    error = Vref - Vfb;
    integral += error;
    modulation_amplitude += (Kp * error + Ki * integral);

    if (modulation_amplitude > 1.0f) modulation_amplitude = 1.0f;
    if (modulation_amplitude < 0.0f) modulation_amplitude = 0.0f;

    // Generate SPWM
    float mod = sine_table[sine_index] * modulation_amplitude;
    Uint16 compare = (Uint16)((mod + 1.0f) * (tbprd / 2.0f));
    EPwm1Regs.CMPA.bit.CMPA = compare;
    EPwm2Regs.CMPA.bit.CMPA = compare;

    sine_index = (sine_index + 1) % SAMPLE_SIZE;

    // Clear interrupt
    EPwm1Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

void InitADC(void) {
    EALLOW;
    CpuSysRegs.PCLKCR13.bit.ADC_A = 1;
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    DELAY_US(1000);

    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;  // ADCINA0
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14;
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;  // EPWM1SOCA
    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;
    AdcaRegs.ADCINTSEL1N2.bit.INT1CONT = 0;
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    EDIS;
}

void InitEPWM(void) {
    tbprd = (float)(200e6 / (PWM_FREQ * 2) - 1);
    float deadband_clocks = DEADBAND_NS * (200e6 / 1e9);

    // EPWM1A
    EPwm1Regs.TBPRD = (Uint16)tbprd;
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;
    EPwm1Regs.TBCTL.bit.PHSEN = 0;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = 1;

    EPwm1Regs.CMPCTL.bit.SHDWAMODE = 1;
    EPwm1Regs.CMPA.bit.CMPA = (Uint16)(tbprd / 2);
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;

    EPwm1Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm1Regs.DBCTL.bit.POLSEL = 2;
    EPwm1Regs.DBRED.bit.DBRED = (Uint16)deadband_clocks;
    EPwm1Regs.DBFED.bit.DBFED = (Uint16)deadband_clocks;

    EPwm1Regs.ETSEL.bit.SOCAEN = 1;
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_ZERO;
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST;

    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;
    EPwm1Regs.ETSEL.bit.INTEN = 1;
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;

    // EPWM2A (complementary)
    EPwm2Regs.TBPRD = (Uint16)tbprd;
    EPwm2Regs.TBCTL.bit.CTRMODE = 2;
    EPwm2Regs.TBCTL.bit.PHSEN = 1;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;

    EPwm2Regs.CMPCTL.bit.SHDWAMODE = 1;
    EPwm2Regs.CMPA.bit.CMPA = (Uint16)(tbprd / 2);
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm2Regs.DBCTL.bit.POLSEL = 2;
    EPwm2Regs.DBRED.bit.DBRED = (Uint16)deadband_clocks;
    EPwm2Regs.DBFED.bit.DBFED = (Uint16)deadband_clocks;
}

void InitGPIO(void) {
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;  // EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;  // EPWM2A
    EDIS;
}

void main(void) {
    InitSysCtrl();
    InitPieCtrl();
    InitPieVectTable();

    // Generate sine table
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        sine_table[i] = sinf(2.0f * M_PI * i / SAMPLE_SIZE);
    }

    EALLOW;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
    CpuSysRegs.PCLKCR13.bit.ADC_A = 1;
    EDIS;

    InitADC();
    InitGPIO();
    InitEPWM();

    EALLOW;
    PieVectTable.EPWM1_INT = &epwm1_isr;
    EDIS;

    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
    IER |= M_INT3;
    EINT;
    ERTM;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    while (1) {
        // Closed-loop runs inside ISR
    }
}
