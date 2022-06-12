#ifndef XC_H
#define XC_H

#include <stdint.h>

#define SLEEP()
#define NOP()
#define __interrupt()

struct PIR0bits {
	uint8_t INTF;
	uint8_t TMR0IF;
};
extern volatile struct PIR0bits PIR0bits;

struct PIR1bits {
	uint8_t ADIF;
	uint8_t RC1IF;
	uint8_t TX1IF;
};
extern volatile struct PIR1bits PIR1bits;

struct PIE0bits {
	unsigned TMR0IE : 1;
};
extern volatile struct PIE0bits PIE0bits;

struct PIE1bits {
	unsigned TX1IE : 1;
	unsigned RC1IE : 1;
	unsigned ADIE : 1;
};
extern volatile struct PIE1bits PIE1bits;

struct PORTAbits {
	unsigned RA0 : 1;
	unsigned RA1 : 1;
	unsigned RA2 : 1;
	unsigned RA3 : 1;
	unsigned RA4 : 1;
	unsigned RA5 : 1;
	unsigned RA6 : 1;
	unsigned RA7 : 1;
};
extern volatile struct PORTAbits PORTAbits;

struct PORTBbits {
	unsigned RB0 : 1;
	unsigned RB1 : 1;
	unsigned RB2 : 1;
	unsigned RB3 : 1;
	unsigned RB4 : 1;
	unsigned RB5 : 1;
	unsigned RB6 : 1;
	unsigned RB7 : 1;
};
extern volatile struct PORTBbits PORTBbits;

struct PORTCbits {
	unsigned RC0 : 1;
	unsigned RC1 : 1;
	unsigned RC2 : 1;
	unsigned RC3 : 1;
	unsigned RC4 : 1;
	unsigned RC5 : 1;
	unsigned RC6 : 1;
	unsigned RC7 : 1;
};
extern volatile struct PORTCbits PORTCbits;

extern volatile uint8_t IOCAF;

extern volatile uint8_t PORTA;
extern volatile uint8_t PORTB;
extern volatile uint8_t PORTC;

extern volatile uint8_t LATA;
extern volatile uint8_t LATB;
extern volatile uint8_t LATC;

extern volatile uint8_t TRISA;
extern volatile uint8_t TRISB;
extern volatile uint8_t TRISC;

extern volatile uint8_t ANSELA;
extern volatile uint8_t ANSELB;
extern volatile uint8_t ANSELC;

extern volatile uint8_t RC1PPS;
extern volatile uint8_t RC2PPS;
extern volatile uint8_t RC6PPS;

extern volatile uint8_t INTPPS;
extern volatile uint8_t SSP1DATPPS;
extern volatile uint8_t RX1PPS;

struct T0CON0bits {
	uint8_t EN;
};
extern volatile struct T0CON0bits T0CON0bits;

struct T0CON1bits {
	uint8_t T0CS;
	uint8_t ASYNC;
};
extern volatile struct T0CON1bits T0CON1bits;
extern volatile uint8_t T0CON1;

extern volatile uint8_t TMR0H;

struct INTCONbits {
	uint8_t GIE;
};
extern volatile struct INTCONbits INTCONbits;
extern volatile uint8_t INTCON;

struct NVMCON1bits {
	unsigned NVMREGS : 1;
	unsigned FREE : 1;
	unsigned WREN : 1;
	unsigned LWLO : 1;
	unsigned WR : 1;
	unsigned RD : 1;
	unsigned WRERR : 1;
};
extern volatile struct NVMCON1bits NVMCON1bits;

extern volatile uint16_t NVMADR;
extern volatile uint8_t NVMADRL;
extern volatile uint8_t NVMADRH;
extern volatile uint8_t NVMCON1;
extern volatile uint8_t NVMCON2;
extern volatile uint8_t NVMDATL;

extern volatile uint8_t SP1BRG;
extern volatile uint8_t TX1STA;
extern volatile uint8_t RC1STA;
extern volatile uint8_t TX1REG;
extern volatile uint8_t RC1REG;

struct SSP1STATbits {
	unsigned BF : 1;
};
extern volatile struct SSP1STATbits SSP1STATbits;
extern volatile uint8_t SSP1BUF;
extern volatile uint8_t SSP1CON1;

extern volatile uint8_t ADRESH;
extern volatile uint8_t ADCON0;
extern volatile uint8_t ADCON1;
extern volatile uint8_t ADACT;

#endif
