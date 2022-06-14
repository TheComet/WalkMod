/*!
 * @file dac.h
 * @author TheComet
 */

#ifndef DAC_H
#define	DAC_H

#include <stdint.h>

void dac_init(void);

void dac_override_disable(void);
void dac_override_clamp(const uint8_t xy[2]);
void dac_override(const uint8_t xy[2]);

#endif	/* DAC_H */
