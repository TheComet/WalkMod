#include "anglemod/uart.h"
#include "anglemod/cli.h"
#include <xc.h>
#include <stdarg.h>

/* -------------------------------------------------------------------------- */
void uart_init(void)
{
    
}

/* -------------------------------------------------------------------------- */
void uart_printf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    va_end(ap);
}

/* -------------------------------------------------------------------------- */
void uart_tx_isr(void)
{
    PIR1bits.TX1IF = 0;
}

/* -------------------------------------------------------------------------- */
void uart_rx_isr(void)
{
    PIR1bits.RC1IF = 0;
}
