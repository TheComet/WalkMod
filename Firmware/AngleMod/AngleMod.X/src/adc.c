#include "anglemod/adc.h"
#include "anglemod/log.h"
#include <xc.h>

static uint8_t adc_xy[2];
static volatile uint8_t has_new_data = 0;
static uint8_t log_counter = 0;

/* -------------------------------------------------------------------------- */
void adc_init(void)
{
    /* Set up a timer to control ADC conversion rate so we're not always
     * sampling at max frequency */
    T0CON1 = 0x95;  /* LFINTOSC as clock source (31 kHz), ASYNC=1 (Timer can
                     * only continue to count during sleep in asynchronous mode),
                     * CKPS=5 (prescale with 1:32) */
    ADCON1 = 0x70;  /* FM=0 (left-justified), CS=111 (ADCRC clock source),
                     * PREF=00 (VREF is connected to VDD) */
    ADACT = 0x02;   /* Use TMR0_overflow as conversion trigger */
    ADCON0 = 0x35;  /* CHS=001101 (RB5), ON=1 */
    
    adc_set_slow_sampling_mode();
    
    PIE1bits.ADIE = 1;   /* Enable ADC interrupt */
    T0CON0bits.EN = 1;   /* Start timer */
}

/* -------------------------------------------------------------------------- */
void adc_set_fast_sampling_mode(void)
{
    /* Roughly 500 Hz */
    TMR0H = 1;
}

/* -------------------------------------------------------------------------- */
void adc_set_slow_sampling_mode(void)
{
    /* 200 Hz should be fast enough to detect joystick command inputs
     * LFOSC frequency is 31 kHz, prescaler is 32:
     *   31 kHz / 32 / 5 = ~200 Hz */
    TMR0H = 5;
}

/* -------------------------------------------------------------------------- */
uint8_t adc_has_new_data_get_and_clear(void)
{
    uint8_t result = has_new_data;
    has_new_data = 0;
    if (result)
    {
        if (log_counter-- == 0)
        {
            log_adc();
            log_counter = 10;
        }
    }
    return result;
}

/* -------------------------------------------------------------------------- */
const uint8_t* adc_joy_xy(void)
{
    return adc_xy;
}

/* -------------------------------------------------------------------------- */
void adc_isr(void)
{
    PIR1bits.ADIF = 0;
    
    /* 
     * Toggle between measuring the JOYX and JOYY signals, and set the global
     * "has new data" flag whenever we finish measuring both so the results can
     * be picked up in the main thread
     */
    
    if (ADCON0 == 0x35)
    {
        adc_xy[0] = ADRESH;
        ADCON0 = 0x39;  /* CHS=001110 (RB6), ON=1 */
    }
    else
    {
        adc_xy[1] = ADRESH;
        ADCON0 = 0x35;  /* CHS=001101 (RB5), ON=1 */
        has_new_data = 1;
    }
}
