#include "driverlib.h"
#include "device.h"
#include <math.h>

float voref = 5.0f, iLref = 0.0f;
float vo = 0, iL = 0, vi = 0;
float e_vo = 0, e_vo_prev = 0, PI_vo = 0;
float e_iL = 0, e_iL_prev = 0, PI_iL = 0;
float duty = 0.0f;

#define K1_VO  0.6f
#define K2_VO -0.59f
#define K1_IL  0.1f
#define K2_IL -0.09f

#define PWM_FREQ_HZ 100000U
#define PWM_TBPRD   (DEVICE_SYSCLK_FREQ / (PWM_FREQ_HZ * 2))

void initADC(void);
void initEPWM(void);
void initCPUTimer(void);
__interrupt void cpuTimer0ISR(void);

void main(void)
{
    Device_init();
    Device_initGPIO();

    Interrupt_initModule();
    Interrupt_initVectorTable();

    Interrupt_register(INT_TIMER0, &cpuTimer0ISR);

    initADC();
    initEPWM();
    initCPUTimer();

    Interrupt_enable(INT_TIMER0);
    EINT;
    ERTM;

    while (1);
}

void initADC(void)
{
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    ADC_enableConverter(ADCA_BASE);
    DEVICE_DELAY_US(1000);

    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN0, 15); // vo
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN1, 15); // iL
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN2, 15); // vi

    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER2);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
}

void initEPWM(void)
{
    GPIO_setPinConfig(0x00060001); // GPIO_6_EPWM4_A
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);

    EPWM_setTimeBasePeriod(EPWM4_BASE, PWM_TBPRD);
    EPWM_setTimeBaseCounterMode(EPWM4_BASE, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_setClockPrescaler(EPWM4_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);

    EPWM_setCounterCompareValue(EPWM4_BASE, EPWM_COUNTER_COMPARE_A, 0);
    EPWM_setCounterCompareShadowLoadMode(EPWM4_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);

    EPWM_setActionQualifierAction(EPWM4_BASE, EPWM_AQ_OUTPUT_A,
                                   EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(EPWM4_BASE, EPWM_AQ_OUTPUT_A,
                                   EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
}

void initCPUTimer(void)
{
    CPUTimer_setPeriod(CPUTIMER0_BASE, DEVICE_SYSCLK_FREQ / 10000); // 100us
    CPUTimer_setPreScaler(CPUTIMER0_BASE, 0);
    CPUTimer_enableInterrupt(CPUTIMER0_BASE);
    CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);
    CPUTimer_startTimer(CPUTIMER0_BASE);
}

__interrupt void cpuTimer0ISR(void)
{

    ADC_forceSOC(ADCA_BASE, ADC_SOC_NUMBER0);
    ADC_forceSOC(ADCA_BASE, ADC_SOC_NUMBER1);
    ADC_forceSOC(ADCA_BASE, ADC_SOC_NUMBER2);

    while (!ADC_getInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1));
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);

    vo = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0) * 3.3f / 4096.0f;
    iL = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1) * 3.3f / 4096.0f;
    vi = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2) * 3.3f / 4096.0f;

    e_vo = voref - vo;
    PI_vo += K1_VO * e_vo + K2_VO * e_vo_prev;
    e_vo_prev = e_vo;
    iLref = fminf(fmaxf(PI_vo, 0.0f), 3.0f); // Clamp iLref

    e_iL = iLref - iL;
    PI_iL += K1_IL * e_iL + K2_IL * e_iL_prev;
    e_iL_prev = e_iL;

    duty = fminf(fmaxf(PI_iL, 0.0f), 0.95f);
    uint16_t cmp = (uint16_t)(duty * PWM_TBPRD);
    EPWM_setCounterCompareValue(EPWM4_BASE, EPWM_COUNTER_COMPARE_A, cmp);

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

