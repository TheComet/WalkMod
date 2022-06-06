#include "anglemod/adc.h"
#include "anglemod/log.h"
#include <xc.h>

uint8_t adc_xy[2];
volatile uint8_t index = 0;

/* -------------------------------------------------------------------------- */
void adc_init(void)
{
    /* Set up a timer to control ADC conversion rate so we're not always
     * sampling at max frequency */
    T0CON1bits.T0CS = 0x04;  /* Use LFINTOSC as clock source (31 kHz) */
    T0CON1bits.ASYNC = 1;    /* Timer can only continue to count during sleep
                              * in asynchronous mode */
    
    /* Sample at 500 Hz initially -- 31 kHz / (31*2) = 500 Hz */
    TMR0H = 31 * 2;
    T0CON0bits.EN = 1;  /* Start timer */
    
    
    
    PIE0bits.TMR0IE = 1;
}

/* -------------------------------------------------------------------------- */
void adc_set_fast_sampling_mode(void)
{
    
}

/* -------------------------------------------------------------------------- */
void adc_set_slow_sampling_mode(void)
{
    
}

/* -------------------------------------------------------------------------- */
uint8_t adc_has_new_data_get_and_clear(void)
{
    uint8_t has_new_data = index > 1;
    index = 0;
    log_adc();
    return has_new_data;
}

/* -------------------------------------------------------------------------- */
const uint8_t* adc_joy_xy(void)
{
    return adc_xy;
}

/* -------------------------------------------------------------------------- */
void tim0_isr(void)
{
    index = 0;
    PIR0bits.TMR0IF = 0;
}

/* -------------------------------------------------------------------------- */
void adc_isr(void)
{
    uint8_t i = index;
    if (i == 0)
        adc_xy[0] = ADRESH;
    else
        adc_xy[1] = ADRESH;
    index = i + 1;
    
    PIR1bits.ADIF = 0;
}
