/*!
 * @file log.h
 * @author TheComet
 */

#ifndef LOG_H
#define	LOG_H

#include "anglemod/cmd_seq.h"

#define LOG_LIST \
    X(NONE, 0x00, "off", "Turn off all logging") \
    X(ALL,  0x0F, "all", "Turn on all logging") \
    X(ADC,  0x01, "adc", "Log joystick values measured by the ADC") \
    X(JOY,  0x02, "joy", "Log how joystick values are quantized") \
    X(SEQ,  0x04, "seq", "Log all detected command sequences") \
    X(DAC,  0x08, "dac", "Log DAC")

enum log_category
{
#define X(name, mask, str, desc) LOG_##name,
    LOG_LIST
#undef X

    LOG_COUNT,

#define X(name, mask, str, desc) LOG_##name##_MASK = mask,
    LOG_LIST
#undef X
};

void log_adc(void);
void log_joy(enum joy_state states[3]);
void log_seq(enum cmd_seq seq);
void log_dac(uint8_t swx, uint8_t swy, const uint8_t* dac01_write_buf);

#endif	/* LOG_H */
