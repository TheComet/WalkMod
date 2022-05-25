#include "anglemod/cli.h"
#include "anglemod/uart.h"
#include "anglemod/param.h"
#include <ctype.h>  /* isprint() */

static char line[CLI_LINE_BUF_LEN + 1];

static uint8_t line_len = 0;
static uint8_t cursor_idx = 0;

static void execute_current_line(void)
{

}

static void set_cursor_horiz(uint8_t idx)
{
    uart_putc(0x1B);
    uart_putc('[');
    uart_putu(idx + 1);
    uart_putc('G');
}

/* -------------------------------------------------------------------------- */
void cli_putc(char c)
{
    /* Don't care about this */
    if (c == '\r')
        return;

    if (c == '\n')  /* Newline */
    {
        execute_current_line();
    }
    else if (c == 0x08 || c == 0x7F)  /* Backspace */
    {
        if (cursor_idx == 0)  /* Can't erase stuff at beginning */
            return;

        /* This memmove will also move the null terminator along */
        memmove(line + cursor_idx - 1, line + cursor_idx, line_len - cursor_idx + 1);
        line_len--;
        cursor_idx--;

        /*uart_prints("\x1b[K");*/
    }
    else if (isprint(c))  /* Printable character */
    {
        if (line_len >= CLI_LINE_BUF_LEN)
            return;

        for (uint8_t i = cursor_idx; i < line_len - 1; ++i)
            line[i + 1] = line[i];
        line[cursor_idx++] = c;
        line_len++;
    }

    set_cursor_horiz(0);
    uart_puts(line);
}
