/*!
 * @file param.h
 * @author TheComet
 */

#ifndef PARAM_H
#define	PARAM_H

#include <stdint.h>

struct config
{
    union {
        struct {
            unsigned normal_mode     : 2;  /* 00 = OFF, 01 = clamp, 10 = quantize */
            unsigned _padding        : 6;
            unsigned cardinal_angles : 8;
            unsigned diagonal_angles : 8;
            unsigned special_angles  : 8;
        };
        uint8_t bytes[4];
    } enable;

    struct {
        uint8_t xythreshold;
        uint8_t hysteresis;
    } joy;
    
    struct {
        uint8_t xl;
        uint8_t xh;
        uint8_t yl;
        uint8_t yh;
    } dac_clamp;

    struct {
        uint8_t x, y;
    } angles[24];
};

void param_load_from_nvm(void);
void param_set_defaults(void);
void param_save_to_nvm(void);

struct param* param_get(void);

#endif	/* PARAM_H */
