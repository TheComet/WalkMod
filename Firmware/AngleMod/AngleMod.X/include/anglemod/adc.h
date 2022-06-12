/*!
 * @file adc.h
 * @author TheComet
 */

#ifndef ADC_H
#define	ADC_H

#include <stdint.h>

void adc_init(void);

void adc_set_fast_sampling_mode(void);
void adc_set_slow_sampling_mode(void);
uint8_t adc_has_new_data_get_and_clear(void);

const uint8_t* adc_joy_xy(void);

void adc_isr(void);

#endif	/* ADC_H */
