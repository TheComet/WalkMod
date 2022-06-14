#include "anglemod/btn.h"
#include <xc.h>

uint8_t _btn_state = 0xFF;

/* -------------------------------------------------------------------------- */
void btn_init(void)
{
    /* 
     * We don't actually do anything in the interrupt service routine. The only
     * purpose of enabling these is so the device wakes up from sleep and then
     * polls the state of the button
     */
    
    /* Enable interrupt-on-change on BTN pin (RA4) for both rising and falling
     * edges */
    IOCxP(BTN_PORT) = BTN_BIT;
    IOCxN(BTN_PORT) = BTN_BIT;
    
    /* Enable interrupt-on-change interrupt */
    PIE0bits.IOCIE = 1;
}

/* -------------------------------------------------------------------------- */
void btn_ioc_isr(void)
{
    /* Still need to clear the flag so we don't get stuck in an endless interrupt loop */
    IOCxF(BTN_PORT) &= ~BTN_BIT;
}
