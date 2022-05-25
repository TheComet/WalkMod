/*!
 * @file uart.h
 * @author TheComet
 */

#ifndef UART_H
#define	UART_H

#include "anglemod/rb.h"
#include <string.h>

#define TX_BUF_SIZE 16u
#define RX_BUF_SIZE 16u

void uart_init(void);

void uart_putbytes(const char* start, uint8_t len);

void uart_putc(char c);
void uart_putu(uint8_t value);
void uart_printf(const char* fmt, ...);
#define uart_puts(s) uart_putbytes(s, strlen(s))

void uart_tx_isr(void);
void uart_rx_isr(void);

RB_DECLARE_API(rx, char);
RB_DECLARE_API(tx, char);

#endif	/* UART_H */
