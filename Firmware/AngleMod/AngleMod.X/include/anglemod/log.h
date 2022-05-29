/*!
 * @file log.h
 * @author TheComet
 */

#ifndef LOG_H
#define	LOG_H

#include "anglemod/cmd_seq.h"

#define LOG_LIST \
    X(NONE, "off", "Turn off logging") \
    X(ADC, "adc", "Log joystick values measured by the ADC") \
    X(CMD, "cmd", "Log how joystick values are quantized") \
    X(SEQ, "seq", "Log all detected command sequences") \
    X(DAC, "dac", "Log DAC")

enum log_mode
{
#define X(name, str, desc) LOG_##name,
    LOG_LIST
#undef X
    
    LOG_COUNT
};

void log_adc(void);
void log_cmd(enum joy_state states[3]);
void log_seq(enum cmd_seq seq);
void log_dac(void);

#endif	/* LOG_H */
