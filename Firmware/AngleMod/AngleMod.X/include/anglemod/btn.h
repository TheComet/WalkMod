/*!
 * @file btn.h
 * @author faglord
 */

#ifndef BTN_H
#define	BTN_H

#include "anglemod/gpio.h"
#include <stdint.h>

extern uint8_t _btn_state;

void btn_init(void);

/*!
 * @brief Gets the current state of the button on the port and stores it for
 * processing. This should be called once by the main thread after being woken 
 * up from sleep before using btn_pressed(), btn_released(), etc.
 * 
 * @note Only works if btn is not mapped to the 8th bit on the port. If this
 * is ever the case just shift everything right instead of left
 */
#define btn_poll() \
    _btn_state = (uint8_t)(_btn_state << 1u) | (PORTx(BTN_PORT) & BTN_BIT)
    
#define btn_pressed() \
    (!(_btn_state & BTN_BIT) && (_btn_state & (BTN_BIT << 1)))
    
#define btn_released() \
    ((_btn_state & BTN_BIT) && !(_btn_state & (BTN_BIT << 1)))

#define btn_is_active() \
    (!(_btn_state & BTN_BIT))

void btn_ioc_isr(void);

#endif	/* BTN_H */
