#include "anglemod/cmd_seq.h"
#include "anglemod/param.h"

static enum joy_state state_history[3];

/* -------------------------------------------------------------------------- */
void cmd_seq_configure(uint8_t xythreshold, uint8_t hysteresis)
{
    struct param* p = param_get();
    
    uint8_t h2 = (uint8_t)(hysteresis / 2);
    p->cmd_seq.thresh1_l = (uint8_t)(128 - xythreshold - h2);
    p->cmd_seq.thresh1_h = (uint8_t)(128 - xythreshold + h2);
    p->cmd_seq.thresh2_l = (uint8_t)(128 + xythreshold - h2);
    p->cmd_seq.thresh2_h = (uint8_t)(128 + xythreshold + h2);
}

/* -------------------------------------------------------------------------- */
static void push_state(enum joy_state state)
{
    if (state_history[0] == state)
        return;
    
    state_history[2] = state_history[1];
    state_history[1] = state_history[0];
    state_history[0] = state;
}

/* -------------------------------------------------------------------------- */
void cmd_seq_push_joy_angle(uint8_t x, uint8_t y)
{
    const struct param* p = param_get();
    
    /* Divide 2D space into a 3x3 grid and detect which grid the joystick is in */
    if (x < p->cmd_seq.thresh1_l)
    {
        if (y < p->cmd_seq.thresh1_l) {
            push_state(JOY_SW); 
            return;
        }
        if (y < p->cmd_seq.thresh1_h)
            return;
        if (y < p->cmd_seq.thresh2_l) {
            push_state(JOY_W);
            return;
        }
        if (y < p->cmd_seq.thresh2_h)
            return;
        push_state(JOY_NW);
        return;
    }
    
    if (x < p->cmd_seq.thresh1_l)
        return;
    
    if (x < p->cmd_seq.thresh2_l)
    {
        if (y < p->cmd_seq.thresh1_l) {
            push_state(JOY_S);
            return;
        }
        if (y < p->cmd_seq.thresh1_h)
            return;
        if (y < p->cmd_seq.thresh2_l) {
            push_state(JOY_NEUTRAL);
            return;
        }
        if (y < p->cmd_seq.thresh2_h)
            return;
        push_state(JOY_N);
        return;
    }
    
    if (x < p->cmd_seq.thresh2_h)
        return;
    
    if (y < p->cmd_seq.thresh1_l) {
        push_state(JOY_SE);
        return;
    }
    if (y < p->cmd_seq.thresh1_h)
        return;
    if (y < p->cmd_seq.thresh2_l) {
        push_state(JOY_E);
        return;
    }
    if (y < p->cmd_seq.thresh2_h)
        return;
    push_state(JOY_NE);
}

/* -------------------------------------------------------------------------- */
enum cmd_seq cmd_seq_determine_command(void)
{
    switch (state_history[0])
    {
        case JOY_NE : switch (state_history[1])
        {
            case JOY_E : return CMD_SLIGHT_NEE;
            case JOY_N : return CMD_SLIGHT_NNE;
            default    : return CMD_NONE;
        }

        case JOY_SE : switch (state_history[1])
        {
            case JOY_E : return CMD_SLIGHT_SEE;
            case JOY_S : return CMD_SLIGHT_SSE;
            default    : return CMD_NONE;
        }

        case JOY_SW : switch (state_history[1])
        {
            case JOY_S : return CMD_SLIGHT_SSW;
            case JOY_W : return CMD_SLIGHT_SWW;
            default    : return CMD_NONE;
        }

        case JOY_NW : switch (state_history[1])
        {
            case JOY_W : return CMD_SLIGHT_NWW;
            case JOY_N : return CMD_SLIGHT_NNW;
            default    : return CMD_NONE;
        }

        case JOY_E : switch (state_history[1])
        {
            case JOY_NE : switch (state_history[2])
            {
                case JOY_N : return CMD_DOUBLE_UP_ZIP;
                default    : return CMD_NONE;
            }
        }

        case JOY_S : switch (state_history[1])
        {
            case JOY_SE : switch (state_history[2])
            {
                case JOY_E : return CMD_TSW_E;
                default    : return CMD_NONE;
            }
            case JOY_SW : switch (state_history[2])
            {
                case JOY_W : return CMD_TSW_W;
                default    : return CMD_NONE;
            }
        }

        default : return CMD_NONE;
    }
}
