#include "F28x_Project.h"
#include <math.h>

#define PWM_FREQ      400000
#define SPWM_FREQ     50
#define SAMPLE_SIZE   200
#define DEADBAND_NS   200
#define ADC_VREF      3.3f
#define ADC_RES       4095.0f

float tbprd;
Uint16 sine_index = 0;
float sine_table[SAMPLE_SIZE];

// PI for Voltage Loop (runs slower)
float Vref = 1.5f;
float Vfb = 0.0f;
float Verr = 0.0f;
float Verr_integral = 0.0f;
float V_Kp = 0.5f;
float V_Ki = 0.01f;
float Iref = 0.0f;

// PI for Current Loop (runs at SPWM speed)
float Ifb = 0.0f;
float Ierr = 0.0f;
float Ierr_integral = 0.0f;
float I_Kp = 0.6f;
float I_Ki = 0.02f;

__interrupt void cpu_timer0_isr(void) {
    // Outer Voltage Loop
    AdcaRegs.ADCSOCFRC1.all = 0x03;  // Trigger both SOC0 and SOC1
    while (AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0);
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

    Vfb = ((float)AdcaResultRegs.ADCRESULT0 * ADC_VREF) / ADC_RES;
    Verr = Vref - Vfb;
    Verr_integral += Verr;

    Iref = V_Kp * Verr + V_Ki * Verr_integral;

    if (Iref > 1.0f) Iref = 1.0f;
    if (Iref < -1.0f) Iref = -1.0f;

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
    CpuTimer0Regs.TCR.bit.TIF = 1;
}

__interrupt void epwm1_isr(void) {
    // Inner Current Loop
    Ifb = ((float)AdcaResultRegs.ADCRESULT1 * ADC_VREF) / ADC_RES;
    Ierr = Iref - Ifb;
    Ierr_integral += Ierr;

    float modulation_amplitude = I_Kp * Ierr + I_Ki * Ierr_integral;

    if (modulation_amplitude > 1.0f) modulation_amplitude = 1.0f;
    if (modulation_amplitude < -1.0f) modulation_amplitude = -1.0f;

    float mod = sine_table[sine_index];
    float compare_f = (mod * modulation_amplitude + 1.0f) * tbprd / 2.0f;
    if (compare_f > tbprd) compare_f = tbprd;
    if (compare_f < 0.0f) compare_f = 0.0f;
    Uint16 compare = (Uint16)compare_f;

    EPwm1Regs.CMPA.bit.CMPA = compare;
    EPwm2Regs.CMPA.bit.CMPA = compare;

    sine_index = (sine_index + 1) % SAMPLE_SIZE;

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
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;  // ADCINA0 for Voltage
    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;  // ADCINA1 for Current
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14;
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = 14;
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 0; // Software trigger
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 0;
    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 1;
    AdcaRegs.ADCINTSEL1N2.bit.INT1CONT = 0;
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    EDIS;
}

void InitEPWM(void) {
    tbprd = (float)(200e6 / (PWM_FREQ * 2) - 1);
    float dbclk = DEADBAND_NS * (200e6 / 1e9);

    EPwm1Regs.TBPRD = (Uint16)tbprd;
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;
    EPwm1Regs.TBCTL.bit.PHSEN = 0;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = 1;

    EPwm1Regs.CMPCTL.bit.SHDWAMODE = 1;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = 0;
    EPwm1Regs.CMPA.bit.CMPA = (Uint16)(tbprd / 2.0f);

    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;

    EPwm1Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm1Regs.DBCTL.bit.POLSEL = 2;
    EPwm1Regs.DBRED.bit.DBRED = (Uint16)dbclk;
    EPwm1Regs.DBFED.bit.DBFED = (Uint16)dbclk;

    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;
    EPwm1Regs.ETSEL.bit.INTEN = 1;
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;

    EPwm2Regs.TBPRD = (Uint16)tbprd;
    EPwm2Regs.TBCTL.bit.CTRMODE = 2;
    EPwm2Regs.TBCTL.bit.PHSEN = 1;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;

    EPwm2Regs.CMPCTL.bit.SHDWAMODE = 1;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = 0;
    EPwm2Regs.CMPA.bit.CMPA = (Uint16)(tbprd / 2.0f);

    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm2Regs.DBCTL.bit.POLSEL = 2;
    EPwm2Regs.DBRED.bit.DBRED = (Uint16)dbclk;
    EPwm2Regs.DBFED.bit.DBFED = (Uint16)dbclk;
}

void InitGPIO(void) {
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;  // EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;  // EPWM2A
    EDIS;
}

void InitCPUTimer0(float freq) {
    ConfigCpuTimer(&CpuTimer0, 200, 1000000.0f / freq); // us
    CpuTimer0Regs.TCR.bit.TIE = 1;
    CpuTimer0Regs.TCR.bit.TSS = 0;
}

void main(void) {
    InitSysCtrl();
    int i;
    for (i = 0; i < SAMPLE_SIZE; i++) {
        sine_table[i] = sinf(2.0f * 3.14159265f * i / SAMPLE_SIZE);
    }

    EALLOW;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
    CpuSysRegs.PCLKCR13.bit.ADC_A = 1;
    EDIS;

    InitADC();
    InitGPIO();
    InitEPWM();
    InitCPUTimer0(200.0f);  // Voltage loop frequency ~200 Hz

    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

    EALLOW;
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    PieVectTable.EPWM1_INT = &epwm1_isr;
    EDIS;

    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
    IER |= M_INT1 | M_INT3;
    EINT; ERTM;

    while (1) {
        // Control handled in ISRs
    }
}
