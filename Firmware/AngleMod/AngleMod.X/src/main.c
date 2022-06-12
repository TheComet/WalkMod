/*
 * File:   main.c
 * Author: TheComet
 *
 * Created on May 18, 2022, 2:56 AM
 */
#include <xc.h>

#include "anglemod/gpio.h"
#include "anglemod/uart.h"
#include "anglemod/adc.h"
#include "anglemod/config.h"

#include "anglemod/btn.h"
#include "anglemod/joy.h"
#include "anglemod/dac.h"
#include "anglemod/seq.h"
#include "anglemod/cli.h"

#if !defined(CLI_SIM) && !defined(GTEST_TESTING)

/* Disable external oscillator */
#pragma config FEXTOSC = 0x01

/* Use HFINTOSC with FRQ = 32 MHz when starting */
#pragma config RSTOSC = 0

/* Don't output clock on RA4 */
#pragma config CLKOUTEN = 1

/* We're operating at 3.3V */
#pragma config VDDAR = 0

/* Disable watchdog */
#pragma config WDTE = 0

/* Disable stack overflow/underflow reset */
#pragma config STVREN = 0

/* Enable Storage Area Flash (0=enable) - We save our config struct here */
#pragma config SAFEN = 0

#endif

static enum seq active_seq = SEQ_NONE;

/* -------------------------------------------------------------------------- */
#if !defined(CLI_SIM) && !defined(GTEST_TESTING)
static void init(void)
#else
void pic16_init(void)
#endif
{
    /* Init system-level stuff */
    gpio_init();
    uart_init();
    adc_init();

    /* Load config before initializing the user-level stuff */
    config_load_from_nvm();

    /* Init user-level stuff */
    btn_init();
    dac_init();

    /* Enable interrupts */
    INTCON = 0xC0;  /* GIE=1, PEIE=1, INTEDG=0 (falling edge on INT pin) */
}

/* -------------------------------------------------------------------------- */
#if !defined(CLI_SIM) && !defined(GTEST_TESTING)
static void process_events(void)
#else
void pic16_process_events(void)
#endif
{
    char c;

    if (btn_pressed_get_and_clear())
    {
        active_seq = seq_find(joy_state_history());

        if (active_seq == SEQ_NONE)
        {
            dac_override_clamp(adc_joy_xy());
            adc_set_fast_sampling_mode();
        }
        else
        {
            dac_override_sequence(active_seq);
        }
    }
    else if (btn_released_get_and_clear())
    {
        dac_override_disable();
        adc_set_slow_sampling_mode();
    }

    if (adc_has_new_data_get_and_clear())
    {
        if (btn_is_active())
        {
            if (active_seq == SEQ_NONE)
                dac_override_clamp(adc_joy_xy());
        }
        else
        {
            joy_push_state(adc_joy_xy());
        }
    }

    /* Update CLI with incoming data */
    while (rb_rx_take_single(&c))
        cli_putc(c);
}


/* -------------------------------------------------------------------------- */
#if !defined(CLI_SIM) && !defined(GTEST_TESTING)
void main(void)
{
    init();

    while (1)
    {
#if 0
        SLEEP();
        NOP();  /* Instruction following sleep instruction is always executed */
#endif
        
        process_events();
    }
}
#endif

/* -------------------------------------------------------------------------- */
/* There is only one interrupt. We have to figure out where it came from and 
 * call the appropriate handler from here */
void __interrupt() isr(void)
{
    if (IOCAF & 0x10)
        btn_ioc_isr();
    if (PIR1bits.ADIF)
        adc_isr();
    if (PIR1bits.RC1IF)
        uart_rx_isr();
    if (PIR1bits.TX1IF)
        uart_tx_isr();
}
