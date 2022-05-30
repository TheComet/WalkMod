#include "anglemod/cmd_seq.h"
#include "anglemod/log.h"
#include "anglemod/param.h"

static enum joy_state state_history[3];
static uint8_t xythreshold;
static uint8_t hysteresis;

static uint8_t thresh1_l = 0;
static uint8_t thresh1_h = 0;
static uint8_t thresh2_l = 0;
static uint8_t thresh2_h = 0;

static enum joy_state match_table[][3] = {
    /* Cardinal angles */
    {JOY_NEUTRAL, JOY_N, JOY_NE},
    {JOY_NEUTRAL, JOY_E, JOY_NE},
    {JOY_NEUTRAL, JOY_E, JOY_SE},
    {JOY_NEUTRAL, JOY_S, JOY_SE},
    {JOY_NEUTRAL, JOY_N, JOY_NW},
    {JOY_NEUTRAL, JOY_W, JOY_NW},
    {JOY_NEUTRAL, JOY_W, JOY_SW},
    {JOY_NEUTRAL, JOY_S, JOY_SW},
    /* Diagonal angles */
    {JOY_NEUTRAL, JOY_NE, JOY_N},
    {JOY_NEUTRAL, JOY_NE, JOY_E},
    {JOY_NEUTRAL, JOY_SE, JOY_E},
    {JOY_NEUTRAL, JOY_SE, JOY_S},
    {JOY_NEUTRAL, JOY_NW, JOY_N},
    {JOY_NEUTRAL, JOY_NW, JOY_W},
    {JOY_NEUTRAL, JOY_SW, JOY_W},
    {JOY_NEUTRAL, JOY_SW, JOY_S},
    /* Special angles*/
    {JOY_E, JOY_NE, JOY_N},
    {JOY_E, JOY_SE, JOY_S},
    {JOY_W, JOY_NW, JOY_N},
    {JOY_W, JOY_SW, JOY_S},
    {JOY_N, JOY_NE, JOY_E},
    {JOY_S, JOY_SE, JOY_E},
    {JOY_N, JOY_NW, JOY_W},
    {JOY_S, JOY_SW, JOY_W}
};

/* -------------------------------------------------------------------------- */
void cmd_seq_configure(uint8_t xythreshold, uint8_t hysteresis)
{
    uint8_t h2 = (uint8_t)(hysteresis / 2);
    thresh1_l = (uint8_t)(128 - xythreshold - h2);
    thresh1_h = (uint8_t)(128 - xythreshold + h2);
    thresh2_l = (uint8_t)(128 + xythreshold - h2);
    thresh2_h = (uint8_t)(128 + xythreshold + h2);
}

/* -------------------------------------------------------------------------- */
static void push_state(enum joy_state state)
{
    if (state_history[2] == state)
        return;
    
    state_history[0] = state_history[1];
    state_history[1] = state_history[2];
    state_history[2] = state;
    
    log_joy(state_history);
}

/* -------------------------------------------------------------------------- */
void cmd_seq_push_joy_angle(uint8_t x, uint8_t y)
{
    /* Divide 2D space into a 3x3 grid and detect which grid the joystick is in */
    if (x < thresh1_l)
    {
        if (y < thresh1_l) {
            push_state(JOY_SW); 
            return;
        }
        if (y < thresh1_h)
            return;
        if (y < thresh2_l) {
            push_state(JOY_W);
            return;
        }
        if (y < thresh2_h)
            return;
        push_state(JOY_NW);
        return;
    }
    
    if (x < thresh1_h)
        return;
    
    if (x < thresh2_l)
    {
        if (y < thresh1_l) {
            push_state(JOY_S);
            return;
        }
        if (y < thresh1_h)
            return;
        if (y < thresh2_l) {
            push_state(JOY_NEUTRAL);
            return;
        }
        if (y < thresh2_h)
            return;
        push_state(JOY_N);
        return;
    }
    
    if (x < thresh2_h)
        return;
    
    if (y < thresh1_l) {
        push_state(JOY_SE);
        return;
    }
    if (y < thresh1_h)
        return;
    if (y < thresh2_l) {
        push_state(JOY_E);
        return;
    }
    if (y < thresh2_h)
        return;
    push_state(JOY_NE);
}

/* -------------------------------------------------------------------------- */
static enum cmd_seq determine_command(void)
{
    /* Note that match_table is missing the SEQ_NONE entry at the beginning,
     * which means that the sequences in the table are off by one when compared
     * to the cmd_seq enum. 
     */
    uint8_t i, s = SEQ_COUNT - 1;
    const struct param* p = param_get();
    while (s--)
    {
        /* Skip matching if the angles are disabled */
        if (s < 8)
        {
            if (!p->enable.cardinal_angles)
                continue;
        }
        else if (s < 16)
        {
            if (!p->enable.diagonal_angles)
                continue;
        }
        else
        {
            uint8_t mask = (uint8_t)(1u << (s - 16u));
            if (!(p->enable.special_angles & mask))
                continue;
        }

        for (i = 0; i != 3; ++i)
        {
            if (match_table[s][i] && match_table[s][i] != state_history[i])
                goto unmatched;
        }

        return (enum cmd_seq)(s + 1);  /* Skip SEQ_NONE */
        
    unmatched:;
    }
    
    return SEQ_NONE;
}

/* -------------------------------------------------------------------------- */
enum cmd_seq cmd_seq_determine_command(void)
{
    enum cmd_seq seq = determine_command();
    log_seq(seq);
    return seq;
}
