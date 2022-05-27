/* !
 * @file cmd_seq.h
 * @author TheComet
 */

#ifndef CMD_SEQ_H
#define	CMD_SEQ_H

#include <stdint.h>

enum joy_state
{
    JOY_NEUTRAL,
    JOY_N,
    JOY_NE,
    JOY_E,
    JOY_SE,
    JOY_S,
    JOY_SW,
    JOY_W,
    JOY_NW
};

enum cmd_seq
{
    CMD_NONE,
    CMD_SLIGHT_NNE,
    CMD_SLIGHT_NEE,
    CMD_SLIGHT_SEE,
    CMD_SLIGHT_SSE,
    CMD_SLIGHT_NNW,
    CMD_SLIGHT_NWW,
    CMD_SLIGHT_SWW,
    CMD_SLIGHT_SSW,
    CMD_DOUBLE_UP_ZIP,
    CMD_TSW_W,
    CMD_TSW_E,
};

/*!
 * @brief Sets the threshold and hysteresis parameters which determines how
 * joystick angles are converted into commands.
 * 
 * The joystick position is converted into one of 9 "states" depending on where
 * it is located on a 3x3 grid. The threshold parameter controls how far the
 * joystick needs to move before it is no longer neutral, and the hysteresis
 * parameter controls the gap to insert at the threshold position.
 * 
 * The maximum allowable value for threshold is 128 minus half of the hysteresis.
 * Default value is set such that the grid is exactly a third of the full range.
 * 
 *               threshold
 *                |<-->|
 *                |    |
 *          | |       | |
 *      NW  | |   N   | |  NE
 *    ______|_|_______|_|______ 
 *    ______|_|_______|_|______ < hystersis
 *          | |       | |        
 *      W   | | Neut. | |  E
 *    ______|_|_______|_|______
 *    ______|_|_______|_|______
 *          | |       | |
 *      SW  | |   S   | |  SE
 *          | |       | |
 */
void cmd_seq_configure(uint8_t xythreshold, uint8_t hysteresis);

/*!
 * Converts the joystick angle into a joystick state and pushes it into the
 * queue of states, if it is different from the last.
 */
void cmd_seq_push_joy_angle(uint8_t x, uint8_t y);

/*!
 * Analyzes the queue of joystick states to find a valid command sequence. If
 * no valid command sequence was found, this will return CMD_NONE.
 */
enum cmd_seq cmd_seq_determine_command(void);


#endif	/* CMD_SEQ_H */
