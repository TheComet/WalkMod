#include "anglemod/cli.h"
#include "anglemod/uart.h"
#include "anglemod/param.h"
#include "anglemod/log.h"
#include "anglemod/joy.h"
#include "anglemod/dac.h"
#include <ctype.h>  /* isprint(), isspace() */

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

static char line[CLI_LINE_LEN + 1];
static char save_line[CLI_LINE_LEN + 1];
static char history[CLI_HISTORY_LEN][CLI_LINE_LEN + 1];

static uint8_t line_len = 0;
static uint8_t cursor_idx = 0;
static uint8_t history_write_idx = 0;
static uint8_t history_read_offset = 0;

static uint8_t log_mode = LOG_NONE;

#define CSI_PARAM_BUF_SIZE 1
static uint8_t csi_param_buf[CSI_PARAM_BUF_SIZE];
static uint8_t csi_param_idx = 0;

static const char* arrow_table[] = {
    "none",  /* SEQ_NONE */
#define X(name, str, x, y) str,
    SEQ_LIST
#undef X
};

void (*cli_putc)(char c) = cli_putc_normal;

static void cmd_help(uint8_t argc, char** argv);
static void cmd_toggle(uint8_t argc, char** argv);
static void cmd_joy(uint8_t argc, char** argv);
static void cmd_angle(uint8_t argc, char** argv);
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
    {"clamp", "<x> <y>", "Set the clamp threshold for normal mode", cmd_clamp},
    {"quantize", "", "", cmd_quantize},
    {"save", "", "Save changes to non-volatile memory", cmd_save},
    {"load", "", "Load values from non-volatile memory, discarding any changes", cmd_discard},
    {"defaults", "", "Set default values", cmd_defaults},
    {"log", "<adc|cmd|dac>", "Log values in real-time", cmd_log},
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
    uart_puts("\x1b[K");
}

/* -------------------------------------------------------------------------- */
static uint8_t u8_atoi(const char* s)
{
    uint8_t result = *s - '0';
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
                switch ((isatoz(*param_start) << 1u) | isatoz(*param_end))
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

    static const char* fmt1 = "\r\n" PAREN1("%u", YELLOW) "  %s : " "\x1B[1;3%cm%s\x1B[0m";
    static const char* fmt2 = "\r\n" PAREN1("c%u", YELLOW) " Special Angle c%u : " "\x1B[1;3%cm%s\x1B[0m";
    static const char* mode_name_table[] = {
        "Normal Mode     ",
        "Cardinal Angles ",
        "Diagonal Angles ",
    };

    uart_printf(fmt1, 1, mode_name_table[0],
        p->enable.normal_mode == 0 ? '1' : '2',
        p->enable.normal_mode == 0 ? "Off" : p->enable.normal_mode == 1 ? "Clamp" : "Quantize");

    for (uint8_t i = 1; i != 3; ++i)
    {
        uint8_t enabled = (uint8_t)(p->enable.bits & (2u << i));
        uart_printf(fmt1, i+1, mode_name_table[i],
            enabled ? '2' : '1',
            enabled ? "On" : "Off");
    }

    for (uint8_t i = 0; i != 8; ++i)
    {
        uint8_t enabled = (uint8_t)(p->enable.special_angles & (1u << i));
        uart_printf(fmt2, i+1, i+1,
            enabled ? '2' : '1',
            enabled ? "On" : "Off");
    }
}

/* -------------------------------------------------------------------------- */
static void cmd_toggle(uint8_t argc, char** argv)
{
    struct param* p = param_get();

    if (argc > 0)
    {
        switch (*argv[0])
        {
        case '1':
            p->enable.normal_mode++;
            if (p->enable.normal_mode > 2)
                p->enable.normal_mode = 0;
            break;
        case '2':
            p->enable.cardinal_angles = ~p->enable.cardinal_angles;
            break;
        case '3':
            p->enable.diagonal_angles = ~p->enable.diagonal_angles;
            break;
        case 'c':
            if (argv[0][1] < '1' || argv[0][1] > '8')
                goto unknown_argument;
            p->enable.special_angles ^= 1 << (argv[0][1] - '1');
            break;
        }
    }

    print_toggle_states();
    return;

unknown_argument:
    uart_printf(REDC("\r\nError: ") "unknown index '%s'", argv[0]);
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
    static const char* fmt = "\r\n  " PAREN1("%c%u", YELLOW) " %s + x:  " GREENC("%u") "," GREENC("%u") " " PAREN1("%u" DEGREES, CYAN);
    
    struct param* p = param_get();

    if (argc > 2 && 
        argv[0][0] >= 'a' && argv[0][0] <= 'c' && 
        argv[0][1] >= '1' && argv[0][1] <= '8')
    {
        uint8_t i = (uint8_t)((argv[0][0] - 'a') << 3) + (argv[0][1] - '1');
        p->angles[i].x = u8_atoi(argv[1]);
        p->angles[i].y = u8_atoi(argv[2]);
    }

    /* NOTE: We start at index 1 because index 0 is SEQ_NONE */
    uart_puts(MAGENTAC("\r\nCardinal Angles:"));
    for (uint8_t i = 1; i <= 8; ++i)
        uart_printf(fmt, 'a', i, arrow_table[i],
            p->angles[i-1].x, p->angles[i-1].y, 45);

    uart_puts(MAGENTAC("\r\nDiagonal Angles:"));
    for (uint8_t i = 1; i <= 8; ++i)
        uart_printf(fmt, 'b', i, arrow_table[i+8],
            p->angles[i+7].x, p->angles[i+7].y, 45);

    uart_puts(MAGENTAC("\r\nSpecial Angles:"));
    for (uint8_t i = 1; i <= 8; ++i)
        uart_printf(fmt, 'c', i, arrow_table[i+16],
            p->angles[i+15].x, p->angles[i+15].y, 45);
}

/* -------------------------------------------------------------------------- */
static void cmd_clamp(uint8_t argc, char** argv)
{
    if (argc < 2)
    {
        uart_puts(REDC("Error: ") "Expected <x> and <y> arguments");
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
void log_adc(void)
{
    if (!(log_mode & LOG_ADC))
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

    if (!(log_mode & LOG_JOY))
        return;

    /* Skip over previous log messages if they're enabled */
    if (log_mode & LOG_ADC)
        uart_puts("\n\n");

    uart_printf("\r\n" YELLOWC("2: ") "%s\x1b[K\r\n" YELLOWC("1: ") "%s\x1b[K\r\n" YELLOWC("0: ") "%s\x1b[K\x1b[3A",
        joy_state_table[states[2]],
        joy_state_table[states[1]],
        joy_state_table[states[0]]);

    /* Restore cursor to where it was before printing our log message */
    if (log_mode & LOG_ADC)
        uart_puts("\x1b[2A");
    set_cursor_h(cursor_idx);
}
void log_seq(enum cmd_seq seq)
{
    uint8_t i;
    static enum cmd_seq seq_history[3];

    if (!(log_mode & LOG_SEQ))
        return;

    /* Skip over previous log messages if they're enabled */
    if (log_mode & LOG_ADC)
        uart_puts("\n\n");
    if (log_mode & LOG_JOY)
        uart_puts("\n\n\n");

    seq_history[0] = seq_history[1];
    seq_history[1] = seq_history[2];
    seq_history[2] = seq;

    i = 3;
    while (i--)
    {
        enum cmd_seq s = seq_history[i];
        uart_printf("\r\n" GREENC("%u: ") "%s%s\x1b[K", i, arrow_table[s], s ? " + x" : "");;
    }
    uart_puts("\x1b[3A");

    /* Restore cursor to where it was before printing our log message */
    if (log_mode & LOG_JOY)
        uart_puts("\x1b[3A");
    if (log_mode & LOG_ADC)
        uart_puts("\x1b[2A");
    set_cursor_h(cursor_idx);
}
void log_dac(uint8_t swx, uint8_t swy, const uint8_t* dac01_write_buf)
{
    if (!(log_mode & LOG_DAC))
        return;

    /* Skip over previous log messages if they're enabled */
    if (log_mode & LOG_ADC)
        uart_puts("\n\n");
    if (log_mode & LOG_JOY)
        uart_puts("\n\n\n");
    if (log_mode & LOG_SEQ)
        uart_puts("\n\n\n");

    uart_puts("\r\n" BLUEC("DAC0: "));
    if (swx)
    {
        uint8_t value = (uint8_t)(dac01_write_buf[1] << 4) | (dac01_write_buf[2] >> 4);
        uart_printf("%u", value);
    }
    else
        uart_puts("--");
    uart_puts("\x1b[K\r\n" BLUEC("DAC1: "));    
    if (swy)
    {
        uint8_t value = (uint8_t)(dac01_write_buf[4] << 4) | (dac01_write_buf[5] >> 4);
        uart_printf("%u", value);
    }
    else
        uart_puts("--");
    uart_puts("\x1b[K\x1b[2A");

    /* Restore cursor to where it was before printing our log message */
    if (log_mode & LOG_SEQ)
        uart_puts("\x1b[3A");
    if (log_mode & LOG_JOY)
        uart_puts("\x1b[3A");
    if (log_mode & LOG_ADC)
        uart_puts("\x1b[2A");
    set_cursor_h(cursor_idx);
}
static void cmd_log(uint8_t argc, char** argv)
{
    struct category {
        const char* name;
        const char* desc;
        uint8_t value;
    };
    static const struct category categories[] = {
#define X(name, value, str, desc) {str, desc, value},
        LOG_LIST
#undef X
    };
    
    if (argc == 0)
    {
        uart_puts(MAGENTA("\r\nAvailable log categories:"));
        for (uint8_t i = 0; i != LOG_COUNT; ++i)
            uart_printf("\r\n  " GREENC("%s") "  %s", categories[i].name, categories[i].desc);
    }
    else
    {
        for (uint8_t i = 0; i != LOG_COUNT; ++i)
            if (strcmp(argv[0], categories[i].name) == 0)
            {
                if (i < LOG_MASK_START)
                    log_mode = categories[i].value;
                else
                    log_mode ^= categories[i].value;
                return;
            }
        uart_printf(REDC("\r\nError: ") "Unknown category '%s'", argv[0]);
    }
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

    uart_puts(REDC("\r\nError:") " Unknown command");
}

/* -------------------------------------------------------------------------- */
static void set_line_and_print(const char* new_text)
{
    strcpy(line, new_text);
    line_len = (uint8_t)strlen(line);
    cursor_idx = line_len;
    set_cursor_h(0);
    clear_from_cursor_until_end();
    uart_puts(line);
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
            /* Here we make the assumption that the CSI param consists of 1 character */
            if (csi_param_idx == 0)
                cursor_idx++;  /* default is 1 */
            else
                cursor_idx += csi_param_buf[0] - '0';

            if (cursor_idx >= line_len)
                cursor_idx = line_len;

            set_cursor_h(cursor_idx);
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
                uart_puts(line + cursor_idx);
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
            uart_puts("^C");

        cursor_idx = 0;
        line_len = 0;
        line[0] = '\0';
        uart_puts("\r\n" PROMPT "\x1b[K");

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
        uart_puts(line + cursor_idx);
        set_cursor_h(cursor_idx);
        break;

    default:
        if (!isprint((uint8_t)c) || line_len >= CLI_LINE_LEN)  /* Printable character */
            break;

        /* Insert character at cursor position */
        memmove(line + cursor_idx + 1, line + cursor_idx, line_len - cursor_idx + 1);
        line[cursor_idx] = c;
        uart_puts(line + cursor_idx);
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
