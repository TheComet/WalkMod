/*!
 * @file gpio.h
 * @author TheComet
 */

#ifndef GPIO_H
#define	GPIO_H

#include <xc.h>

void gpio_init(void);

#define SW_PORT PORTC
#define SWX_BIT 0x20
#define SWY_BIT 0x10

#define gpio_latch_dac() PORTAbits.RA2 = 0
#define gpio_unlatch_dac() PORTAbits.RA2 = 1
#define gpio_select_dac() PORTBbits.RB4 = 0
#define gpio_deselect_dac() PORTBbits.RB4 = 1
#define gpio_set_swxy() PORTC |= 0x30
#define gpio_clear_swxy() PORTC &= ~0x30

#endif	/* GPIO_H */
