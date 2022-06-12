/*!
 * @file main.c
 * @author TheComet
 */
#include "anglemod/gpio.h"

/*
 *                 ______________
 *                |              |
 *   ICSPDAT o----| RA0      RC0 |----o MISO
 *   ICSPCLK o----| RA1      RC1 |----o SCK
 *   nLAT    o----| RA2      RC2 |----o MOSI
 *   nMCLR   o----| RA3      RC3 |----o
 *   BTN     o----| RA4      RC4 |----o SWY
 *           o----| RA5      RC5 |----o SWX
 *                |          RC6 |----o TX
 *   nCS     o----| RB4      RC7 |----o RX
 *   JOYX    o----| RB5          |
 *   JOYY    o----| RB6          |
 *   GND     o----| RB7          |
 *                |______________|
 */

/* -------------------------------------------------------------------------- */
void gpio_init(void)
{
    /* Configure all GPIO pins */
    PORTA  = 0x04;  /* nLAT=1 by default */
    LATA   = 0x00;  /* Clear port latch bits */
    TRISA  = 0x18;  /* BTN and nMCLR are inputs (set to 1).
                     * nMCLR must be input. Rest are outputs. */

    PORTB  = 0x10;  /* nCS=1 by default */
    LATB   = 0x00;  /* Clear port latch bits */
    TRISB  = 0xE0;  /* JOYX, JOYY and RB7 are inputs. */

    PORTC  = 0x00;  /* Clear port bits */
    LATC   = 0x00;  /* Clear port latch bits */
    TRISC  = 0x81;  /* SWX and SWY are digital outputs, MISO and RX are digital 
                     * inputs */

    ANSELA = 0x00;  /* Set to digital IO (default is analog) */
    ANSELB = 0xE0;  /* JOYX, JOYY and RB7 are analog */
    ANSELC = 0x00;  /* All are digital */
    
    WPUC = 0x01;    /* MISO pin is high impedance and the signal exponentially 
                     * decays to 0V after each transmission over quite a long
                     * period of time. This prevents this behavior since the
                     * datasheet notes that any analog voltages on digital inputs
                     * could cause excess current to flow */

    /* Digital output re-mappings using PPS registers */
    RC1PPS = 0x07;  /* Maps SCK1 to RC1 */
    RC2PPS = 0x08;  /* Maps SDO1 to RC2 */
    RC6PPS = 0x05;  /* Maps TX1 to RC6 */

    /* Digital input re-mappings using PPS registers.
     * Note the format is PORT[5:3] PIN[2:0] */
    INTPPS     = 0x04;  /* Use external interrupt for BTN input. RA4 -> PORT[5:3] = 000, PIN[2:0] = 100 */
    SSP1DATPPS = 0x10;  /* MISO. RC0 -> PORT[5:3] = 010, PIN[2:0] = 000 */
    /*SSP1CLKPPS = 0x11;  * SCK. RC1 -> PORT[5:3] = 010, PIN[2:0] = 001 */
    SSP1SSPPS  = 0x05;  /* nSS. RA5 -> PORT[5:3] = 000, PIN[2:0] = 101 */
    RX1PPS     = 0x17;  /* RX. RC7 -> PORT[5:3] = 010, PIN[2:0] = 111 */
}
