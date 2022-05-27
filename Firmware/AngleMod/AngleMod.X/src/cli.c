#include "anglemod/cli.h"
#include "anglemod/uart.h"
#include "anglemod/param.h"
#include <ctype.h>  /* isprint() */

#define PROMPT "> "
#define DEGREES "\xC2\xB0"

#define CLEAR(x)   "\x1b[0m" x
#define RED(x)     "\x1b[1;31m" x
#define GREEN(x)   "\x1b[1;32m" x
#define YELLOW(x)  "\x1b[1;33m" x
#define BLUE(x)    "\x1b[1;34m" x
#define MAGENTA(x) "\x1b[1;35m" x
#define CYAN(x)    "\x1b[1;36m" x
#define WHITE(x)   "\x1b[1;37m" x
#define REDC(x)     "\x1b[1;31m" x "\x1b[0m"
#define GREENC(x)   "\x1b[1;32m" x "\x1b[0m"
#define YELLOWC(x)  "\x1b[1;33m" x "\x1b[0m"
#define BLUEC(x)    "\x1b[1;34m" x "\x1b[0m"
#define MAGENTAC(x) "\x1b[1;35m" x "\x1b[0m"
#define CYANC(x)    "\x1b[1;36m" x "\x1b[0m"
#define WHITEC(x)   "\x1b[1;37m" x "\x1b[0m"

#define PAREN1(x, color) "(" color(x) CLEAR(")")
#define PAREN2(x1, x2, color) "(" color(x1) CLEAR(",") color(x2) CLEAR(")")

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

static void cmd_help(uint8_t argc, char** argv);
static void cmd_toggle(uint8_t argc, char** argv);
static void cmd_thresh(uint8_t argc, char** argv);
static void cmd_slight(uint8_t argc, char** argv);
static void cmd_save(uint8_t argc, char** argv);
static void cmd_discard(uint8_t argc, char** argv);
static void cmd_defaults(uint8_t argc, char** argv);

struct cli_cmd {
    const char* name;
    const char* param;
    const char* help;
    void (*handler)(uint8_t argc, char** argv);
};

static struct cli_cmd commands[] = {
    {"help", "", "Print help", cmd_help},
    {"toggle", "<index>", "Enable/disable different modes", cmd_toggle},
    {"threshold", "<xy value> [hysteresis]", "Configure how command inputs are detected", cmd_thresh},
    {"slight", "<index> <angle|<x> <y>>", "Configure slight angles", cmd_slight},
    {"save", "", "Save changes to non-volatile memory", cmd_save},
    {"load", "", "Load values from non-volatile memory, discarding any changes", cmd_discard},
    {"defaults", "", "Set default values", cmd_defaults},
    {NULL}
};

/* -------------------------------------------------------------------------- */
static void set_cursor_horiz(uint8_t idx)
{
    uart_printf("\x1B[%uG", idx + sizeof(PROMPT));
}

/* -------------------------------------------------------------------------- */
#define isatoz(c) (c >= 'a' && c <= 'z')
static void cmd_help(uint8_t argc, char** argv)
{
    uart_puts(WHITE("\r\nAvailable Commands:"));
    for (const struct cli_cmd* cmd = commands; cmd->name; ++cmd)
    {
        uart_printf(GREEN("\r\n  %s "), cmd->name);

        if (cmd->param[0])
        {
            const char* param_start = cmd->param;
            const char* param_end = cmd->param;
            for (; *param_end; ++param_end)
            {
                switch ((isatoz(*param_start) << 1) | isatoz(*param_end))
                {
                case 1:  /* Transition from '<' to 'a' */
                    uart_puts(CLEAR(""));
                    uart_putbytes(param_start, (uint8_t)(param_end - param_start));
                    param_start = param_end;
                    break;

                case 2:  /* Transition from 'a' to '>' */
                    uart_puts(YELLOW(""));
                    uart_putbytes(param_start, (uint8_t)(param_end - param_start));
                    param_start = param_end;
                    break;
                }
            }
            uart_puts(CLEAR(""));
            uart_putbytes(param_start, (uint8_t)(param_end - param_start));
        }

        uart_printf(CLEAR("\r\n    %s"), cmd->help);
    }
}

/* -------------------------------------------------------------------------- */
static void print_toggle_states()
{
    const struct param* p = param_get();

    static const char* fmt = "\r\n" PAREN1("%u", YELLOW) " %s : " "\x1B[1;3%cm%s\x1B[0m";
    static const char* mode_name_table[] = {
        "Normal Mode",
        "A-Angles   ",
        "B-Angles   ",
        "C-Angles   "
    };

    char angles_table[] = {
        p->enable.a_angles,
        p->enable.b_angles,
        p->enable.c_angles
    };

    uart_printf(fmt, 1, mode_name_table[0],
        p->enable.normal_mode == 0 ? '1' : '2',
        p->enable.normal_mode == 0 ? "Off" : p->enable.normal_mode == 1 ? "Clamp" : "Quantize");

    for (uint8_t i = 1; i != 4; ++i)
    {
        uart_printf(fmt, i+1, mode_name_table[i],
            angles_table[i-1] ? '2' : '1',
            angles_table[i-1] ? "On" : "Off");
    }
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
        p->enable.a_angles = ~p->enable.a_angles;
        break;
    case '3':
        p->enable.b_angles = ~p->enable.b_angles;
        break;
    case '4':
        p->enable.c_angles = ~p->enable.c_angles;
        break;
    default:
        uart_puts(REDC("\r\nError: ") "Expected a value from 1-3");
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
static void cmd_slight(uint8_t argc, char** argv)
{
#define ARROW_LEFT       "\xF0\x9F\xA1\xA0"
#define ARROW_UP         "\xF0\x9F\xA1\xA1"
#define ARROW_RIGHT      "\xF0\x9F\xA1\xA2"
#define ARROW_DOWN       "\xF0\x9F\xA1\xA3"
#define ARROW_UP_LEFT    "\xF0\x9F\xA1\xA4"
#define ARROW_UP_RIGHT   "\xF0\x9F\xA1\xA5"
#define ARROW_DOWN_RIGHT "\xF0\x9F\xA1\xA6"
#define ARROW_DOWN_LEFT  "\xF0\x9F\xA1\xA7"

    static const char* arrow_table[20] = {
        /* A-Angles */
        ARROW_UP ARROW_UP_RIGHT,
        ARROW_RIGHT ARROW_UP_RIGHT,
        ARROW_RIGHT ARROW_DOWN_RIGHT,
        ARROW_DOWN ARROW_DOWN_RIGHT,
        ARROW_UP ARROW_UP_LEFT,
        ARROW_LEFT ARROW_UP_LEFT,
        ARROW_LEFT ARROW_DOWN_LEFT,
        ARROW_DOWN ARROW_DOWN_LEFT,

        /* B-Angles */
        ARROW_UP_RIGHT ARROW_UP,
        ARROW_UP_RIGHT ARROW_RIGHT,
        ARROW_DOWN_RIGHT ARROW_RIGHT,
        ARROW_DOWN_RIGHT ARROW_DOWN,
        ARROW_UP_LEFT ARROW_UP,
        ARROW_UP_LEFT ARROW_LEFT,
        ARROW_DOWN_LEFT ARROW_LEFT,
        ARROW_DOWN_LEFT ARROW_DOWN,

        /* C-Angles */
        ARROW_RIGHT ARROW_UP_RIGHT ARROW_UP,
        ARROW_RIGHT ARROW_DOWN_RIGHT ARROW_DOWN,
        ARROW_LEFT ARROW_UP_LEFT ARROW_UP,
        ARROW_LEFT ARROW_DOWN_LEFT ARROW_DOWN
    };

    static const char* fmt = "\r\n  " PAREN1("%c%u", YELLOW) "%s + x:  " GREENC("%u" DEGREES) PAREN2("%u", "%u", CYAN);
    
    struct param* p = param_get();

    uart_puts(MAGENTAC("\r\nA-Angles:"));
    for (uint8_t i = 1; i <= 8; ++i)
        uart_printf(fmt, 'a', i, arrow_table[i-1], 45, 255, 255);

    uart_puts(MAGENTAC("\r\nB-Angles:"));
    for (uint8_t i = 1; i <= 8; ++i)
        uart_printf(fmt, 'b', i, arrow_table[i+7], 45, 255, 255);

    uart_puts(MAGENTAC("\r\nC-Angles:"));
    for (uint8_t i = 1; i <= 4; ++i)
        uart_printf(fmt, 'c', i, arrow_table[i+15], 45, 255, 255);
}

/* -------------------------------------------------------------------------- */
static void cmd_save(uint8_t argc, char** argv)
{
    param_save_to_nvm();
    uart_puts("\r\nValues written to NVM");
}

/* -------------------------------------------------------------------------- */
static void cmd_discard(uint8_t argc, char** argv)
{
    param_load_from_nvm();
    uart_puts("\r\nValues loaded from NVM");
}

/* -------------------------------------------------------------------------- */
static void cmd_defaults(uint8_t argc, char** argv)
{
    param_set_defaults();
    uart_puts("\r\nDefault values set\r\n" CYANC("Note: ") "Use " GREENC("save") " if you want to keep default values");
}

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

    uart_puts(REDC("\r\nError:") " Unknown command");
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
