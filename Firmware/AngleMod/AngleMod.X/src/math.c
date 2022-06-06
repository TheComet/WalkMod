#include "anglemod/math.h"

/* ------------------------------------------------------------------------- */
static int8_t table[] = {
    0,   
    2,   4,   6,   8,   11,  13,  15,  17,  19,  22,  24,  26,  28,  30,  32,  /* 15 */
    35,  37,  39,  41,  43,  45,  47,  49,  51,  53,  55,  57,  59,  61,  63,  /* 30 */
    65 , 67,  69,  71,  72,  74,  76,  78,  79,  81,  83,  84,  86,  88,  89,  /* 45 */
    91,  92,  94,  95,  97,  98,  100, 101, 102, 104, 105, 106, 107, 108, 109, /* 60 */
    111, 112, 113, 114 ,115, 116, 116, 117, 118, 119, 120, 120, 121, 122, 122, /* 75 */
    123, 123, 124, 124, 125, 125, 125, 126, 126, 126, 126, 126, 126, 126, 127, /* 90 */
};
int8_t fpsin(uint16_t a)
{
    while (a > 360)
        a -= 360;
    
    if (a >= 270)
        return -table[360 - a];
    if (a >= 180)
        return -table[a - 180];
    if (a >= 90)
        return table[180 - a];
    return table[a];
}

/* ------------------------------------------------------------------------- */
uint16_t fpatan2(int8_t y, int8_t x)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Unit tests */
/* ------------------------------------------------------------------------- */

#if defined(GTEST_TESTING)

#define _USE_MATH_DEFINES

#include <gmock/gmock.h>
#include <cmath>

using namespace testing;

TEST(math, sin_typical_values)
{
    EXPECT_THAT(fpsin(0),   Eq(0));
    EXPECT_THAT(fpsin(30),  Eq(63));
    EXPECT_THAT(fpsin(45),  Eq(89));
    EXPECT_THAT(fpsin(60),  Eq(109));
    EXPECT_THAT(fpsin(90),  Eq(127));
    EXPECT_THAT(fpsin(120), Eq(109));
    EXPECT_THAT(fpsin(135), Eq(89));
    EXPECT_THAT(fpsin(150), Eq(63));
    EXPECT_THAT(fpsin(180), Eq(0));
    EXPECT_THAT(fpsin(210), Eq(-63));
    EXPECT_THAT(fpsin(225), Eq(-89));
    EXPECT_THAT(fpsin(240), Eq(-109));
    EXPECT_THAT(fpsin(270), Eq(-127));
    EXPECT_THAT(fpsin(300), Eq(-109));
    EXPECT_THAT(fpsin(315), Eq(-89));
    EXPECT_THAT(fpsin(330), Eq(-63));
    EXPECT_THAT(fpsin(360), Eq(0));
}

TEST(math, sin_all_values)
{
    for (int i = 0; i != 1000; ++i)
    {
        double expected_f = std::sin(i * M_PI / 180) * 127;
        int8_t expected_i = (int8_t)expected_f;
        EXPECT_THAT(fpsin(i), Eq(expected_i)) << "i=" << i;
    }
}

TEST(math, cos_typical_values)
{
    EXPECT_THAT(fpcos(0),   Eq(127));
    EXPECT_THAT(fpcos(30),  Eq(109));
    EXPECT_THAT(fpcos(45),  Eq(89));
    EXPECT_THAT(fpcos(60),  Eq(63));
    EXPECT_THAT(fpcos(90),  Eq(0));
    EXPECT_THAT(fpcos(120), Eq(-63));
    EXPECT_THAT(fpcos(135), Eq(-89));
    EXPECT_THAT(fpcos(150), Eq(-109));
    EXPECT_THAT(fpcos(180), Eq(-127));
    EXPECT_THAT(fpcos(210), Eq(-109));
    EXPECT_THAT(fpcos(225), Eq(-89));
    EXPECT_THAT(fpcos(240), Eq(-63));
    EXPECT_THAT(fpcos(270), Eq(0));
    EXPECT_THAT(fpcos(300), Eq(63));
    EXPECT_THAT(fpcos(315), Eq(89));
    EXPECT_THAT(fpcos(330), Eq(109));
    EXPECT_THAT(fpcos(360), Eq(127));
}

TEST(math, cos_all_values)
{
    for (int i = 0; i != 1000; ++i)
    {
        double expected_f = std::sin((i + 90) * M_PI / 180) * 127;
        int8_t expected_i = (int8_t)expected_f;
        EXPECT_THAT(fpcos(i), Eq(expected_i)) << "i=" << i;
    }
}

#endif
