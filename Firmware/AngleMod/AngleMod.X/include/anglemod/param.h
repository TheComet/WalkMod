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
        uint8_t thresh1_l;
        uint8_t thresh1_h;
        uint8_t thresh2_l;
        uint8_t thresh2_h;
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
