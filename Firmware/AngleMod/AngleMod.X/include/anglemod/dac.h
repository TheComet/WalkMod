/*!
 * @file dac.h
 * @author TheComet
 */

#ifndef DAC_H
#define	DAC_H

#include "anglemod/cmd_seq.h"
#include <stdint.h>

void dac_init(void);

void dac_set_clamp_threshold(uint8_t x, uint8_t y);

void dac_override_disable(void);
void dac_override_clamp(uint8_t x, uint8_t y);
void dac_override_sequence(enum cmd_seq seq);

#endif	/* DAC_H */
