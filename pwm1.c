#include "F28x_Project.h"

#define PWM_PERIOD 1000
#define DELAY_D    100
#define LED_GPIO   31

void InitEPwm1(void);
void InitLEDPin(void);
void BlinkLEDWithPWM(void);

void main(void)
{
    InitSysCtrl();
    InitGpio();

    InitEPwm1Gpio();
    InitLEDPin();

    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

    InitEPwm1();

    EINT;
    ERTM;

    while(1)
    {

        BlinkLEDWithPWM();
    }
}

void InitLEDPin()
{
    EALLOW;
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;
    EDIS;
}

void InitEPwm1()
{
    EALLOW;

    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    EPwm1Regs.TBPRD = PWM_PERIOD / 2;
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm1Regs.TBCTR = 0;


    EPwm1Regs.CMPA.bit.CMPA = (PWM_PERIOD / 2) - DELAY_D;

    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;

    EDIS;
}

void BlinkLEDWithPWM()
{
    GpioDataRegs.GPASET.bit.GPIO0 = 1;
    DELAY_US(((PWM_PERIOD / 2) + DELAY_D) * 5);

    GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;
    DELAY_US((PWM_PERIOD - ((PWM_PERIOD / 2) + DELAY_D)) * 5);
}
