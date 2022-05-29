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

struct PIE1bits {
	uint8_t TX1IE;
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

extern volatile uint8_t TMR0H;

struct PIE0bits {
	uint8_t TMR0IE;
};
extern volatile struct PIE0bits PIE0bits;

struct INTCONbits {
	uint8_t GIE;
};
extern volatile struct INTCONbits INTCONbits;

struct NVMCON1bits {
	uint8_t NVMREGS;
	uint8_t FREE;
	uint8_t WREN;
	uint8_t LWLO;
	uint8_t WR;
	uint8_t RD;
};
extern volatile struct NVMCON1bits NVMCON1bits;

extern volatile uint16_t NVMADR;
extern volatile uint8_t NVMCON2;
extern volatile uint8_t NVMDATL;

extern volatile uint8_t TX1REG;
extern volatile uint8_t RC1REG;

extern volatile uint8_t ADRESH;

#endif
