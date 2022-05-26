#include "anglemod/cmd_seq.h"

static enum joy_state state_history[3];
static uint8_t xythreshold;
static uint8_t hysteresis;

static uint8_t thresh1_l = 80;
static uint8_t thresh1_h = 90;
static uint8_t thresh2_l = 165;
static uint8_t thresh2_h = 175;

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
    if (state_history[0] == state)
        return;
    
    state_history[2] = state_history[1];
    state_history[1] = state_history[0];
    state_history[0] = state;
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
    
    if (x < thresh1_l)
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
            default: break;
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
            default: break;
        }

        default : return CMD_NONE;
    }
}
