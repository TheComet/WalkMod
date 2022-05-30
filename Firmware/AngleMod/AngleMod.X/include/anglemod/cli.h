/*!
 * @file cli.h
 * @author TheComet
 */

#ifndef CLI_H
#define	CLI_H

#define CLI_LINE_LEN 15
#define CLI_HISTORY_LEN 8
#define CLI_USE_COLOR

extern void (*cli_putc)(char c);

#endif	/* CLI_H */
