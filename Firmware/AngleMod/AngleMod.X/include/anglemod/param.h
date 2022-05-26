/*!
 * @file param.h
 * @author TheComet
 */

#ifndef PARAM_H
#define	PARAM_H

#include <stdint.h>

struct param
{
    struct {
        unsigned normal_mode : 2;  /* 00 = OFF, 01 = clamp, 10 = quantize */
        unsigned angle_modifiers : 1;
        unsigned command_inputs : 1;
    } enable;

    struct {
        uint8_t xythreshold;
        uint8_t hysteresis;
    } cmd_seq;
    
    struct {
        uint8_t xl;
        uint8_t xh;
        uint8_t yl;
        uint8_t yh;
    } dac_clamp;
};

void param_init(void);
struct param* param_get(void);
void param_save_to_nvm(void);

#endif	/* PARAM_H */
