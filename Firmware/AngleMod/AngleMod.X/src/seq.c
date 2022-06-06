#include "anglemod/seq.h"
#include "anglemod/joy.h"
#include "anglemod/log.h"
#include "anglemod/config.h"

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
static enum seq find_sequence(const enum joy_state* state_history)
{
    uint8_t i, s = SEQ_COUNT;
    const struct config* c = config_get();
    while (s--)
    {
        /* Skip matching if the angles are disabled */
        uint8_t cat_idx = s >> 3;
        uint8_t item_idx = s & 0x07;
        uint8_t mask = (uint8_t)(1u << item_idx);
        if (!(c->enable.bytes[cat_idx] & mask))
            continue;

        for (i = 0; i != 3; ++i)
        {
            if (match_table[s][i] != JOY_NEUTRAL && match_table[s][i] != state_history[i])
                goto unmatched;
        }

        return (enum seq)s;
        
    unmatched:;
    }
    
    return SEQ_NONE;
}

/* -------------------------------------------------------------------------- */
enum seq seq_find(const enum joy_state* state_history)
{
    enum seq s = find_sequence(state_history);
    log_seq(s);
    return s;
}
