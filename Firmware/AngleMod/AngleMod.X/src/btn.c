#include "anglemod/btn.h"
#include <xc.h>

enum state_bits
{
    NONE = 0x00,
    PRESSED = 0x01,
    RELEASED = 0x02
};

static volatile uint8_t state;

/* -------------------------------------------------------------------------- */
void btn_init(void)
{
}

/* -------------------------------------------------------------------------- */
uint8_t btn_pressed_get_and_clear(void)
{
    uint8_t pressed = (state & PRESSED);
    state &= ~PRESSED;
    return pressed;
}

/* -------------------------------------------------------------------------- */
uint8_t btn_released_get_and_clear(void)
{
    uint8_t released = (state & RELEASED);
    state &= ~RELEASED;
    return released;
}

/* -------------------------------------------------------------------------- */
uint8_t btn_is_active(void)
{
    return PORTA & 0x10;
}

/* -------------------------------------------------------------------------- */
void btn_ioc_isr(void)
{
    state = (PORTA & 0x10) ? PRESSED : RELEASED;
    IOCAF &= ~0x10;
}
