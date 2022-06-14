/*!
 * @file gpio.h
 * @author TheComet
 */

#ifndef GPIO_H
#define	GPIO_H

#include <xc.h>

void gpio_init(void);

#define PASTE2(x, y) x##y
#define PASTE3(x, y, z) x##y##z

#define PORTx(x) PASTE2(PORT, x)
#define IOCxP(x) PASTE3(IOC, x, P)
#define IOCxN(x) PASTE3(IOC, x, N)
#define IOCxF(x) PASTE3(IOC, x, F)

#define SW_PORT C
#define SWX_BIT 0x20
#define SWY_BIT 0x10

#define BTN_PORT A
#define BTN_BIT 0x10

#define gpio_latch_dac() PORTAbits.RA2 = 0
#define gpio_unlatch_dac() PORTAbits.RA2 = 1
#define gpio_select_dac() PORTBbits.RB4 = 0
#define gpio_deselect_dac() PORTBbits.RB4 = 1
#define gpio_set_swxy() PORTC |= 0x30
#define gpio_clear_swxy() PORTC &= ~0x30

#endif	/* GPIO_H */
