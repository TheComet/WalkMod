#include "anglemod/joy.h"
#include "anglemod/log.h"
#include <xc.h>

volatile uint8_t joy_data[2];
volatile uint8_t index = 0;

/* -------------------------------------------------------------------------- */
void joy_init(void)
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
void joy_set_fast_sampling_mode(void)
{
    
}

/* -------------------------------------------------------------------------- */
void joy_set_slow_sampling_mode(void)
{
    
}

/* -------------------------------------------------------------------------- */
uint8_t joy_has_new_data_get_and_clear(void)
{
    uint8_t has_new_data = index > 1;
    index = 0;
    log_adc();
    return has_new_data;
}

/* -------------------------------------------------------------------------- */
uint8_t joy_x(void)
{
    return joy_data[0];
}

/* -------------------------------------------------------------------------- */
uint8_t joy_y(void)
{
    return joy_data[1];
}

/* -------------------------------------------------------------------------- */
void joy_tim0_isr(void)
{
    index = 0;
    PIR0bits.TMR0IF = 0;
}

/* -------------------------------------------------------------------------- */
void joy_adc_isr(void)
{
    uint8_t i = index;
    if (i == 0)
        joy_data[0] = ADRESH;
    else
        joy_data[1] = ADRESH;
    index = i + 1;
    
    PIR1bits.ADIF = 0;
}
