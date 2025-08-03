#include "F28x_Project.h"

// Function prototypes
void ConfigureADC(void);
void ConfigureDAC(void);
void ConfigureEPWM(void);
void ConfigureGPIO(void);

void main(void)
{
    Uint16 adcResult;

    InitSysCtrl();        // Initialize system clock

    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

    ConfigureGPIO();      // Configure GPIOs for ADC and DAC
    ConfigureADC();       // Configure ADC
    ConfigureDAC();       // Configure DAC

    while(1)
    {
        // Start SOC
        AdcaRegs.ADCSOCFRC1.all = 0x1;

        // Wait for conversion
        while(AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0);
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

        // Read ADC result
        adcResult = AdcaResultRegs.ADCRESULT0;

        // Scale and write to DAC
        DacbRegs.DACVALS.bit.DACVALS = adcResult >> 4; // 12-bit DAC, scale 16-bit ADC
    }
}

void ConfigureGPIO(void)
{
    EALLOW;

    // ADCINA0 -> GPIO0 (default analog function)
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 0;

    // DACB_OUT -> GPIO13
    GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 3;  // Set to DACB
    GpioCtrlRegs.GPADIR.bit.GPIO13 = 1;

    EDIS;
}

void ConfigureADC(void)
{
    EALLOW;
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;     // ADC clock = SYSCLK / 4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;     // Power up ADC
    DELAY_US(1000);                        // Delay for ADC power-up

    // SOC0 config - trigger by software, channel A0
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;     // ADCINA0
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14;    // Acquisition window
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 0;   // Software trigger

    // Interrupt config
    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0; // EOC0 triggers INT1
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   // Enable INT1
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    EDIS;
}

void ConfigureDAC(void)
{
    EALLOW;
    DacbRegs.DACCTL.bit.DACREFSEL = 1;     // Use VDAC reference
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;    // Enable DAC output
    DacbRegs.DACVALS.bit.DACVALS = 0;      // Initial value
    DELAY_US(10);                          // DAC power-up time
    EDIS;
}
