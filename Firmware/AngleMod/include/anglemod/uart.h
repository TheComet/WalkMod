/*!
 * @file uart.h
 * @author TheComet
 */

#ifndef UART_H
#define	UART_H

#include "anglemod/rb.h"

void uart_init(void);

void uart_putc(char c);
void uart_puts(const char* s);
void uart_printf(const char* fmt, ...);

void uart_tx_isr(void);
void uart_rx_isr(void);

RB_DECLARE_API(rx, char);

#endif	/* UART_H */
