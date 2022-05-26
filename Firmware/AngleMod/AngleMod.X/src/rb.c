#include "anglemod/rb.h"

#if defined(GTEST_TESTING)

#include "gmock/gmock.h"
#define NAME ring_buffer

#define N 32
#define S uint8_t
RB_DEFINE_API(test_u8, uint8_t, N, S);

using namespace testing;

class NAME : public Test
{
public:
    void SetUp() override
    {
        // Reset global state
        memset(&rb_test_u8.buffer, 0, N);
        rb_test_u8_init();
    }
};

/* -------------------------------------------------------------------------- */
/* Ring Buffer tests */
/* -------------------------------------------------------------------------- */

TEST_F(NAME, space_macro)
{
    rb_test_u8.read = 0; rb_test_u8.write = 0; EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(N - 1));  // 1 slot is always "used" to detect when buffer is full
    rb_test_u8.read = 5; rb_test_u8.write = 8; EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(N - 4));
    rb_test_u8.read = 8; rb_test_u8.write = 5; EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(2));
    rb_test_u8.read = 16; rb_test_u8.write = 15; EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(0));

    rb_test_u8.read = 0; rb_test_u8.write = N - 1;
    EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(0));

    rb_test_u8.read = N - 1; rb_test_u8.write = 0;
    EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(N - 2));
}

TEST_F(NAME, space_to_end_macro)
{
    uint8_t result;
    rb_test_u8.read = 0; rb_test_u8.write = 0; RB_SPACE_TO_END(result, &rb_test_u8, N, S); EXPECT_THAT(result, Eq(N - 1));
    rb_test_u8.read = 5; rb_test_u8.write = 8; RB_SPACE_TO_END(result, &rb_test_u8, N, S); EXPECT_THAT(result, Eq(N - 8));
    rb_test_u8.read = 8; rb_test_u8.write = 5; RB_SPACE_TO_END(result, &rb_test_u8, N, S); EXPECT_THAT(result, Eq(2));
    rb_test_u8.read = 5; rb_test_u8.write = 5; RB_SPACE_TO_END(result, &rb_test_u8, N, S); EXPECT_THAT(result, Eq(N - 5));
    rb_test_u8.read = 6; rb_test_u8.write = 5; RB_SPACE_TO_END(result, &rb_test_u8, N, S); EXPECT_THAT(result, Eq(0));

    rb_test_u8.read = 0; rb_test_u8.write = N - 1;
    RB_SPACE_TO_END(result, &rb_test_u8, N, S);
    EXPECT_THAT(result, Eq(0));
}

TEST_F(NAME, is_full_and_is_empty_macros)
{
    rb_test_u8.read = 2; rb_test_u8.write = 2;
    EXPECT_THAT(RB_IS_FULL(&rb_test_u8, N), IsFalse());
    EXPECT_THAT(RB_IS_EMPTY(&rb_test_u8, N), IsTrue());

    rb_test_u8.read = 2; rb_test_u8.write = 3;
    EXPECT_THAT(RB_IS_FULL(&rb_test_u8, N), IsFalse());
    EXPECT_THAT(RB_IS_EMPTY(&rb_test_u8, N), IsFalse());

    rb_test_u8.read = 3; rb_test_u8.write = 2;
    EXPECT_THAT(RB_IS_FULL(&rb_test_u8, N), IsTrue());
    EXPECT_THAT(RB_IS_EMPTY(&rb_test_u8, N), IsFalse());

    rb_test_u8.read = 0; rb_test_u8.write = N - 1;
    EXPECT_THAT(RB_IS_FULL(&rb_test_u8, N), IsTrue());
    EXPECT_THAT(RB_IS_EMPTY(&rb_test_u8, N), IsFalse());

    rb_test_u8.read = N - 1; rb_test_u8.write = 0;
    EXPECT_THAT(RB_IS_FULL(&rb_test_u8, N), IsFalse());
    EXPECT_THAT(RB_IS_EMPTY(&rb_test_u8, N), IsFalse());
}

TEST_F(NAME, write_some_bytes_single)
{
    EXPECT_THAT(RB_COUNT(&rb_test_u8, N), Eq(0));

    EXPECT_THAT(rb_test_u8_put_single_value(0xA), IsTrue());
    EXPECT_THAT(rb_test_u8_put_single_value(0xB), IsTrue());
    EXPECT_THAT(rb_test_u8_put_single_value(0xC), IsTrue());

    EXPECT_THAT(rb_test_u8.read, Eq(0));
    EXPECT_THAT(RB_COUNT(&rb_test_u8, N), Eq(3));
    EXPECT_THAT(rb_test_u8.write, Eq(3));

    EXPECT_THAT(rb_test_u8.buffer[0], Eq(0xA));
    EXPECT_THAT(rb_test_u8.buffer[1], Eq(0xB));
    EXPECT_THAT(rb_test_u8.buffer[2], Eq(0xC));
}

TEST_F(NAME, read_some_bytes_single)
{
    uint8_t out_buf[4] = { 0 };
    EXPECT_THAT(rb_test_u8_put_single_value(0xA), IsTrue());
    EXPECT_THAT(rb_test_u8_put_single_value(0xB), IsTrue());
    EXPECT_THAT(rb_test_u8_put_single_value(0xC), IsTrue());
    ASSERT_THAT(rb_test_u8_take_single(out_buf+0), IsTrue());
    ASSERT_THAT(rb_test_u8_take_single(out_buf+1), IsTrue());
    ASSERT_THAT(rb_test_u8_take_single(out_buf+2), IsTrue());

    EXPECT_THAT(RB_COUNT(&rb_test_u8, N), Eq(0));
    EXPECT_THAT(rb_test_u8.read, Eq(3));
    EXPECT_THAT(rb_test_u8.write, Eq(3));

    EXPECT_THAT(out_buf[0], Eq(0xA));
    EXPECT_THAT(out_buf[1], Eq(0xB));
    EXPECT_THAT(out_buf[2], Eq(0xC));
    EXPECT_THAT(out_buf[3], Eq(0x0));
}

TEST_F(NAME, read_from_empty_rb_single)
{
    uint8_t out_buf;
    EXPECT_THAT(rb_test_u8_put_single_value(0xA), IsTrue());
    EXPECT_THAT(rb_test_u8_take_single(&out_buf), IsTrue());
    EXPECT_THAT(out_buf, Eq(0xA));
    EXPECT_THAT(rb_test_u8_take_single(&out_buf), IsFalse());
    EXPECT_THAT(rb_test_u8_take_single(&out_buf), IsFalse());
    EXPECT_THAT(rb_test_u8_take_single(&out_buf), IsFalse());
    EXPECT_THAT(out_buf, Eq(0xA));
}

TEST_F(NAME, write_and_read_bytes_with_wrap_around_single)
{
    // We want to write an odd number of bytes to the ring buffer,
    // so all possible ways in which the pointers wrap are tested.
    // For this to work, the buffer size must be an even number.
    ASSERT_THAT(N % 2, Eq(0));

    for (int i = 0; i != N * 64; ++i)
    {
        uint8_t out_buf[3] = { 0 };
        // write 3 bytes
        EXPECT_THAT(rb_test_u8_put_single_value(0xA), IsTrue());
        EXPECT_THAT(rb_test_u8_put_single_value(0xB), IsTrue());
        EXPECT_THAT(rb_test_u8_put_single_value(0xC), IsTrue());
        // read 3 bytes
        ASSERT_THAT(rb_test_u8_take_single(out_buf+0), IsTrue());
        ASSERT_THAT(rb_test_u8_take_single(out_buf+1), IsTrue());
        ASSERT_THAT(rb_test_u8_take_single(out_buf+2), IsTrue());

        EXPECT_THAT(RB_COUNT(&rb_test_u8, N), Eq(0));
        EXPECT_THAT(rb_test_u8.write, Eq(rb_test_u8.read));
        EXPECT_THAT(rb_test_u8.write, Lt(N));
        EXPECT_THAT(rb_test_u8.read, Lt(N));
    }
}

TEST_F(NAME, if_it_dont_fit_dont_shit_single)
{
    uint8_t out_buf;

    // Fill up the buffer completely
    int bytes_left = N - 1;
    while (bytes_left--)
    {
        EXPECT_THAT(rb_test_u8_put_single_value(bytes_left), IsTrue());
    }

    EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(0));
    EXPECT_THAT(RB_IS_FULL(&rb_test_u8, N), IsTrue());
    EXPECT_THAT(rb_test_u8.read, Eq(0));
    EXPECT_THAT(rb_test_u8.write, Eq(N - 1));

    // Writing doesn't work
    EXPECT_THAT(rb_test_u8_put_single_value(0xA), IsFalse());
    EXPECT_THAT(rb_test_u8_put_single_value(0xB), IsFalse());
    EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(0));
    EXPECT_THAT(rb_test_u8.read, Eq(0));
    EXPECT_THAT(rb_test_u8.write, Eq(N - 1));

    // Read everything back
    bytes_left = N - 1;
    while (bytes_left--)
    {
        EXPECT_THAT(rb_test_u8_take_single(&out_buf), IsTrue());
        EXPECT_THAT(out_buf, Eq(bytes_left));
    }

    EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(N - 1));
    EXPECT_THAT(RB_IS_EMPTY(&rb_test_u8, N), IsTrue());
    EXPECT_THAT(rb_test_u8.read, Eq(N - 1));
    EXPECT_THAT(rb_test_u8.write, Eq(N - 1));
}

TEST_F(NAME, write_some_bytes)
{
    EXPECT_THAT(RB_COUNT(&rb_test_u8, N), Eq(0));
    uint8_t in_buf[3] = {0xA, 0xB, 0xC};

    EXPECT_THAT(rb_test_u8_put(in_buf, 3), Eq(3));

    EXPECT_THAT(rb_test_u8.read, Eq(0));
    EXPECT_THAT(RB_COUNT(&rb_test_u8, N), Eq(3));
    EXPECT_THAT(rb_test_u8.write, Eq(3));

    EXPECT_THAT(rb_test_u8.buffer[0], Eq(0xA));
    EXPECT_THAT(rb_test_u8.buffer[1], Eq(0xB));
    EXPECT_THAT(rb_test_u8.buffer[2], Eq(0xC));
}

TEST_F(NAME, read_some_bytes)
{
    uint8_t in_buf[3] = {0xA, 0xB, 0xC};
    uint8_t out_buf[4] = { 0 };
    EXPECT_THAT(rb_test_u8_put(in_buf, 3), Eq(3));
    ASSERT_THAT(rb_test_u8_take(out_buf, 3), Eq(3));

    EXPECT_THAT(RB_COUNT(&rb_test_u8, N), Eq(0));
    EXPECT_THAT(rb_test_u8.read, Eq(3));
    EXPECT_THAT(rb_test_u8.write, Eq(3));

    EXPECT_THAT(out_buf[0], Eq(0xA));
    EXPECT_THAT(out_buf[1], Eq(0xB));
    EXPECT_THAT(out_buf[2], Eq(0xC));
    EXPECT_THAT(out_buf[3], Eq(0x0));
}

TEST_F(NAME, read_from_empty_rb)
{
    uint8_t in_buf[3] = {0xA, 0xB, 0xC};
    uint8_t out_buf;
    EXPECT_THAT(rb_test_u8_put(in_buf, 1), Eq(1));
    EXPECT_THAT(rb_test_u8_take(&out_buf, 1), Eq(1));
    EXPECT_THAT(out_buf, Eq(0xA));
    EXPECT_THAT(rb_test_u8_take(&out_buf, 1), Eq(0));
    EXPECT_THAT(rb_test_u8_take(&out_buf, 1), Eq(0));
    EXPECT_THAT(rb_test_u8_take(&out_buf, 1), Eq(0));
    EXPECT_THAT(out_buf, Eq(0xA));
}

TEST_F(NAME, write_and_read_bytes_with_wrap_around)
{
    // We want to write an odd number of bytes to the ring buffer,
    // so all possible ways in which the pointers wrap are tested.
    // For this to work, the buffer size must be an even number.
    ASSERT_THAT(N % 2, Eq(0));

    for (int i = 0; i != N * 64; ++i)
    {
        uint8_t in_buf[3] = {0xA, 0xB, 0xC};
        uint8_t out_buf[3] = { 0 };
        // write 3 bytes
        EXPECT_THAT(rb_test_u8_put(in_buf, 3), Eq(3));
        // read 3 bytes
        ASSERT_THAT(rb_test_u8_take(out_buf, 3), Eq(3));

        EXPECT_THAT(RB_COUNT(&rb_test_u8, N), Eq(0));
        EXPECT_THAT(rb_test_u8.write, Eq(rb_test_u8.read));
        EXPECT_THAT(rb_test_u8.write, Lt(N));
        EXPECT_THAT(rb_test_u8.read, Lt(N));
    }
}

TEST_F(NAME, if_it_dont_fit_dont_shit)
{
    uint8_t out_buf[N];

    // Fill up the buffer completely
    uint8_t in_buf[N];
    for (int i = 0; i != N; ++i)
        in_buf[i] = i;
    EXPECT_THAT(rb_test_u8_put(in_buf, N), Eq(N - 1));

    EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(0));
    EXPECT_THAT(RB_IS_FULL(&rb_test_u8, N), IsTrue());
    EXPECT_THAT(rb_test_u8.read, Eq(0));
    EXPECT_THAT(rb_test_u8.write, Eq(N - 1));

    // Writing doesn't work
    EXPECT_THAT(rb_test_u8_put(in_buf, 5), Eq(0));
    EXPECT_THAT(rb_test_u8_put(in_buf, 5), Eq(0));
    EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(0));
    EXPECT_THAT(rb_test_u8.read, Eq(0));
    EXPECT_THAT(rb_test_u8.write, Eq(N - 1));

    // Read everything back
    EXPECT_THAT(rb_test_u8_take(out_buf, N), Eq(N - 1));
    for (int i = 0; i != N - 1; ++i)
        EXPECT_THAT(out_buf[i], Eq(i));

    EXPECT_THAT(RB_SPACE(&rb_test_u8, N), Eq(N - 1));
    EXPECT_THAT(RB_IS_EMPTY(&rb_test_u8, N), IsTrue());
    EXPECT_THAT(rb_test_u8.read, Eq(N - 1));
    EXPECT_THAT(rb_test_u8.write, Eq(N - 1));
}

TEST_F(NAME, write_and_read_more_than_available_with_wrap_around)
{
    uint8_t in_buf[N * 2];
    uint8_t out_buf[N * 2] = { 0 };
    for (int i = 0; i != N * 2; ++i)
        in_buf[i] = i;

    rb_test_u8.read = N / 2;
    rb_test_u8.write = N / 2;
    EXPECT_THAT(rb_test_u8_put(in_buf, N * 2), Eq(N - 1));
    EXPECT_THAT(rb_test_u8_take(out_buf, N * 2), Eq(N - 1));

    for (int i = 0; i != N - 1; ++i)
        ASSERT_THAT(out_buf[i], Eq(i));
    for (int i = N; i != N * 2; ++i)
        ASSERT_THAT(out_buf[i], Eq(0));
}

TEST_F(NAME, write_and_read_more_less_available_with_wrap_around)
{
    uint8_t in_buf[N];
    uint8_t out_buf[N] = { 0 };
    for (int i = 0; i != N; ++i)
        in_buf[i] = i;

    rb_test_u8.read = N / 3 * 2;
    rb_test_u8.write = N / 3 * 2;
    EXPECT_THAT(rb_test_u8_put(in_buf, N / 2), Eq(N / 2));
    EXPECT_THAT(rb_test_u8_take(out_buf, N - 1), Eq(N / 2));

    for (int i = 0; i != N / 2; ++i)
        ASSERT_THAT(out_buf[i], Eq(i));
    for (int i = N / 2 + 1; i != N; ++i)
        ASSERT_THAT(out_buf[i], Eq(0));
}

#endif
