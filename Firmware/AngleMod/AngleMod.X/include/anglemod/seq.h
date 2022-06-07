/* !
 * @file cmd_seq.h
 * @author TheComet
 */

#ifndef CMD_SEQ_H
#define	CMD_SEQ_H

#include "anglemod/cli.h"

#if defined (CLI_USE_UNICODE)
#   define ARROW_W  "\xE2\x86\x90"
#   define ARROW_N  "\xE2\x86\x91"
#   define ARROW_E  "\xE2\x86\x92"
#   define ARROW_S  "\xE2\x86\x93"
#   define ARROW_NW "\xE2\x86\x96"
#   define ARROW_NE "\xE2\x86\x97"
#   define ARROW_SE "\xE2\x86\x98"
#   define ARROW_SW "\xE2\x86\x99"
#else
#   define ARROW_W  " W"
#   define ARROW_N  " N"
#   define ARROW_E  " E"
#   define ARROW_S  " S"
#   define ARROW_NW " NW"
#   define ARROW_NE " NE"
#   define ARROW_SE " SE"
#   define ARROW_SW " SW"
#endif

/* name, mirror X, mirror Y, string, initial X, intial Y */
#define SEQ_LIST                                                      \
    /* Cardinal angles */                                             \
    X(CARD_N_NE, CARD_N_NW, CARD_S_SE, ARROW_N ARROW_NE, 200, 231)    \
    X(CARD_E_NE, CARD_W_NW, CARD_E_SE, ARROW_E ARROW_NE, 231, 200)    \
    X(CARD_E_SE, CARD_W_SW, CARD_E_NE, ARROW_E ARROW_SE, 231, 55)     \
    X(CARD_S_SE, CARD_S_SW, CARD_N_NE, ARROW_S ARROW_SE, 200, 24)     \
    X(CARD_N_NW, CARD_N_NE, CARD_S_SW, ARROW_N ARROW_NW, 55,  231)    \
    X(CARD_W_NW, CARD_E_NE, CARD_W_SW, ARROW_W ARROW_NW, 24,  200)    \
    X(CARD_W_SW, CARD_E_SE, CARD_W_NW, ARROW_W ARROW_SW, 24,  55)     \
    X(CARD_S_SW, CARD_S_SE, CARD_N_NW, ARROW_S ARROW_SW, 55,  24)     \
    /* Diagonal angles */                                             \
    X(DIAG_NE_N, DIAG_NW_N, DIAG_SE_S, ARROW_NE ARROW_N, 149, 252)    \
    X(DIAG_NE_E, DIAG_NW_W, DIAG_SE_E, ARROW_NE ARROW_E, 252, 149)    \
    X(DIAG_SE_E, DIAG_SW_W, DIAG_NE_E, ARROW_SE ARROW_E, 252, 106)    \
    X(DIAG_SE_S, DIAG_SW_S, DIAG_NE_N, ARROW_SE ARROW_S, 149, 3)      \
    X(DIAG_NW_N, DIAG_NE_N, DIAG_SW_S, ARROW_NW ARROW_N, 106, 252)    \
    X(DIAG_NW_W, DIAG_NE_E, DIAG_SW_W, ARROW_NW ARROW_W, 3,   149)    \
    X(DIAG_SW_W, DIAG_SE_E, DIAG_NW_W, ARROW_SW ARROW_W, 3,   106)    \
    X(DIAG_SW_S, DIAG_SE_S, DIAG_NW_N, ARROW_SW ARROW_S, 106, 3)      \
    /* Special angles */                                              \
    X(SPEC_E_NE_N, SPEC_W_NW_N, SPEC_E_SE_S, ARROW_E ARROW_NE ARROW_N, 0, 0) \
    X(SPEC_E_SE_S, SPEC_W_SW_S, SPEC_E_NE_N, ARROW_E ARROW_SE ARROW_S, 0, 0) \
    X(SPEC_W_NW_N, SPEC_E_NE_N, SPEC_W_SW_S, ARROW_W ARROW_NW ARROW_N, 0, 0) \
    X(SPEC_W_SW_S, SPEC_E_SE_S, SPEC_W_NW_N, ARROW_W ARROW_SW ARROW_S, 0, 0) \
    X(SPEC_N_NE_E, SPEC_N_NW_W, SPEC_S_SE_E, ARROW_N ARROW_NE ARROW_E, 0, 0) \
    X(SPEC_S_SE_E, SPEC_S_SW_W, SPEC_N_NE_E, ARROW_S ARROW_SE ARROW_E, 0, 0) \
    X(SPEC_N_NW_W, SPEC_N_NE_E, SPEC_S_SW_W, ARROW_N ARROW_NW ARROW_W, 0, 0) \
    X(SPEC_S_SW_W, SPEC_S_SE_E, SPEC_N_NW_W, ARROW_S ARROW_SW ARROW_W, 0, 0)

enum seq
{
#define X(name, mirrx, mirry, str, x, y) SEQ_##name,
    SEQ_LIST
#undef X
    SEQ_COUNT,
    
    SEQ_NONE = 255,
};

/*!
 * Analyzes the queue of joystick states to find a valid command sequence. If
 * no valid command sequence was found, this will return SEQ_NONE.
 */
enum joy_state;
enum seq seq_find(const enum joy_state* state_history);


#endif	/* CMD_SEQ_H */
