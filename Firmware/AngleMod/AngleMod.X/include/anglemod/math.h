/*!
 * @file math.h
 * @author TheComet
 */

#ifndef MATH_H
#define	MATH_H

#include <stdint.h>

int8_t fpsin(uint16_t a);

#define fpcos(a) fpsin((a) + 90)

uint16_t fpatan2(int8_t x, int8_t y);

#endif	/* MATH_H */
