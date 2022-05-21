#include "anglemod/dac.h"
#include "anglemod/gpio.h"

static uint8_t xl = 85;
static uint8_t xh = 171;
static uint8_t yl = 85;
static uint8_t yh = 171;

/* -------------------------------------------------------------------------- */
void dac_init(void)
{
}

/* -------------------------------------------------------------------------- */
void dac_set_clamp_threshold(uint8_t x, uint8_t y)
{
    if (x < 128)
    {
        xl = (uint8_t)(128 - x);
        xh = (uint8_t)(128 + x);
    }
    else
    {
        xl = 0;
        xh = 255;
    }
    
    if (y < 128)
    {
        yl = (uint8_t)(128 - y);
        yh = (uint8_t)(128 + y);
    }
    else
    {
        yl = 0;
        yh = 255;
    }
}

/* -------------------------------------------------------------------------- */
void dac_override_disable(void)
{
    gpio_clear_swxy();
}

/* -------------------------------------------------------------------------- */
void dac_override_clamp(uint8_t x, uint8_t y)
{
    if (x >= xl && x <= xh)
        gpio_clear_swx();
    else
    {
        gpio_set_swx();
    }
    
    if (y >= yl && y <= yh)
        gpio_clear_swy();
    else
    {
        gpio_set_swy();
    }
}
