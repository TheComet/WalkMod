/*!
 * @file joy.h
 * @author TheComet
 */

#ifndef JOY_H
#define	JOY_H

#include <stdint.h>

void joy_init(void);

void joy_set_fast_sampling_mode(void);
void joy_set_slow_sampling_mode(void);

uint8_t joy_has_new_data_get_and_clear(void);
uint8_t joy_x(void);
uint8_t joy_y(void);

void joy_tim0_isr(void);
void joy_adc_isr(void);

#endif	/* JOY_H */
