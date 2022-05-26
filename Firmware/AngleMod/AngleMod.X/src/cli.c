#include "anglemod/cli.h"
#include "anglemod/uart.h"
#include "anglemod/param.h"
#include <ctype.h>  /* isprint() */

#define PROMPT "> "

static void cli_putc_normal(char c);
static void cli_putc_escape(char c);
static void cli_putc_csi(char c);
static void cli_putc_ignore_crlf(char c);

static char line[CLI_LINE_LEN + 1];
static char save_line[CLI_LINE_LEN + 1];
static char history[CLI_HISTORY_LEN][CLI_LINE_LEN + 1];

static uint8_t line_len = 0;
static uint8_t cursor_idx = 0;
static uint8_t history_write_idx = 0;
static uint8_t history_read_offset = 0;

#define CSI_PARAM_BUF_SIZE 1
static uint8_t csi_param_buf[CSI_PARAM_BUF_SIZE];
static uint8_t csi_param_idx = 0;

void (*cli_putc)(char c) = cli_putc_normal;

struct cli_cmd {
    const char* name;
    const char* help;
    void (*handler)(uint8_t argc, char** argv);
};

/* -------------------------------------------------------------------------- */
static void set_cursor_horiz(uint8_t idx)
{
    uart_printf("\x1B[%uG", idx + sizeof(PROMPT));
}

/* -------------------------------------------------------------------------- */
static void print_toggle_states()
{
    const struct param* p = param_get();

    uart_puts("\r\n(1) Normal Mode:      ");
    uart_printf("\x1B[1;3%cm%s\x1B[0m\r\n",
        p->enable.normal_mode == 0 ? '1' : '2',
        p->enable.normal_mode == 0 ? "Off" : p->enable.normal_mode == 1 ? "Clamp" : "Quantize");

    uart_puts("(2) Angle Modifiers:  ");
    uart_printf("\x1B[1;3%cm%s\x1B[0m\r\n",
        p->enable.angle_modifiers == 0 ? '1' : '2',
        p->enable.angle_modifiers == 0 ? "Off" : "On");

    uart_puts("(3) Command Inputs:   ");
    uart_printf("\x1B[1;3%cm%s\x1B[0m",
        p->enable.command_inputs == 0 ? '1' : '2',
        p->enable.command_inputs == 0 ? "Off" : "On");
}

/* -------------------------------------------------------------------------- */
static void cmd_toggle(uint8_t argc, char** argv)
{
    struct param* p = param_get();

    if (argc == 0)
    {
        print_toggle_states();
        return;
    }

    switch (*argv[0])
    {
    case '1':
        p->enable.normal_mode++;
        if (p->enable.normal_mode > 2)
            p->enable.normal_mode = 0;
        break;
    case '2':
        p->enable.angle_modifiers = ~p->enable.angle_modifiers;
        break;
    case '3':
        p->enable.command_inputs = ~p->enable.command_inputs;
        break;
    default:
        uart_puts("\r\n\x1b[1;31mError: \x1b[0mExpected a value from 1-3");
        return;
    }
    print_toggle_states();
}

/* -------------------------------------------------------------------------- */
static void cmd_thresh(uint8_t argc, char** argv)
{
    struct param* p = param_get();

    if (argc == 0)
    {
        uart_printf("\r\nThreshold: %u\r\nHysteresis: %u", p->cmd_seq.xythreshold, p->cmd_seq.hysteresis);
        return;
    }
}

/* -------------------------------------------------------------------------- */
static void cmd_save(uint8_t argc, char** argv)
{
    param_save_to_nvm();
    uart_puts("\r\n\x1b[1;32mSuccess: \x1b[0mValues written to NVM");
}

static struct cli_cmd commands[] = {
    {"toggle", "", cmd_toggle},
    {"thresh", "", cmd_thresh},
    {"save", "", cmd_save},
    {NULL}
};

/* -------------------------------------------------------------------------- */
static uint8_t split_args(char* s, char** argv, uint8_t maxsplit)
{
    char* start;
    uint8_t argc = 0;
    for (start = s; *s && argc < maxsplit; ++s)
    {
        if (*s == ' ')
        {
            argv[argc++] = start;
            start = s + 1;
            *s = '\0';
        }
    }
    if (argc < maxsplit)
        argv[argc++] = start;
    return argc;
}

/* -------------------------------------------------------------------------- */
static void execute_current_line(void)
{
    char* argv[3];
    uint8_t argc = split_args(line, argv, 3);
    for (const struct cli_cmd* cmd = commands; cmd->name; ++cmd)
    {
        if (strcmp(argv[0], cmd->name))
            continue;
        
        cmd->handler(argc - 1, argv + 1);
        return;
    }

    uart_puts("\r\n\x1b[1;31mError: \x1b[0mUnknown command");
}

/* -------------------------------------------------------------------------- */
static void clear_from_cursor_until_end(void)
{
    uart_puts("\x1b[K");
}

/* -------------------------------------------------------------------------- */
static void set_line_and_print(const char* new_text)
{
    strcpy(line, new_text);
    line_len = (uint8_t)strlen(line);
    cursor_idx = line_len;
    set_cursor_horiz(0);
    clear_from_cursor_until_end();
    uart_puts(line);
    set_cursor_horiz(cursor_idx);
}

/* -------------------------------------------------------------------------- */
static void cli_putc_escape(char c)
{
    switch (c)
    {
    case '[':  /* CSI (Control sequence introducer) */
        cli_putc = cli_putc_csi;
        break;
    default:  /* Unknown escape sequence, abort */
        cli_putc = cli_putc_normal;
        break;
    }
}

/* -------------------------------------------------------------------------- */
static void cli_putc_csi(char c)
{
    /* 
     * Wiki says parameter bytes could be followed by "intermediate bytes" in the
     * range 0x20-0x2F, but none of the sequences we care about have these, so we
     * ignore them.
     */
    if (c >= 0x30 && c <= 0x3F)
    {
        if (csi_param_idx > CSI_PARAM_BUF_SIZE)
            return;
        csi_param_buf[csi_param_idx++] = c;
    }
    else
    {
        switch (c)
        {
        case 'A':  /* Cursor up */
            if (history_read_offset == 0)
                strcpy(save_line, line);

            if (history_read_offset == CLI_HISTORY_LEN - 1)
                break;
            history_read_offset++;

            {
                uint8_t read_idx = history_write_idx - history_read_offset;
                if (read_idx > CLI_HISTORY_LEN - 1)
                    read_idx = CLI_HISTORY_LEN - 1;

                if (history[read_idx][0] == '\0')
                {
                    history_read_offset--;
                    break;
                }

                set_line_and_print(history[read_idx]);
            }

            break;

        case 'B':  /* Cursor down */
            switch (history_read_offset)
            {
            case 0: 
                break;

            case 1:
                history_read_offset = 0;
                set_line_and_print(save_line);
                break;

            default:
                history_read_offset--;
                {
                    uint8_t read_idx = history_write_idx - history_read_offset;
                    if (read_idx > CLI_HISTORY_LEN - 1)
                        read_idx = CLI_HISTORY_LEN - 1;
                    set_line_and_print(history[read_idx]);
                }

                break;
            }

        case 'C':  /* Cursor forward */
            /* Here we make the assumption that the CSI param consists of 1 character */
            if (csi_param_idx == 0)
                cursor_idx++;  /* default is 1 */
            else
                cursor_idx += csi_param_buf[0] - '0';

            if (cursor_idx >= line_len)
                cursor_idx = line_len;

            set_cursor_horiz(cursor_idx);
            break;

        case 'D':  /* Cursor back */
            /* Here we make the assumption that the CSI param consists of 1 character */
            if (csi_param_idx == 0 && cursor_idx > 0)
            {
                if (cursor_idx > 0)
                    cursor_idx--;  /* default is 1 */
            }
            else
            {
                uint8_t delta = csi_param_buf[0] - '0';
                if (delta > cursor_idx)
                    cursor_idx = 0;
                else
                    cursor_idx -= delta;
            }

            set_cursor_horiz(cursor_idx);
            break;

        case '~':  /* Input sequence */
            if (csi_param_idx == 0)
                break;
            switch (csi_param_buf[0]) {
            case '1':  /* Home key */
            case '7':  /* Home key */
                cursor_idx = 0;
                set_cursor_horiz(0);
                break;

            case '4':  /* End key */
            case '8':  /* End key */
                cursor_idx = line_len;
                set_cursor_horiz(cursor_idx);
                break;

            case '3':  /* Delete key */
                if (cursor_idx == line_len)  /* Cant erase stuff at end */
                    break;

                /* This memmove will also move the null terminator along */
                memmove(line + cursor_idx, line + cursor_idx + 1, line_len - cursor_idx);
                line_len--;
                history_read_offset = 0;

                clear_from_cursor_until_end();
                uart_puts(line + cursor_idx);
                set_cursor_horiz(cursor_idx);
                break;
            }
            break;

        default:   /* Invalid character, abort */
            break;
        }

        /* Regardless of what happens, we will return to normal mode */
        cli_putc = cli_putc_normal;
    }
}

/* -------------------------------------------------------------------------- */
static void cli_putc_ignore_crlf(char c)
{
    cli_putc = cli_putc_normal;

    if (c != '\r' && c != '\n')
        cli_putc_normal(c);
}

/* -------------------------------------------------------------------------- */
static void cli_putc_normal(char c)
{
    static uint8_t ignore_char = 0;

    switch (c)
    {
    case '\r':
    case '\n':
        if (line[0] != '\0' && c != ignore_char)
        {
            ignore_char = c == '\r' ? '\n' : '\r';

            strcpy(history[history_write_idx++], line);
            if (history_write_idx == CLI_HISTORY_LEN)
                history_write_idx = 0;
            history_read_offset = 0;

            execute_current_line();
        }
        else
        {
            ignore_char = 0;
        }
        /* Fall through */

    case 0x03:  /* CTRL+C (End of Text) */
        if (c == 0x03)
            uart_puts("^C");

        cursor_idx = 0;
        line_len = 0;
        line[0] = '\0';
        uart_puts("\r\n" PROMPT);
        set_cursor_horiz(0);

        break;

    case 0x1B: /* Escape */
        csi_param_idx = 0;  /* Init param buffer */
        cli_putc = cli_putc_escape;
        break;

    case 0x7F:  /* DEL (backspace) */
        if (cursor_idx == 0)  /* Can't erase stuff at beginning */
            break;

        /* This memmove will also move the null terminator along */
        memmove(line + cursor_idx - 1, line + cursor_idx, line_len - cursor_idx + 1);
        line_len--;
        cursor_idx--;
        history_read_offset = 0;

        set_cursor_horiz(cursor_idx);
        clear_from_cursor_until_end();
        uart_puts(line + cursor_idx);
        set_cursor_horiz(cursor_idx);
        break;

    default:
        if (!isprint(c) || line_len >= CLI_LINE_LEN)  /* Printable character */
            break;

        /* Insert character at cursor position */
        memmove(line + cursor_idx + 1, line + cursor_idx, line_len - cursor_idx + 1);
        line[cursor_idx] = c;
        uart_puts(line + cursor_idx);
        cursor_idx++;
        line_len++;
        history_read_offset = 0;
        set_cursor_horiz(cursor_idx);
    }
}
