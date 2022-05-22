#include "anglemod/cli.h"
#include "anglemod/uart.h"

static char line[CLI_LINE_BUF_LEN + 1];

static uint8_t line_len = 0;
static uint8_t line_idx = 0;

/* -------------------------------------------------------------------------- */
void cli_putc(char c)
{
    uint8_t i;
    
    if (c == '\n')
    {
        
    }

    if (line_idx >= CLI_LINE_BUF_LEN)
        return;

    for (i = line_idx; i < line_len - 1; ++i)
        line[i+1] = line[i];
    line[line_idx++] = c;
    line_len++;
}
