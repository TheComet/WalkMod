/*!
 * @file gpio.h
 * @author TheComet
 */

#ifndef GPIO_H
#define	GPIO_H

#include <xc.h>

void gpio_init(void);

#define gpio_set_swx() PORTCbits.RC5 = 1
#define gpio_clear_swx() PORTCbits.RC5 = 0
#define gpio_set_swy() PORTCbits.RC4 = 1
#define gpio_clear_swy() PORTCbits.RC4 = 0
#define gpio_set_swxy() PORTC |= 0x30
#define gpio_clear_swxy() PORTC &= ~0x30

#endif	/* GPIO_H */
