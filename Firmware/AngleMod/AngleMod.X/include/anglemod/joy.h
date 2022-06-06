/*!
 * @file joy.h
 * @author TheComet
 */

#ifndef JOY_H
#define	JOY_H

#include <stdint.h>

/* Needs to be in this order, see joy.c:41 */
#define JOY_STATE_LIST \
    X(NW, "NW") \
    X(W, "W")   \
    X(SW, "SW") \
    X(N, "N")   \
    X(NEUTRAL, "neutral") \
    X(S, "S")   \
    X(NE, "NE") \
    X(E, "E")   \
    X(SE, "SE") \

enum joy_state
{
#define X(name, str) JOY_##name,
    JOY_STATE_LIST
#undef X

    JOY_STATE_COUNT
};

/*!
 * Converts the joystick angle into a joystick state and pushes it into the
 * queue of states, if it is different from the last.
 */
void joy_push_state(const uint8_t xy[2]);

const enum joy_state* joy_state_history(void);

#endif	/* JOY_H */
