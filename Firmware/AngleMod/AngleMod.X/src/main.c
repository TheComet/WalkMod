/*
 * File:   main.c
 * Author: TheComet
 *
 * Created on May 18, 2022, 2:56 AM
 */
#include <xc.h>

#include "anglemod/osc.h"
#include "anglemod/gpio.h"
#include "anglemod/uart.h"
#include "anglemod/param.h"

#include "anglemod/btn.h"
#include "anglemod/joy.h"
#include "anglemod/dac.h"
#include "anglemod/cmd_seq.h"
#include "anglemod/cli.h"

// We're operating at 3.3V
#pragma config VDDAR = 0

// Disable watchdog
#pragma config WDTE = 0

static enum cmd_seq active_cmd = CMD_NONE;

/* -------------------------------------------------------------------------- */
#if !defined(CLI_SIM)
static void init(void)
#else
void pic16_init(void)
#endif
{
    /* Init system-level stuff */
    osc_init();
    gpio_init();
    uart_init();
    param_init();

    /* Init user-level stuff */
    joy_init();
    btn_init();
    dac_init();

    /* Enable interrupts */
    INTCONbits.GIE = 1;
}

/* -------------------------------------------------------------------------- */
#if !defined(CLI_SIM)
static void process_events(void)
#else
void pic16_process_events(void)
#endif
{
    char c;

    if (btn_pressed_get_and_clear())
    {
        active_cmd = cmd_seq_determine_command();

        if (active_cmd == CMD_NONE)
        {
            dac_override_clamp(joy_x(), joy_y());
            joy_set_fast_sampling_mode();
        }
    }
    else if (btn_released_get_and_clear())
    {
        dac_override_disable();
        joy_set_slow_sampling_mode();
    }

    if (joy_has_new_data_get_and_clear())
    {
        if (btn_is_active())
        {
            if (active_cmd == CMD_NONE)
                dac_override_clamp(joy_x(), joy_y());
        }
        else
        {
            cmd_seq_push_joy_angle(joy_x(), joy_y());
        }
    }

    /* Update CLI with incoming data */
    while (rb_rx_take_single(&c))
        cli_putc(c);
}

/* -------------------------------------------------------------------------- */
#if !defined(CLI_SIM)
void main(void)
{
    init();

    while (1)
    {
        process_events();
        
        SLEEP();
        NOP();  /* Instruction following sleep instruction is always executed */
    }
}
#endif

/* -------------------------------------------------------------------------- */
/* There is only one interrupt. We have to figure out where it came from and 
 * call the appropriate handler from here */
void __interrupt() isr(void)
{
    if (PIR0bits.INTF)
        btn_int_isr();
    if (PIR0bits.TMR0IF)
        joy_tim0_isr();
    if (PIR1bits.ADIF)
        joy_adc_isr();
    if (PIR1bits.RC1IF)
        uart_rx_isr();
    if (PIR1bits.TX1IF)
        uart_tx_isr();
}