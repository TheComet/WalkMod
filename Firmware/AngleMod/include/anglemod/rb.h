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
    uint8_t rb_##name##_put_single_value(T data);                             \
    uint8_t rb_##name##_put(const T* data, uint8_t len);                      \
    uint8_t rb_##name##_take(T* data, uint8_t maxlen);                        \
    uint8_t rb_##name##_take_single(T* data);                                 \

#define RB_DEFINE_API(name, T, N)                                             \
    static struct                                                             \
    {                                                                         \
        uint8_t read;                                                         \
        uint8_t write;                                                        \
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
                                                                              \
    /* -------------------------------------------------------------------- */\
    uint8_t rb_##name##_put_single_value(T data)                              \
    {                                                                         \
        uint8_t write;                                                        \
        if (RB_IS_FULL(rb_##name.read, rb_##name.write, N))                   \
            return 0;                                                         \
                                                                              \
        uint8_t write = rb_##name.write;                                      \
        rb_##name.buffer[write] = data;                                       \
        rb_##name.write = (write + 1u) & ((N)-1u);                            \
        return 1;                                                             \
    }                                                                         \
                                                                              \
                                                                              \
    /* -------------------------------------------------------------------- */\
    uint8_t rb_##name##_put(const T* data, uint8_t len)                       \
    {                                                                         \
        uint8_t write, cont_space;                                            \
        if (RB_IS_FULL(rb_##name.read, rb_##name.write, N))                   \
            return 0;                                                         \
                                                                              \
        write = rb_##name.write;                                              \
        RB_SPACE_TO_END(cont_space, rb_##name.read, rb_##name.write, N);      \
        cont_space = (uint8_t)(cont_space > len ? len : cont_space);          \
        memcpy(rb_##name.buffer + write, data, cont_space * sizeof(T));       \
        rb_##name.write = (uint8_t)(write + cont_space) & ((N)-1u);           \
                                                                              \
        return cont_space + (cont_space == 0u ? 0u :                          \
                rb_##name##_put(data + cont_space,                            \
                                (uint8_t)(len - cont_space)));                \
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
        (RB_SPACE(read, write, N) == 0)

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
