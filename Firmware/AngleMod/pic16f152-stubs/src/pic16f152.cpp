#include <xc.h>

volatile struct PIR0bits PIR0bits;
volatile struct PIR1bits PIR1bits;
volatile struct PIE1bits PIE1bits;
volatile struct PORTAbits PORTAbits;
volatile struct PORTBbits PORTBbits;
volatile struct PORTCbits PORTCbits;

volatile uint8_t IOCAF;

volatile uint8_t PORTA;
volatile uint8_t PORTB;
volatile uint8_t PORTC;

volatile uint8_t LATA;
volatile uint8_t LATB;
volatile uint8_t LATC;

volatile uint8_t TRISA;
volatile uint8_t TRISB;
volatile uint8_t TRISC;

volatile uint8_t ANSELA;
volatile uint8_t ANSELB;
volatile uint8_t ANSELC;

volatile uint8_t RC1PPS;
volatile uint8_t RC2PPS;
volatile uint8_t RC6PPS;

volatile uint8_t INTPPS;
volatile uint8_t SSP1DATPPS;
volatile uint8_t RX1PPS;

volatile struct T0CON0bits T0CON0bits;
volatile struct T0CON1bits T0CON1bits;

volatile uint8_t TMR0H;

volatile struct PIE0bits PIE0bits;
volatile struct INTCONbits INTCONbits;
volatile struct NVMCON1bits NVMCON1bits;

volatile uint16_t NVMADR; 
volatile uint8_t NVMADRL;
volatile uint8_t NVMADRH;
volatile uint8_t NVMCON1;
volatile uint8_t NVMCON2;
volatile uint8_t NVMDATL;

volatile uint8_t TX1REG;
volatile uint8_t RC1REG;

volatile struct SSP1STATbits SSP1STATbits;
volatile uint8_t SSP1BUF;
volatile uint8_t SSP1CON1;

volatile uint8_t ADRESH;
