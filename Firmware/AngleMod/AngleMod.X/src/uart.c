#include "anglemod/uart.h"
#include "anglemod/cli.h"
#include <xc.h>
#include <stdarg.h>

RB_DEFINE_API(rx, char, RX_BUF_SIZE, RX_RWTYPE)
RB_DEFINE_API(tx, char, TX_BUF_SIZE, TX_RWTYPE)

/* -------------------------------------------------------------------------- */
void uart_init(void)
{
    
}

/* -------------------------------------------------------------------------- */
void uart_putbytes(const char* start, uint8_t len)
{
    const char* end = start + len;
    
    /* Enabling the TX interrupt will cause the interrupt to execute immediately,
     * so make sure to put data into the buffer before doing so. */
    start += rb_tx_put(start, len);
    PIE1bits.TX1IE = 1;
    
    /* Assuming the bytes are transmitted much slower than we are able to put
     * bytes into the buffer, it's more efficient to insert them one by one
     * because we know that the buffer will be full */
    while (start != end)
        start += rb_tx_put_single(start);
}

/* -------------------------------------------------------------------------- */
void uart_putc(char c)
{
    while (rb_tx_put_single(&c) == 0)
    {}
    
    PIE1bits.TX1IE = 1;
}

/* -------------------------------------------------------------------------- */
void uart_putu(uint8_t value)
{
    char buf[4] = {'0', '0', '0'};

    /* Convert to decimal */
    while (value >= 100) {
        value -= 100;
        buf[0]++;
    }
    while (value >= 10) {
        value -= 10;
        buf[1]++;
    }
    buf[2] = '0' + (uint8_t)value;

    /* Write */
    if (buf[0] != '0')
        uart_putbytes(buf, 3);
    else if (buf[1] != '0')
        uart_putbytes(buf + 1, 2);
    else
        uart_putbytes(buf + 2, 1);
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
            uart_putbytes(fmt, (uint8_t)(end - fmt));
            ++end;
            switch (*end)
            {
                case 's': {
                    const char* s = va_arg(ap, const char*);
                    uart_puts(s);
                } break;
                
                case 'u': {
                    uint8_t value = va_arg(ap, uint8_t);
                    uart_putu(value);
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
            fmt = end + 1;
        }
    }
    
    {
        uint8_t len = (uint8_t)(end - fmt);
        if (len > 0)
            uart_putbytes(fmt, len);
    }
    
    va_end(ap);
}

/* -------------------------------------------------------------------------- */
void uart_tx_isr(void)
{
    char c;
    if (rb_tx_take_single(&c))
        TX1REG = c;
    else
        PIE1bits.TX1IE = 0;
}

/* -------------------------------------------------------------------------- */
void uart_rx_isr(void)
{
    char c = RC1REG;
    rb_rx_put_single(&c);
}
