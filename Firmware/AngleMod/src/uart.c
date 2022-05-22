#include "anglemod/uart.h"
#include "anglemod/cli.h"
#include <xc.h>
#include <stdarg.h>

#define TX_BUF_SIZE 16u
#define RX_BUF_SIZE 16u
RB_DEFINE_API(tx, char, TX_BUF_SIZE)
RB_DEFINE_API(rx, char, RX_BUF_SIZE)

/* -------------------------------------------------------------------------- */
void uart_init(void)
{
    
}

/* -------------------------------------------------------------------------- */
static void write_block(const char* start, uint8_t len)
{
    while (len > 0)
    {
        uint8_t written = rb_tx_put(start, len);
        PIE1bits.TX1IE = 1;
        len -= written;
    }
}

/* -------------------------------------------------------------------------- */
void uart_putc(char c)
{
    while (rb_tx_put_single_value(c) == 0)
    {}
    
    PIE1bits.TX1IE = 1;
}

/* -------------------------------------------------------------------------- */
void uart_puts(const char* s)
{
    write_block(s, strlen(s));
}

/* -------------------------------------------------------------------------- */
void uart_printf(const char* fmt, ...)
{
    const char* end;
    va_list ap;
    va_start(ap, fmt);
    
    for (end = fmt; *end; ++end)
    {
        if (*end == '%')
        {
            write_block(fmt, (uint8_t)(end - fmt));
            ++end;
            switch (*end)
            {
                case 's': {
                    const char* s = va_arg(ap, const char*);
                    write_block(s, strlen(s));
                } break;
                
                case 'b': {
                    char buf[3];
                    uint8_t d;
                    uint8_t value = va_arg(ap, uint8_t);
                    
                    /* Convert to decimal */
                    for (d = 0; value >= 100; ++d)
                        value -= 100;
                    buf[0] = '0' + d;
                    for (d = 0; value >= 10; ++d)
                        value -= 10;
                    buf[1] = '0' + d;
                    buf[2] = '0' + value;
                    
                    /* Write */
                    if (buf[0] != '0')
                        write_block(buf, 3);
                    else if (buf[1] != '0')
                        write_block(buf+1, 2);
                    else
                        write_block(buf+2, 1);
                } break;
                
                case '%': {
                    uart_putc('%');
                } break;
            }
            ++end;
            fmt = end;
        }
    }
    
    write_block(fmt, (uint8_t)(end - fmt));
    
    va_end(ap);
}

/* -------------------------------------------------------------------------- */
void uart_tx_isr(void)
{
    uint8_t c;
    
    if (rb_tx_take_single(&c))
        TX1REG = c;
    else
        PIE1bits.TX1IE = 0;
}

/* -------------------------------------------------------------------------- */
void uart_rx_isr(void)
{
    PIR1bits.RC1IF = 0;
    rb_rx_put_single_value(RC1REG);
}
