/*!
 * @file rb.h
 * @author TheComet
 * @brief Macros for defining a ring buffer.
 *
 * The ring buffer consists of a read and write index, and a chunk of memory:
 * ```c
 * struct rb_t {
 *     int read, write;
 *     T data[N];
 * }
 * ```
 *
 * The buffer is considered empty when rb->read == rb->write. It is considered
 * full when rb->write is one slot behind rb->read. This is necessary because
 * otherwise there would be no way to tell the difference between an empty and
 * a full ring buffer.
 */
#ifndef RB_H
#define	RB_H

#include <string.h>

#define RB_DECLARE_API(name, T, S)                                            \
    void rb_##name##_init(void);                                              \
    S rb_##name##_put_single(const T* data);                                  \
    S rb_##name##_put_single_value(T data);                                   \
    S rb_##name##_put(const T* data, S len);                                  \
    S rb_##name##_take(T* data, S maxlen);                                    \
    S rb_##name##_take_single(T* data);                                       \
    S rb_##name##_count(void);

#define RB_DEFINE_API(name, T, N, S)                                          \
    static struct                                                             \
    {                                                                         \
        volatile S read;                                                      \
        volatile S write;                                                     \
        T buffer[N];                                                          \
    } rb_##name;                                                              \
                                                                              \
    /* -------------------------------------------------------------------- */\
    void rb_##name##_init(void)                                               \
    {                                                                         \
        rb_##name.read = 0;                                                   \
        rb_##name.write = 0;                                                  \
    }                                                                         \
                                                                              \
    /* -------------------------------------------------------------------- */\
    S rb_##name##_put_single_value(T data)                                    \
    {                                                                         \
        if (RB_IS_FULL(&rb_##name, N))                                        \
            return 0;                                                         \
                                                                              \
        S write = rb_##name.write;                                            \
        rb_##name.buffer[write] = data;                                       \
        rb_##name.write = (write + 1u) & ((N)-1u);                            \
        return 1;                                                             \
    }                                                                         \
                                                                              \
    /* -------------------------------------------------------------------- */\
    S rb_##name##_put_single(const T* data)                                   \
    {                                                                         \
        return rb_##name##_put_single_value(*data);                           \
    }                                                                         \
                                                                              \
    /* -------------------------------------------------------------------- */\
    static S rb_##name##_put_contiguous(const T* data, S len)                 \
    {                                                                         \
        S count;                                                              \
        RB_SPACE_TO_END(count, &rb_##name, N, S);                             \
        if (count > len)                                                      \
            count = len;                                                      \
        memcpy(rb_##name.buffer + rb_##name.write, data, count * sizeof(T));  \
        rb_##name.write = (S)(rb_##name.write + count) & ((N)-1u);            \
        return count;                                                         \
    }                                                                         \
    S rb_##name##_put(const T* data, S len)                                   \
    {                                                                         \
        S count = rb_##name##_put_contiguous(data, len);                      \
        S left = len - count;                                                 \
        if (left)                                                             \
            count += rb_##name##_put_contiguous(data + count, left);          \
        return count;                                                         \
    }                                                                         \
                                                                              \
    /* -------------------------------------------------------------------- */\
    static S rb_##name##_take_contiguous(T* data, S maxlen)                   \
    {                                                                         \
        S count;                                                              \
        RB_COUNT_TO_END(count, &rb_##name, N, S);                             \
        if (count > maxlen)                                                   \
            count = maxlen;                                                   \
        memcpy(data, rb_##name.buffer + rb_##name.read, count * sizeof(T));   \
        rb_##name.read = (S)(rb_##name.read + count) & ((N)-1u);              \
        return count;                                                         \
    }                                                                         \
    S rb_##name##_take(T* data, S maxlen)                                     \
    {                                                                         \
        S count = rb_##name##_take_contiguous(data, maxlen);                  \
        S left = maxlen - count;                                              \
        if (left)                                                             \
            count += rb_##name##_take_contiguous(data + count, left);         \
        return count;                                                         \
    }                                                                         \
                                                                              \
    /* -------------------------------------------------------------------- */\
    S rb_##name##_take_single(T* data)                                        \
    {                                                                         \
        S read;                                                               \
        if (RB_IS_EMPTY(&rb_##name, N))                                       \
            return 0;                                                         \
                                                                              \
        read = rb_##name.read;                                                \
        *data = rb_##name.buffer[read];                                       \
        rb_##name.read = (read + 1u) & ((N)-1u);                              \
        return 1;                                                             \
    }                                                                         \
                                                                              \
    /* -------------------------------------------------------------------- */\
    S rb_##name##_count(void)                                                 \
    {                                                                         \
        return RB_COUNT(&rb_##name, N);                                       \
    }

/*
 * For the following 6 macros it is very important that each argument is only
 * accessed once.
 */

#define RB_COUNT(rb, N) \
        (((rb)->write - (rb)->read) & ((N)-1u))

#define RB_SPACE(rb, N) \
        (((rb)->read - (rb)->write - 1) & ((N)-1u))

#define RB_IS_FULL(rb, N) \
        ((((rb)->write + 1u) & ((N)-1u)) == (rb)->read)

#define RB_IS_EMPTY(rb, N) \
        ((rb)->read == (rb)->write)

#define RB_COUNT_TO_END(result, rb, N, S) {                      \
            S end = (S)((N) - (rb)->read);                       \
            S n = (S)((rb)->write + end) & ((N)-1u);             \
            result = (S)(n < end ? n : end);                     \
        }

#define RB_SPACE_TO_END(result, rb, N, S) {                      \
            S end = (S)(((N)-1u) - (rb)->write);                 \
            S n = (S)(end + (rb)->read) & ((N)-1u);              \
            result = n <= end ? n : end + 1u;                    \
        }

#endif	/* RB_H */
