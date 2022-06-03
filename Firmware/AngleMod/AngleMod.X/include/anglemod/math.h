/*!
 * @file math.h
 * @author TheComet
 */

#ifndef MATH_H
#define	MATH_H

#include <stdint.h>

/*!
 * @brief Implements the 5-order polynomial approximation to sin(x).
 * The result is accurate to within +- 1 count. ie: +/-2.44e-4.
 * @note taken from https://www.nullhardware.com/blog/fixed-point-sine-and-cosine-for-embedded-systems/
 * @param i angle (with 2^15 units/circle)
 * @return 16 bit fixed point Sine value (4.12) (ie: +4096 = +1 & -4096 = -1)
 *
 */
int16_t fpsin(int16_t i);

/* cos(x) = sin(x + pi/2) */
#define fpcos(i) fpsin((int16_t)(((uint16_t)(i)) + 8192U))

int8_t sin_lookup(uint16_t a);

#define cos_lookup(a) sin_lookup(a + 90);

uint16_t atan2(int8_t x, int8_t y);

#endif	/* MATH_H */
