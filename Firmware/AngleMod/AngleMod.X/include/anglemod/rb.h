/*!
 * @file rb.h
 * @author Alex Murray
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
 * full when rb->write is one slot behind rb->write. This is necessary because
 * otherwise there would be no way to tell the difference between an empty and
 * a full ring buffer.
 */
#ifndef RB_H
#define	RB_H

#include <stdint.h>
#include <string.h>

#define RB_DECLARE_API(name, T)                                               \
    uint8_t rb_##name##_put_single(const T* data);                            \
    uint8_t rb_##name##_put(const T* data, uint8_t len);                      \
    uint8_t rb_##name##_take(T* data, uint8_t maxlen);                        \
    uint8_t rb_##name##_take_single(T* data);                                 \
    uint8_t rb_##name##_count(void);                                          \

#define RB_DEFINE_API(name, T, N)                                             \
    static struct                                                             \
    {                                                                         \
        volatile uint8_t read;                                                \
        volatile uint8_t write;                                               \
        T buffer[N];                                                          \
    } rb_##name;                                                              \
                                                                              \
    /* -------------------------------------------------------------------- */\
    uint8_t rb_##name##_put_single(const T* data)                             \
    {                                                                         \
        if (RB_IS_FULL(rb_##name.read, rb_##name.write, N))                   \
            return 0;                                                         \
                                                                              \
        uint8_t write = rb_##name.write;                                      \
        rb_##name.buffer[write] = *data;                                      \
        rb_##name.write = (write + 1u) & ((N)-1u);                            \
        return 1;                                                             \
    }                                                                         \
                                                                              \
    /* -------------------------------------------------------------------- */\
    uint8_t rb_##name##_put(const T* data, uint8_t len)                       \
    {                                                                         \
        uint8_t copy1;                                                        \
                                                                              \
        if (RB_IS_FULL(rb_##name.read, rb_##name.write, N))                   \
            return 0;                                                         \
                                                                              \
        RB_SPACE_TO_END(copy1, rb_##name.read, rb_##name.write, N);           \
        if (copy1 > len)                                                      \
            copy1 = len;                                                      \
        memcpy(rb_##name.buffer + rb_##name.write, data, copy1 * sizeof(T));  \
        len -= copy1;                                                         \
                                                                              \
        if (len == 0)                                                         \
        {                                                                     \
            rb_##name.write = (uint8_t)(rb_##name.write + copy1) & ((N)-1u);  \
            return copy1;                                                     \
        }                                                                     \
        else                                                                  \
        {                                                                     \
            uint8_t copy2;                                                    \
            RB_SPACE_TO_END(copy2, rb_##name.read, 0, N);                     \
            if (copy2 > len)                                                  \
                copy2 = len;                                                  \
            memcpy(rb_##name.buffer, data + copy1, copy2);                    \
            rb_##name.write = copy2;                                          \
            return (uint8_t)(copy1 + copy2);                                  \
        }                                                                     \
    }                                                                         \
                                                                              \
    /* -------------------------------------------------------------------- */\
    uint8_t rb_##name##_take(T* data, uint8_t maxlen)                         \
    {                                                                         \
        uint8_t read, cont_space;                                             \
        if (RB_IS_EMPTY(rb_##name.read, rb_##name.write, N))                  \
            return 0;                                                         \
                                                                              \
        read = rb_##name.read;                                                \
        RB_COUNT_TO_END(cont_space, rb_##name.read, rb_##name.write, N);      \
        cont_space = (uint8_t)(cont_space > maxlen ? maxlen : cont_space);    \
        memcpy(data, rb_##name.buffer + read, cont_space * sizeof(T));        \
        rb_##name.read = (uint8_t)(read + cont_space) & ((N)-1u);             \
                                                                              \
        return cont_space + (cont_space == 0u ? 0u :                          \
                rb_##name##_take(data + cont_space,                           \
                                 (uint8_t)(maxlen - cont_space)));            \
    }                                                                         \
                                                                              \
    /* -------------------------------------------------------------------- */\
    uint8_t rb_##name##_take_single(T* data)                                  \
    {                                                                         \
        uint8_t read;                                                         \
        if (RB_IS_EMPTY(rb_##name.read, rb_##name.write, N))                  \
            return 0;                                                         \
                                                                              \
        read = rb_##name.read;                                                \
        *data = rb_##name.buffer[read];                                       \
        rb_##name.read = (read + 1u) & ((N)-1u);                              \
        return 1;                                                             \
    }                                                                         \
                                                                              \
    /* -------------------------------------------------------------------- */\
    uint8_t rb_##name##_count(void)                                           \
    {                                                                         \
        return RB_COUNT(rb_##name.read, rb_##name.write, N);                  \
    }

/*
 * For the following 6 macros it is very important that each argument is only
 * accessed once.
 */

#define RB_COUNT(read, write, N) \
        (((write) - (read)) & ((N)-1u))

#define RB_SPACE(read, write, N) \
        (RB_COUNT((write)+1u, (read), N))

#define RB_IS_FULL(read, write, N) \
        (((write + 1u) & ((N)-1u)) == read)

#define RB_IS_EMPTY(read, write, N) \
        ((read) == (write))

#define RB_COUNT_TO_END(result, read, write, N) {            \
            uint8_t end = (uint8_t)((N) - (read));           \
            uint8_t n = (uint8_t)((write) + end) & ((N)-1u); \
            result = (uint8_t)(n < end ? n : end);           \
        }

#define RB_SPACE_TO_END(result, read, write, N) {           \
            uint8_t end = (uint8_t)(((N)-1u) - (write))   ; \
            uint8_t n = (uint8_t)(end + (read)) & ((N)-1u); \
            result = n <= end ? n : end + 1u;               \
        }

#endif	/* RB_H */
