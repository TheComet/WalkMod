/*!
 * @file btn.h
 * @author faglord
 */

#ifndef BTN_H
#define	BTN_H

#include <stdint.h>

void btn_init(void);

uint8_t btn_pressed_get_and_clear(void);
uint8_t btn_released_get_and_clear(void);
uint8_t btn_is_active(void);

void btn_int_isr(void);

#endif	/* BTN_H */
