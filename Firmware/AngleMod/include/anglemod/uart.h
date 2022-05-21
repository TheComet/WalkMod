/*!
 * @file uart.h
 * @author TheComet
 */

#ifndef UART_H
#define	UART_H

void uart_init(void);

void uart_printf(const char* fmt, ...);

void uart_tx_isr(void);
void uart_rx_isr(void);

#endif	/* UART_H */
