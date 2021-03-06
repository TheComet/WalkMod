/*!
 * @file uart.h
 * @author TheComet
 */

#ifndef UART_H
#define	UART_H

#include "anglemod/rb.h"
#include <string.h>
#include <stdint.h>

#define TX_BUF_SIZE 64u
#define RX_BUF_SIZE 8u
#define TX_RWTYPE uint8_t
#define RX_RWTYPE uint8_t

/* 
 * When simulating, the write "thread" isn't actually a thread,
 * so we have to make sure the buffer is large enough to contain
 * the largest possible message, or otherwise the program
 * deadlocks
 */
#if defined(CLI_SIM)
#   undef TX_BUF_SIZE
#   define TX_BUF_SIZE (1024u * 8u)

#   undef TX_RWTYPE
#   define TX_RWTYPE uint16_t
#endif

void uart_init(void);

void uart_putc(char c);
void uart_printf(const char* fmt, ...);

void uart_tx_isr(void);
void uart_rx_isr(void);

RB_DECLARE_API(rx, char, RX_RWTYPE);
RB_DECLARE_API(tx, char, TX_RWTYPE);

#endif	/* UART_H */
