/* !
 * @file cmd_seq.h
 * @author TheComet
 */

#ifndef CMD_SEQ_H
#define	CMD_SEQ_H

#include <stdint.h>

#define ARROW_W  "\xF0\x9F\xA1\xA0"
#define ARROW_N  "\xF0\x9F\xA1\xA1"
#define ARROW_E  "\xF0\x9F\xA1\xA2"
#define ARROW_S  "\xF0\x9F\xA1\xA3"
#define ARROW_NW "\xF0\x9F\xA1\xA4"
#define ARROW_NE "\xF0\x9F\xA1\xA5"
#define ARROW_SE "\xF0\x9F\xA1\xA6"
#define ARROW_SW "\xF0\x9F\xA1\xA7"

#define JOY_STATE_LIST \
    X(NEUTRAL, "neutral") \
    X(N, "N")   \
    X(NE, "NE") \
    X(E, "E")   \
    X(SE, "SE") \
    X(S, "S")   \
    X(SW, "SW") \
    X(W, "W")   \
    X(NW, "NW")

#define SEQ_LIST                                \
    /* Cardinal angles */                       \
    X(CARD_N_NE, ARROW_N ARROW_NE, 215, 255)    \
    X(CARD_E_NE, ARROW_E ARROW_NE, 255, 215)    \
    X(CARD_E_SE, ARROW_E ARROW_SE, 255, 41)     \
    X(CARD_S_SE, ARROW_S ARROW_SE, 41,  0)      \
    X(CARD_N_NW, ARROW_N ARROW_NW, 215, 255)    \
    X(CARD_W_NW, ARROW_W ARROW_NW, 215, 255)    \
    X(CARD_W_SW, ARROW_W ARROW_SW, 215, 255)    \
    X(CARD_S_SW, ARROW_S ARROW_SW, 215, 255)    \
    /* Diagonal angles */                       \
    X(DIAG_NE_N, ARROW_NE ARROW_N, 215, 255)    \
    X(DIAG_NE_E, ARROW_NE ARROW_E, 215, 255)    \
    X(DIAG_SE_E, ARROW_SE ARROW_E, 215, 255)    \
    X(DIAG_SE_S, ARROW_SE ARROW_S, 215, 255)    \
    X(DIAG_NW_N, ARROW_NW ARROW_N, 215, 255)    \
    X(DIAG_NW_W, ARROW_NW ARROW_W, 215, 255)    \
    X(DIAG_SW_W, ARROW_SW ARROW_W, 215, 255)    \
    X(DIAG_SW_S, ARROW_SW ARROW_S, 215, 255)    \
    /* Special angles */                        \
    X(SPEC_E_NE_N, ARROW_E ARROW_NE ARROW_N, 0, 0) \
    X(SPEC_E_SE_S, ARROW_E ARROW_SE ARROW_S, 0, 0) \
    X(SPEC_W_NW_N, ARROW_W ARROW_NW ARROW_N, 0, 0) \
    X(SPEC_W_SW_S, ARROW_W ARROW_SW ARROW_S, 0, 0) \
    X(SPEC_N_NE_E, ARROW_N ARROW_NE ARROW_E, 0, 0) \
    X(SPEC_S_SE_E, ARROW_S ARROW_SE ARROW_E, 0, 0) \
    X(SPEC_N_NW_W, ARROW_N ARROW_NW ARROW_W, 0, 0) \
    X(SPEC_S_SW_W, ARROW_S ARROW_SW ARROW_W, 0, 0)

enum joy_state
{
#define X(name, str) JOY_##name,
    JOY_STATE_LIST
#undef X

    JOY_STATE_COUNT
};

enum cmd_seq
{
    SEQ_NONE,
#define X(name, str, x, y) SEQ_##name,
    SEQ_LIST
#undef X
    SEQ_COUNT
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
