/*!
 * @file dac.h
 * @author TheComet
 */

#ifndef DAC_H
#define	DAC_H

#include "anglemod/seq.h"
#include <stdint.h>

void dac_init(void);

void dac_override_disable(void);
void dac_override_clamp(const uint8_t xy[2]);
void dac_override_sequence(enum seq seq);

#endif	/* DAC_H */
