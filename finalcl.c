#include "F28x_Project.h"

float Kp = 1.0;
float Ki = 0.1;
float Ts = 0.00001; // 10 탎

float ref = 1.0;
float feedback = 0.5;
float error = 0.0;
float integral = 0.0;
float control_output = 0.0;

float duty_ratio = 0.5;
int use_PI = 0;  // 0 = Manual, 1 = PI control

void ConfigureEPWM1(void);
void ConfigureEPWM2(void);
void ConfigureEPWM3(void);
void ConfigureEPWM5(void); // Final adjusted PWM
void ConfigureCPUTimer0(void);
__interrupt void controlISR(void);

void main(void)
{
    InitSysCtrl();

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    InitEPwm1Gpio();  // GPIO0 = ePWM1A
    InitEPwm2Gpio();  // GPIO2 = ePWM2A
    InitEPwm3Gpio();  // GPIO4 = ePWM3A, GPIO5 = ePWM3B
    InitEPwm5Gpio();  // GPIO8 = ePWM5A (adjusted signal)

    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

    EALLOW;
    PieVectTable.TIMER0_INT = &controlISR;
    EDIS;

    ConfigureEPWM1();
    ConfigureEPWM2();
    ConfigureEPWM3();
    ConfigureEPWM5();

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    ConfigureCPUTimer0();

    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    IER |= M_INT1;
    EINT;
    ERTM;

    while (1) {}
}

void ConfigureEPWM1(void)
{
    EPwm1Regs.TBPRD = 1999;
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm1Regs.CMPA.bit.CMPA = (Uint16)(duty_ratio * EPwm1Regs.TBPRD);
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
}

void ConfigureEPWM2(void)
{
    EPwm2Regs.TBPRD = 1999;
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm2Regs.CMPA.bit.CMPA = 0;
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm2Regs.TBPHS.bit.TBPHS = 0;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;

}

void ConfigureEPWM3(void)
{
    EPwm3Regs.TBPRD = 1999;
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm3Regs.CMPA.bit.CMPA = 0;
    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm3Regs.AQCTLA.bit.ZRO = AQ_SET;
//    EPwm3Regs.AQCTLB.bit.CAU = AQ_SET;
//    EPwm3Regs.AQCTLB.bit.ZRO = AQ_CLEAR;
    EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm3Regs.TBPHS.bit.TBPHS = 0;
    EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;

}

void ConfigureEPWM5(void)
{
    EPwm5Regs.TBPRD = 1999;
    EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm5Regs.CMPA.bit.CMPA = 0;
    EPwm5Regs.CMPB.bit.CMPB = 0;
    EPwm5Regs.AQCTLA.bit.CAU = AQ_SET;    // ON at CMPA
    EPwm5Regs.AQCTLA.bit.CBU = AQ_CLEAR;  // OFF at CMPB
    EPwm5Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm5Regs.TBPHS.bit.TBPHS = 0;
    EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;

}

void ConfigureCPUTimer0(void)
{
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer0, 200, 10);  // 10 탎
    CpuTimer0Regs.TCR.all = 0x4001;
}

__interrupt void controlISR(void)
{
    Uint16 tbprd = EPwm1Regs.TBPRD;
    float base_cmpa;

    if (use_PI)
    {
        error = ref - feedback;
        integral += error * Ts;
        if (integral > 10.0f) integral = 10.0f;
        if (integral < 0.0f) integral = 0.0f;
        control_output = Kp * error + Ki * integral;
        if (control_output > tbprd) control_output = tbprd;
        if (control_output < 0.0f) control_output = 0.0f;
        base_cmpa = control_output;
    }
    else
    {
        if (duty_ratio > 1.0f) duty_ratio = 1.0f;
        if (duty_ratio < 0.0f) duty_ratio = 0.0f;
        base_cmpa = duty_ratio * tbprd;
    }

    EPwm1Regs.CMPA.bit.CMPA = (Uint16)(base_cmpa);

    Uint16 cmpa2 = (Uint16)(base_cmpa + 200);  // +10 탎
    if (cmpa2 > tbprd) cmpa2 = tbprd;
    EPwm2Regs.CMPA.bit.CMPA = cmpa2;

    Uint16 cmpa3 = (Uint16)(base_cmpa + 300);  // +15 탎
    if (cmpa3 > tbprd) cmpa3 = tbprd;
    EPwm3Regs.CMPA.bit.CMPA = cmpa3;

    // ePWM5A: ON at CMPA3 (same as ePWM3B), OFF 10 탎 before TBPRD
    Uint16 on_time = cmpa3;
    Uint16 off_time = tbprd - 200;  // 10 탎 before end
    if (off_time <= on_time) off_time = on_time + 1;

    EPwm5Regs.CMPA.bit.CMPA = on_time;
    EPwm5Regs.CMPB.bit.CMPB = off_time;

    CpuTimer0Regs.TCR.bit.TIF = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
