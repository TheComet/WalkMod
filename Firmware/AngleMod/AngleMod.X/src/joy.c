#include "anglemod/joy.h"
#include "anglemod/log.h"
#include "anglemod/config.h"

static enum joy_state state_history[3] = {JOY_NEUTRAL, JOY_NEUTRAL, JOY_NEUTRAL};

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
void joy_push_state(const uint8_t xy[2])
{
    const struct config* c = config_get();
    
    uint8_t h2 = (uint8_t)(c->joy.hysteresis / 2);
    uint8_t thresh[5] = {
        (uint8_t)(128 - c->joy.xythreshold - h2 - 1),
        (uint8_t)(128 - c->joy.xythreshold + h2),
        (uint8_t)(128 + c->joy.xythreshold - h2 - 1),
        (uint8_t)(128 + c->joy.xythreshold + h2),
        255
    };
    
    for (uint8_t ix = 0; ix != 5; ++ix)
        for (uint8_t iy = 0; iy != 5; ++iy)
            if (xy[0] <= thresh[ix] && xy[1] <= thresh[iy])
            {
                /* Coordinates are located within the hysteresis region, ignore */
                if ((ix & 0x01) || (iy & 0x01))
                    return;
                
                enum joy_state state = (enum joy_state)((ix>>1)*3 + (iy>>1));
                push_state(state);
                return;
            }
}

/* -------------------------------------------------------------------------- */
const enum joy_state* joy_state_history(void)
{
    return state_history;
}

/* -------------------------------------------------------------------------- */
/* Unit Tests */
/* -------------------------------------------------------------------------- */

#if defined(GTEST_TESTING)

#include <gmock/gmock.h>

using namespace testing;

class joy : public Test
{
public:
    void SetUp() override 
    {
        struct config* c = config_get();
        c->joy.xythreshold = 42;
        c->joy.hysteresis = 30;

        for (int i = 0; i != 2; ++i)
            state_history[i] = JOY_NEUTRAL;
    }

    void TearDown() override
    {
        config_set_defaults();

        for (int i = 0; i != 2; ++i)
            state_history[i] = JOY_NEUTRAL;
    }

    enum joy_state state() const
    {
        return state_history[2];
    }

    void set_state(enum joy_state state) const
    {
        state_history[2] = state;
    }

    enum joy_state push(uint8_t x, uint8_t y) const
    {
        joy_push_state(x, y);
        return state();
    }
};

TEST_F(joy, neutral_boundaries)
{
    /* Neutral boundary extends from 102-154 inclusive */
    set_state(JOY_N);
    EXPECT_THAT(push(101, 128), Eq(JOY_N));
    set_state(JOY_N);
    EXPECT_THAT(push(102, 128), Eq(JOY_NEUTRAL));
    set_state(JOY_N);
    EXPECT_THAT(push(154, 128), Eq(JOY_NEUTRAL));
    set_state(JOY_N);
    EXPECT_THAT(push(155, 128), Eq(JOY_N));

    /* Neutral boundary extends from 102-154 inclusive */
    set_state(JOY_N);
    EXPECT_THAT(push(128, 101), Eq(JOY_N));
    set_state(JOY_N);
    EXPECT_THAT(push(128, 102), Eq(JOY_NEUTRAL));
    set_state(JOY_N);
    EXPECT_THAT(push(128, 154), Eq(JOY_NEUTRAL));
    set_state(JOY_N);
    EXPECT_THAT(push(128, 155), Eq(JOY_N));
}

TEST_F(joy, W_boundaries)
{
    /* Test upper and lower thresholds 102-154 inclusive */
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(35, 101), Eq(JOY_NEUTRAL));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(35, 102), Eq(JOY_W));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(35, 154), Eq(JOY_W));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(35, 155), Eq(JOY_NEUTRAL));

    /* Test left and right thresholds 0-70 inclusive */
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(0, 128), Eq(JOY_W));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(70, 128), Eq(JOY_W));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(71, 128), Eq(JOY_NEUTRAL));
}

TEST_F(joy, E_boundaries)
{
    /* Test upper and lower thresholds 102-154 inclusive */
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(220, 101), Eq(JOY_NEUTRAL));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(220, 102), Eq(JOY_E));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(220, 154), Eq(JOY_E));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(220, 155), Eq(JOY_NEUTRAL));

    /* Test left and right thresholds 186-255 inclusive */
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(255, 128), Eq(JOY_E));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(186, 128), Eq(JOY_E));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(185, 128), Eq(JOY_NEUTRAL));
}

TEST_F(joy, N_boundaries)
{
    /* Test upper and lower thresholds 102-154 inclusive */
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(101, 35), Eq(JOY_NEUTRAL));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(102, 35), Eq(JOY_N));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(154, 35), Eq(JOY_N));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(155, 35), Eq(JOY_NEUTRAL));

    /* Test left and right thresholds 0-70 inclusive */
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(128, 0), Eq(JOY_N));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(128, 70), Eq(JOY_N));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(128, 71), Eq(JOY_NEUTRAL));
}

TEST_F(joy, S_boundaries)
{
    /* Test upper and lower thresholds 102-154 inclusive */
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(101, 220), Eq(JOY_NEUTRAL));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(102, 220), Eq(JOY_S));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(154, 220), Eq(JOY_S));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(155, 220), Eq(JOY_NEUTRAL));

    /* Test left and right thresholds 186-255 inclusive */
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(128, 255), Eq(JOY_S));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(128, 186), Eq(JOY_S));
    set_state(JOY_NEUTRAL);
    EXPECT_THAT(push(128, 185), Eq(JOY_NEUTRAL));
}

#endif
