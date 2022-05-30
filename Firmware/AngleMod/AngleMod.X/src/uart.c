#include "anglemod/uart.h"
#include "anglemod/cli.h"
#include <xc.h>
#include <stdarg.h>

/* -------------------------------------------------------------------------- */
void uart_init(void)
{
    /* TODO */
}
/* -------------------------------------------------------------------------- */
void uart_putc(char c)
{
    /* Enabling the TX interrupt will cause the ISR to execute immediately,
     * so make sure to put data into the buffer before doing so. */
    while (rb_tx_put_single_value(c) == 0) {}
    PIE1bits.TX1IE = 1;
}

/* -------------------------------------------------------------------------- */
/*!
 * @brief Converts a u8 integer into a decimal string and sends it over uart
 */
static void uart_put_u8(uint8_t value)
{
    static char buf[3];
    uint8_t digits = 0x01;  /* This horrible shit saves 0.5% program space */

    /* Convert to decimal */
    buf[2] = '0';
    while (value >= 100) {
        value -= 100;
        buf[2]++;
        digits |= 0x02;
    }
    buf[1] = '0';
    while (value >= 10) {
        value -= 10;
        buf[1]++;
        if ((digits & 0x02) == 0)
            digits = 0x02;
    }
    buf[0] = '0' + (uint8_t)value;
    
    while (digits--)
        uart_putc(buf[digits]);
}

/* -------------------------------------------------------------------------- */
void uart_printf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    
    for (; *fmt; ++fmt)
    {
        if (*fmt == '%')
        {
            fmt++;
            switch (*fmt)
            {
                case 's': {
                    const char* s = va_arg(ap, const char*);
                    while (*s)
                        uart_putc(*s++);
                } break;
                
                case 'u': {
                    uint8_t value = va_arg(ap, uint8_t);
                    uart_put_u8(value);
                } break;
                
                case 'c': {
                    uart_putc(va_arg(ap, char));
                } break;
                
                case '%': {
                    uart_putc('%');
                } break;

                case '\0': {
                    va_end(ap);
                    return;
                }
            }
        }
        else
        {
            uart_putc(*fmt);
        }
    }
    
    va_end(ap);
}

/* -------------------------------------------------------------------------- */
void uart_tx_isr(void)
{
    char c;
    if (rb_tx_take_single(&c))
        TX1REG = c;  /* Write to transmit register */
    else
    {
        /* Last byte was queued, disable interrupt.
         * The TX1IF flag functions a bit differently than the other interrupt
         * flags, namely, it is read-only, and it is only ever clear when
         * the transmit queue is full. If we don't disable the interrupt,
         * then we will be stuck in an endless loop of serving this ISR for
         * no reason. */
        PIE1bits.TX1IE = 0;
    }
}

/* -------------------------------------------------------------------------- */
void uart_rx_isr(void)
{
    char c = RC1REG;  /* Read from receive register */
    rb_rx_put_single_value(c);
}

RB_DEFINE_API(rx, char, RX_BUF_SIZE, RX_RWTYPE)
RB_DEFINE_API(tx, char, TX_BUF_SIZE, TX_RWTYPE)
