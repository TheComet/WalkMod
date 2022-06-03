#include "anglemod/cli.h"
#include "anglemod/uart.h"
#include "anglemod/param.h"
#include "anglemod/log.h"
#include "anglemod/joy.h"
#include "anglemod/dac.h"
#include "anglemod/math.h"
#include <ctype.h>  /* isprint(), isspace() */
#include <assert.h>

#define PROMPT "> "
#define DEGREES "d"

#if defined (CLI_USE_COLOR)
#define CLEAR(x)    "\x1b[0m"    x
#define RED(x)      "\x1b[1;31m" x
#define GREEN(x)    "\x1b[1;32m" x
#define YELLOW(x)   "\x1b[1;33m" x
#define BLUE(x)     "\x1b[1;34m" x
#define MAGENTA(x)  "\x1b[1;35m" x
#define CYAN(x)     "\x1b[1;36m" x
#define WHITE(x)    "\x1b[1;37m" x
#define REDC(x)     "\x1b[1;31m" x "\x1b[0m"
#define GREENC(x)   "\x1b[1;32m" x "\x1b[0m"
#define YELLOWC(x)  "\x1b[1;33m" x "\x1b[0m"
#define BLUEC(x)    "\x1b[1;34m" x "\x1b[0m"
#define MAGENTAC(x) "\x1b[1;35m" x "\x1b[0m"
#define CYANC(x)    "\x1b[1;36m" x "\x1b[0m"
#define WHITEC(x)   "\x1b[1;37m" x "\x1b[0m"
#else
#define CLEAR(x) x
#define RED(x) x
#define GREEN(x) x
#define YELLOW(x) x
#define BLUE(x) x
#define MAGENTA(x) x
#define CYAN(x) x
#define WHITE(x) x
#define REDC(x) x
#define GREENC(x) x
#define YELLOWC(x) x
#define BLUEC(x) x
#define MAGENTAC(x) x
#define CYANC(x) x
#define WHITEC(x) x
#endif

#define PAREN1(x, color) "(" color(x) CLEAR(")")
#define PAREN2(x1, x2, color) "(" color(x1) CLEAR(",") color(x2) CLEAR(")")

static void cli_putc_normal(char c);
static void cli_putc_escape(char c);
static void cli_putc_csi(char c);

static char line[CLI_LINE_LEN + 1];
static char save_line[CLI_LINE_LEN + 1];
static char history[CLI_HISTORY_LEN][CLI_LINE_LEN + 1];

static uint8_t line_len = 0;
static uint8_t cursor_idx = 0;
static uint8_t history_write_idx = 0;
static uint8_t history_read_offset = 0;

static uint8_t log_category = 0;

#define CSI_PARAM_BUF_SIZE 1
static uint8_t csi_param_buf[CSI_PARAM_BUF_SIZE];
static uint8_t csi_param_idx = 0;

static const char* sequence_name_table[] = {
    "none",  /* SEQ_NONE */
#define X(name, str, mirrx, mirry, x, y) str " + x",
    SEQ_LIST
#undef X
};

void (*cli_putc)(char c) = cli_putc_normal;

static void cmd_help(uint8_t argc, char** argv);
static void cmd_toggle(uint8_t argc, char** argv);
static void cmd_joy(uint8_t argc, char** argv);
static void cmd_angle(uint8_t argc, char** argv);
static void cmd_mirror(uint8_t argc, char** argv);
static void cmd_clamp(uint8_t argc, char** argv);
static void cmd_quantize(uint8_t argc, char** argv);
static void cmd_log(uint8_t argc, char** argv);
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
    {"joy", "<xy value> [hysteresis]", "Configure how the joystick is converted into directions", cmd_joy},
    {"angle", "<index> <angle|<x> <y>>", "Configure angles for command inputs", cmd_angle},
    {"mirror", "<index> <x|y|xy>", "Mirrors the coordinates across the X or Y axis", cmd_mirror},
    {"clamp", "<x> <y>", "Set the clamp threshold for normal mode", cmd_clamp},
    {"quantize", "", "", cmd_quantize},
    {"save", "", "Save changes to non-volatile memory", cmd_save},
    {"load", "", "Load values from non-volatile memory, discarding any changes", cmd_discard},
    {"defaults", "", "Set default values", cmd_defaults},
    {"log", "<all|off|adc|joy|seq|dac>", "Log values in real-time", cmd_log},
    {NULL}
};

/* -------------------------------------------------------------------------- */
static void set_cursor_h(uint8_t idx)
{
    uart_printf("\x1B[%uG", idx + sizeof(PROMPT));
}

/* -------------------------------------------------------------------------- */
static void clear_from_cursor_until_end(void)
{
    uart_printf("\x1b[K");
}

/* -------------------------------------------------------------------------- */
#define u8_atoi(x) ((uint8_t)u16_atoi(x))
static uint16_t u16_atoi(const char* s)
{
    uint16_t result = *s - '0';
    while (*++s)
    {
        result *= 10;
        result += *s - '0';
    }
    return result;
}

/* -------------------------------------------------------------------------- */
#define isatoz(c) (c >= 'a' && c <= 'z')
static void cmd_help(uint8_t argc, char** argv)
{
    uart_printf(MAGENTA("\r\nAvailable Commands:"));
    for (const struct cli_cmd* cmd = commands; cmd->name; ++cmd)
    {
        uart_printf(GREENC("\r\n  %s "), cmd->name);
        for (const char* p = cmd->param; *p; ++p)
        {
            uart_putc(*p);
#if defined(CLI_USE_COLOR)
            switch ((isatoz(p[0]) << 1u) | (isatoz(p[1])))
            {
            case 1:  /* Transition from '<' to 'a' */
                uart_printf(YELLOW(""));
                break;
            case 2:  /* Transition from 'a' to '>' */
                uart_printf(CLEAR(""));
                break;
            }
#endif
        }

        uart_printf(CLEAR("\r\n    %s"), cmd->help);
    }
}

/* -------------------------------------------------------------------------- */
static void print_angles_and_toggle_states()
{
    const struct param* p = param_get();
    
    static const char* category_name_table[] = {
        "Cardinal Angles",
        "Diagonal Angles",
        "Special Angles"
    };
    
    /* NOTE: We're off by 1 here, because SEQ_NONE is at index 0 */
    for (uint8_t i = 0; i != SEQ_COUNT - 1; ++i)
    {
        uint8_t cat_idx = i >> 3;
        uint8_t item_idx = i & 0x07;
        if (item_idx == 0)
            uart_printf(MAGENTAC("\r\n%s:"), category_name_table[cat_idx]);
        uart_printf("\r\n  (\x1b[1;3%cm%c%u\x1b[0m) %s :  " YELLOWC("%u") "," YELLOWC("%u") " " PAREN1("%u" DEGREES, CYAN),
                (p->enable.bytes[cat_idx+1] & (1 << item_idx)) ? '2' : '1',
                'b' + cat_idx,
                item_idx + 1,
                sequence_name_table[i+1],
                p->angles[i].x, p->angles[i].y,
                (uint8_t)atan2((int8_t)(p->angles[i].y-127), (int8_t)(p->angles[i].x-127)));
    }
}

/* -------------------------------------------------------------------------- */
static void cmd_toggle(uint8_t argc, char** argv)
{
    uint8_t do_print = 1;
    struct param* p = param_get();

    while (argc--)
    {
        uint8_t cat_idx = argv[0][0] - 'a';
        uint8_t item_idx = argv[0][1] ? argv[0][1] - '1' : 0;
        uint8_t mask = argv[0][1] ? (uint8_t)(1 << item_idx) : 0xFF;
        if (cat_idx >= 4 || item_idx >= 8)
        {
            uart_printf(REDC("\r\nError: ") "unknown category/index");
            do_print = 0;
            continue;
        }

        if (cat_idx == 0)
        {
            p->enable.normal_mode++;
            if (p->enable.normal_mode > 2)
                p->enable.normal_mode = 0;
        }
        else
        {
            p->enable.bytes[cat_idx] ^= mask;
        }

        argv++;
    }

    if (do_print)
    {
        uart_printf(MAGENTAC("\r\nNormal Mode:"));
        uart_printf("\r\n  (\x1b[1;3%cma1\x1b[0m) When no Command is Detected : \x1b[1;3%cm%s\x1b[0m",
            p->enable.normal_mode == 0 ? '1' : '2',
            p->enable.normal_mode == 0 ? '1' : '2',
            p->enable.normal_mode == 0 ? "Do Nothing" : p->enable.normal_mode == 1 ? "Clamp" : "Quantize");

        print_angles_and_toggle_states();
    }
}

/* -------------------------------------------------------------------------- */
static void cmd_joy(uint8_t argc, char** argv)
{
    struct param* p = param_get();

    if (argc == 0)
    {
        uart_printf("\r\nThreshold: %u\r\nHysteresis: %u", p->cmd_seq.xythreshold, p->cmd_seq.hysteresis);
        return;
    }
}

/* -------------------------------------------------------------------------- */
static void cmd_angle(uint8_t argc, char** argv)
{
    struct param* p = param_get();

    if (argc >= 2 && 
        argv[0][0] >= 'b' && argv[0][0] <= 'd' && 
        argv[0][1] >= '1' && argv[0][1] <= '8')
    {
        uint8_t i = (uint8_t)((argv[0][0] - 'b') << 3) + (argv[0][1] - '1');
        
        if (argc == 3)
        {
            p->angles[i].x = u8_atoi(argv[1]);
            p->angles[i].y = u8_atoi(argv[2]);
        }
        else if (argc == 2)
        {
            uint16_t a = *argv[1] == '-' ?
                360 - u16_atoi(argv[1] + 1) :
                u16_atoi(argv[1]);
            int8_t x0 = cos_lookup(a);
            int8_t y0 = sin_lookup(a);
            p->angles[i].x = (uint8_t)(x0 + 127);
            p->angles[i].y = (uint8_t)(y0 + 127);
        }
    }

    print_angles_and_toggle_states();
}

/* -------------------------------------------------------------------------- */
static void cmd_mirror(uint8_t argc, char** argv)
{
    struct param* p = param_get();

    static const uint8_t mirror_table[SEQ_COUNT] = {
#define X(name, str, mirrx, mirry, x, y) (uint8_t)((((uint8_t)(SEQ_##mirrx - 1) << 4)) | ((uint8_t)(SEQ_##mirry) - 1)),
        SEQ_LIST
#undef X
    };

    if (argc == 2 &&
        argv[0][0] >= 'b' && argv[0][0] <= 'd' &&
        argv[0][1] >= '1' && argv[0][1] <= '8')
    {
        uint8_t from_idx = (uint8_t)((argv[0][0] - 'b') << 3) + (argv[0][1] - '1');
        uint8_t mirror_y_idx = mirror_table[from_idx] & 0x0F;
        uint8_t mirror_x_idx = mirror_table[from_idx] >> 4;
        uint8_t mirror_xy_idx = mirror_table[mirror_x_idx] & 0x0F;

        if (argv[1][0] == 'x' && argv[1][1] == 'y')
        {
            p->angles[mirror_xy_idx].x = (uint8_t)(255 - p->angles[from_idx].x);
            p->angles[mirror_xy_idx].y = (uint8_t)(255 - p->angles[from_idx].y);
        }
        if (argv[1][0] == 'x' || argv[1][1] == 'x')
        {
            p->angles[mirror_x_idx].x = p->angles[from_idx].x;
            p->angles[mirror_x_idx].y = (uint8_t)(255 - p->angles[from_idx].y);
        }
        if (argv[1][0] == 'y' || argv[1][1] == 'y')
        {
            p->angles[mirror_y_idx].x = (uint8_t)(255 - p->angles[from_idx].x);
            p->angles[mirror_y_idx].y = p->angles[from_idx].y;
        }
    }

    print_angles_and_toggle_states();
}

/* -------------------------------------------------------------------------- */
static void cmd_clamp(uint8_t argc, char** argv)
{
    if (argc < 2)
    {
        uart_printf(REDC("Error: ") "Expected <x> and <y> arguments");
        return;
    }
    dac_set_clamp_threshold(
        u8_atoi(argv[0]),
        u8_atoi(argv[1]));
}

/* -------------------------------------------------------------------------- */
static void cmd_quantize(uint8_t argc, char** argv)
{
    
}

/* -------------------------------------------------------------------------- */
static void cmd_save(uint8_t argc, char** argv)
{
    param_save_to_nvm();
    uart_printf("\r\nValues written to NVM");
}

/* -------------------------------------------------------------------------- */
static void cmd_discard(uint8_t argc, char** argv)
{
    param_load_from_nvm();
    uart_printf("\r\nValues loaded from NVM");
}

/* -------------------------------------------------------------------------- */
static void cmd_defaults(uint8_t argc, char** argv)
{
    param_set_defaults();
    uart_printf("\r\nDefault values set\r\n" CYANC("Note: ") "Use " GREENC("save") " if you want to keep default values");
}

/* -------------------------------------------------------------------------- */
static uint8_t log_lines_per_category[] = {
    2,          /* ADC prints 2 lines */
    2 + 3,      /* joy prints 3 lines */
    2 + 3 + 3,  /* seq prints 3 lines */
};
static void log_skip_other_log_outputs(enum log_category category)
{
#if defined(CLI_SIM)
    assert(category >= LOG_JOY && category <= LOG_DAC);
#endif
    uint8_t l = log_lines_per_category[category - LOG_JOY];
    while (l--)
        uart_putc('\n');
}
static void log_unskip_other_log_outputs(enum log_category category)
{
#if defined(CLI_SIM)
    assert(category >= LOG_JOY && category <= LOG_DAC);
#endif
    uint8_t l = log_lines_per_category[category - LOG_JOY];
    while (l--)
        uart_printf("\x1b[A");
}

/* -------------------------------------------------------------------------- */
void log_adc(void)
{
    if (!(log_category & LOG_ADC_MASK))
        return;

    uart_printf("\r\n" CYANC("X: ") "%u" "\x1b[K\r\n" CYANC("Y: ") "%u\x1b[K\x1b[2A",
        joy_x(), 
        joy_y());

    /* Restore cursor to where it was before printing our log message */
    set_cursor_h(cursor_idx);
}
void log_joy(enum joy_state states[3])
{
    static const char* joy_state_table[] = {
#define X(name, str) str,
        JOY_STATE_LIST
#undef X
    };

    if (!(log_category & LOG_JOY_MASK))
        return;

    /* Skip over previous log messages if they're enabled */
    log_skip_other_log_outputs(LOG_JOY);

    uart_printf("\r\n" YELLOWC("2: ") "%s\x1b[K\r\n" YELLOWC("1: ") "%s\x1b[K\r\n" YELLOWC("0: ") "%s\x1b[K\x1b[3A",
        joy_state_table[states[2]],
        joy_state_table[states[1]],
        joy_state_table[states[0]]);

    /* Restore cursor to where it was before printing our log message */
    log_unskip_other_log_outputs(LOG_JOY);
    set_cursor_h(cursor_idx);
}
void log_seq(enum cmd_seq seq)
{
    uint8_t i;
    static enum cmd_seq seq_history[3];

    if (!(log_category & LOG_SEQ_MASK))
        return;

    /* Skip over previous log messages if they're enabled */
    log_skip_other_log_outputs(LOG_SEQ);

    seq_history[0] = seq_history[1];
    seq_history[1] = seq_history[2];
    seq_history[2] = seq;

    i = 3;
    while (i--)
    {
        enum cmd_seq s = seq_history[i];
        uart_printf("\r\n" GREENC("%u: ") "%s\x1b[K", i, sequence_name_table[s]);
    }
    uart_printf("\x1b[3A");

    /* Restore cursor to where it was before printing our log message */
    log_unskip_other_log_outputs(LOG_SEQ);
    set_cursor_h(cursor_idx);
}
void log_dac(uint8_t swx, uint8_t swy, const uint8_t* dac01_write_buf)
{
    if (!(log_category & LOG_DAC))
        return;

    /* Skip over previous log messages if they're enabled */
    log_skip_other_log_outputs(LOG_DAC);

    uart_printf("\r\n" BLUEC("DAC0: "));
    if (swx)
    {
        uint8_t value = (uint8_t)(dac01_write_buf[1] << 4) | (dac01_write_buf[2] >> 4);
        uart_printf("%u", value);
    }
    else
        uart_printf("--");
    uart_printf("\x1b[K\r\n" BLUEC("DAC1: "));    
    if (swy)
    {
        uint8_t value = (uint8_t)(dac01_write_buf[4] << 4) | (dac01_write_buf[5] >> 4);
        uart_printf("%u", value);
    }
    else
        uart_printf("--");
    uart_printf("\x1b[K\x1b[2A");

    /* Restore cursor to where it was before printing our log message */
    log_unskip_other_log_outputs(LOG_DAC);
    set_cursor_h(cursor_idx);
}
static void cmd_log(uint8_t argc, char** argv)
{
    struct category {
        const char* name;
        const char* desc;
        uint8_t mask;
    };
    static const struct category categories[] = {
#define X(name, mask, str, desc) {str, desc, mask},
        LOG_LIST
#undef X
    };
    
    if (argc == 0)
    {
        uart_printf(MAGENTA("\r\nAvailable log categories:"));
        for (uint8_t i = 0; i != LOG_COUNT; ++i)
            uart_printf("\r\n  " GREENC("%s") "  %s", categories[i].name, categories[i].desc);
        return;
    }
    
    for (uint8_t i = 0; i != LOG_COUNT; ++i)
        if (strcmp(argv[0], categories[i].name) == 0)
        {
            if (i < 2)  /* ALL and NONE categories */
                log_category = categories[i].mask;
            else
                log_category ^= categories[i].mask;
            return;
        }
    
    uart_printf(REDC("\r\nError: ") "Unknown category");
}

/* -------------------------------------------------------------------------- */
/*!
 * @brief Splits a string at each whitespace by inserting null-bytes. Pointers
 * to each substring are written to the argv parameter. Empty strings are
 * removed and multiple consecutive spaces, including leading and trailing
 * spaces, are ignored.
 */
static uint8_t split_args(char* s, char** argv, uint8_t maxsplit)
{
    char* start;
    uint8_t argc = 0;
    for (start = s; *s && argc < maxsplit; ++s)
    {
        if (*s == ' ')
        {
            if (s == start)
            {
                start++;
                continue;
            }

            argv[argc++] = start;
            *s = '\0';
            start = s + 1;
        }
    }
    if (argc < maxsplit && *start)
        argv[argc++] = start;
    return argc;
}

/* -------------------------------------------------------------------------- */
static void execute_current_line(void)
{
    char* argv[4];
    uint8_t argc = split_args(line, argv, 4);
    for (const struct cli_cmd* cmd = commands; cmd->name; ++cmd)
    {
        if (strcmp(argv[0], cmd->name))
            continue;
        
        cmd->handler(argc - 1, argv + 1);
        return;
    }

    uart_printf(REDC("\r\nError:") " Unknown command");
}

/* -------------------------------------------------------------------------- */
static void set_line_and_print(const char* new_text)
{
    strcpy(line, new_text);
    line_len = (uint8_t)strlen(line);
    cursor_idx = line_len;
    set_cursor_h(0);
    clear_from_cursor_until_end();
    uart_printf(line);
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
     * Wiki says parameter bytes could be followed by "intermediate bytes" in 
     * the range 0x20-0x2F, but none of the sequences we care about have these,
     * so we ignore them.
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
            /* Here we make the assumption that the CSI param consists of 1 character. I hope that's OK */
            if (csi_param_idx == 0)
                cursor_idx++;  /* default is 1 */
            else
                cursor_idx += csi_param_buf[0] - '0';

            if (cursor_idx >= line_len)
                cursor_idx = line_len;

            set_cursor_h(cursor_idx);
            break;

        case 'D':  /* Cursor back */
            /* Here we make the assumption that the CSI param consists of 1 character. I hope that's OK */
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

            set_cursor_h(cursor_idx);
            break;

        case '~':  /* Input sequence */
            if (csi_param_idx == 0)
                break;
            switch (csi_param_buf[0]) {
            case '1':  /* Home key */
            case '7':  /* Home key */
                cursor_idx = 0;
                set_cursor_h(0);
                break;

            case '4':  /* End key */
            case '8':  /* End key */
                cursor_idx = line_len;
                set_cursor_h(cursor_idx);
                break;

            case '3':  /* Delete key */
                if (cursor_idx == line_len)  /* Cant erase stuff at end */
                    break;

                /* This memmove will also move the null terminator along */
                memmove(line + cursor_idx, line + cursor_idx + 1, line_len - cursor_idx);
                line_len--;
                history_read_offset = 0;

                clear_from_cursor_until_end();
                uart_printf(line + cursor_idx);
                set_cursor_h(cursor_idx);
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
            uart_printf("^C");

        cursor_idx = 0;
        line_len = 0;
        line[0] = '\0';
        uart_printf("\r\n" PROMPT "\x1b[K");

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

        set_cursor_h(cursor_idx);
        clear_from_cursor_until_end();
        uart_printf(line + cursor_idx);
        set_cursor_h(cursor_idx);
        break;

    default:
        if (!isprint((uint8_t)c) || line_len >= CLI_LINE_LEN)  /* Printable character */
            break;

        /* Insert character at cursor position */
        memmove(line + cursor_idx + 1, line + cursor_idx, line_len - cursor_idx + 1);
        line[cursor_idx] = c;
        uart_printf(line + cursor_idx);
        cursor_idx++;
        line_len++;
        history_read_offset = 0;
        set_cursor_h(cursor_idx);
    }
}

#if defined(GTEST_TESTING)

#include <gmock/gmock.h>

using namespace ::testing;

TEST(split_args, empty_string)
{
    char s[] = "";
    char* argv[3];
    ASSERT_THAT(split_args(s, argv, 3), Eq(0));
}

TEST(split_args, single_word)
{
    char s[] = "test";
    char* argv[3];
    ASSERT_THAT(split_args(s, argv, 3), Eq(1));
    EXPECT_THAT(argv[0], StrEq("test"));
}

TEST(split_args, two_words)
{
    char s[] = "another test";
    char* argv[3];
    ASSERT_THAT(split_args(s, argv, 3), Eq(2));
    EXPECT_THAT(argv[0], StrEq("another"));
    EXPECT_THAT(argv[1], StrEq("test"));
}

TEST(split_args, three_words)
{
    char s[] = "foo bar baz";
    char* argv[3];
    ASSERT_THAT(split_args(s, argv, 3), Eq(3));
    EXPECT_THAT(argv[0], StrEq("foo"));
    EXPECT_THAT(argv[1], StrEq("bar"));
    EXPECT_THAT(argv[2], StrEq("baz"));
}

TEST(split_args, three_letters)
{
    char s[] = "a b c";
    char* argv[3];
    ASSERT_THAT(split_args(s, argv, 3), Eq(3));
    EXPECT_THAT(argv[0], StrEq("a"));
    EXPECT_THAT(argv[1], StrEq("b"));
    EXPECT_THAT(argv[2], StrEq("c"));
}

TEST(split_args, more_words_than_max)
{
    char s[] = "foo bar baz bla";
    char* argv[3];
    ASSERT_THAT(split_args(s, argv, 3), Eq(3));
    EXPECT_THAT(argv[0], StrEq("foo"));
    EXPECT_THAT(argv[1], StrEq("bar"));
    EXPECT_THAT(argv[2], StrEq("baz"));
}

TEST(split_args, multiple_spaces)
{
    char s[] = "foo   bar";
    char* argv[3];
    ASSERT_THAT(split_args(s, argv, 3), Eq(2));
    EXPECT_THAT(argv[0], StrEq("foo"));
    EXPECT_THAT(argv[1], StrEq("bar"));
}

TEST(split_args, leading_spaces)
{
    char s[] = "   foo bar";
    char* argv[3];
    ASSERT_THAT(split_args(s, argv, 3), Eq(2));
    EXPECT_THAT(argv[0], StrEq("foo"));
    EXPECT_THAT(argv[1], StrEq("bar"));
}

TEST(split_args, trailing_spaces)
{
    char s[] = "foo bar   ";
    char* argv[3];
    ASSERT_THAT(split_args(s, argv, 3), Eq(2));
    EXPECT_THAT(argv[0], StrEq("foo"));
    EXPECT_THAT(argv[1], StrEq("bar"));
}

#endif
